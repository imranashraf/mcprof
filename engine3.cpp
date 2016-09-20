/*
         __       __   ______   _______   _______    ______   ________
        /  \     /  | /      \ /       \ /       \  /      \ /        |
        $$  \   /$$ |/$$$$$$  |$$$$$$$  |$$$$$$$  |/$$$$$$  |$$$$$$$$/
        $$$  \ /$$$ |$$ |  $$/ $$ |__$$ |$$ |__$$ |$$ |  $$ |$$ |__
        $$$$  /$$$$ |$$ |      $$    $$/ $$    $$< $$ |  $$ |$$    |
        $$ $$ $$/$$ |$$ |   __ $$$$$$$/  $$$$$$$  |$$ |  $$ |$$$$$/
        $$ |$$$/ $$ |$$ \__/  |$$ |      $$ |  $$ |$$ \__$$ |$$ |
        $$ | $/  $$ |$$    $$/ $$ |      $$ |  $$ |$$    $$/ $$ |
        $$/      $$/  $$$$$$/  $$/       $$/   $$/  $$$$$$/  $$/

                A Memory and Communication Profiler

 * This file is a part of MCPROF.
 * https://bitbucket.org/imranashraf/mcprof
 * 
 * Copyright (c) 2014-2015 TU Delft, The Netherlands.
 * All rights reserved.
 * 
 * MCPROF is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCPROF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MCPROF.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Authors: Imran Ashraf
 *
 */

#include "globals.h"
#include "shadow.h"
#include "engine3.h"
#include "commatrix.h"
#include "symbols.h"

extern CallStackType CallStack; // main call stack
extern std::map <std::string,IDNoType> FuncName2ID;
extern Symbols symTable;
extern std::ofstream pcout;
extern bool FlushCalls;
extern bool DoTrace;
extern u32 FlushCallsLimit;

// all calls to a single func, index is call no
typedef vector<Call> AllCalls2OneFtnType;

// all calls to all functions, index is the funcID
typedef map<u16,AllCalls2OneFtnType> AllCalls2AllFtnsType;

AllCalls2AllFtnsType AllCalls;
Call tempCall;  // To avoid seg fault for info stored b4 calling main
Call* currCall=&tempCall; // pointer to current call
// TODO This can also be achieved by adding a call for UnknownID

/*
whenever a func is entered, a new call is created. "currCall" pointer then
points to this new call, which is updated only at function enty/exit. Hence it
is not required to determine the right Call per read/write access.
*/

/*
GlobalCallSeqNo is used to assign sequence numbers to all calls globaly.
Based on this seq no, temporal orders of calls can be obtained
*/
static u64 GlobalCallSeqNo=0;

// only used in this file
void PrintCalls(AllCalls2OneFtnType& calls, ofstream& fout);

void SetCurrCallOnEntry()
{
    IDNoType funcid = CallStack.Top();
    D2ECHO("Setting currCall on Entry for " << FUNC(funcid) );
    auto it = AllCalls.find(funcid);
    if( it == AllCalls.end() )
    {
        AllCalls[funcid] = AllCalls2OneFtnType();
    }

    AllCalls[funcid].push_back( Call() ); // insert a new call
    currCall = &( AllCalls[funcid].back() );

    //set callpath of currCall by traversing call stack
    currCall->CallPath = CallStack;
    currCall->SeqNo = GlobalCallSeqNo++;
    D2ECHO("Setting currCall on Entry for " << FUNC(funcid) << " Done" );
}

void SetCurrCallOnExit(IDNoType lastCallID)
{
    IDNoType funcid = CallStack.Top();
    D2ECHO("Setting currCall on Exit for " << FUNC(funcid) );
    if( funcid != UnknownID )
    {
        CHECK(currCall); // TODO Remove these checks
        currCall = &( AllCalls[funcid].back() );
    }

    if(
        ( FlushCalls ) &&
        ( lastCallID != UnknownID ) &&
        ( lastCallID != funcid    )     // do not flush recursive calls. These
                                        // are flushed at the end of recursion
      )
    {
        AllCalls2OneFtnType& callVector = AllCalls[lastCallID];
        u32 size = callVector.size();
        if (size > FlushCallsLimit )
        {
            PrintCalls(callVector, pcout);
            callVector.clear();
        }
    }
    D2ECHO("Setting currCall on Exit for " << FUNC(funcid) << " Done" );
}

void RecordWriteEngine3(uptr addr, u32 size)
{
    if(DoTrace)
    {
        IDNoType prod = CallStack.Top();

        D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
        IDNoType objid = GetObjectID(addr);
        D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );

        CHECK(currCall); // TODO Remove these checks
        currCall->Writes[objid]+=size;
        for(u32 i=0; i<size; i++)
        {
            SetProducer(prod, addr+i);
        }
    }
}

void RecordReadEngine3(uptr addr, u32 size)
{
    if(DoTrace)
    {
        D2ECHO("Recording Read " << VAR(size) << " at " << ADDR(addr) << dec);

        IDNoType objid = GetObjectID(addr);
        D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );
        CHECK(currCall); // TODO Remove these checks
        currCall->Reads[objid]+=size;

        //     IDNoType prod;
        //TODO do we need to record the prod to obj, and obj to cons
        // as separate entities ?
    }
}

// print a single call to a single function
void PrintCall(Call& call, ofstream& fout)
{
    // fout << "Call Seq No: " << call.SeqNo << "\n" ;
    fout << call.SeqNo << "," ;
    call.CallPath.Print(fout);

    //for ( auto& readPair : call.Reads)
    map<IDNoType,float>::iterator iter;
    for( iter = call.Reads.begin(); iter != call.Reads.end(); iter++)
    {
        IDNoType oid = iter->first;
        //fout << "Reads from " << symTable.GetSymName(oid) << ": " << iter->second << "\n" ;
        fout << ",R:" << symTable.GetSymName(oid) << ":" << iter->second;
    }

    //for ( auto& writePair : call.Writes)
    //map<IDNoType,float>::iterator iter;
    for( iter = call.Writes.begin(); iter != call.Writes.end(); iter++)
    {
        IDNoType oid = iter->first;
        //fout << "Writes to " << symTable.GetSymName(oid) << ": " << iter->second << "\n" ;
        fout << ",W:" << symTable.GetSymName(oid) << ":" << iter->second;
    }
    fout << "\n" ;
}

// print all calls to a single function
void PrintCalls(AllCalls2OneFtnType& calls, ofstream& fout)
{
    //u32 totalCalls = calls.size();
    //fout <<"Total Calls : " << totalCalls << "\n";

    vector<Call>::iterator iter;
    //for ( auto& call : calls)
    for(iter=calls.begin(); iter!=calls.end(); iter++)
    {
        PrintCall(*iter, fout);
    }
}

// print all call to all functions
void PrintAllCalls(ofstream& fout)
{
    //fout << "Printing All Calls\n";
    //for ( auto& callpair :AllCalls)
    AllCalls2AllFtnsType::iterator iter;
    for(iter=AllCalls.begin(); iter!=AllCalls.end(); iter++)
    {
        //IDNoType fid = iter->first;
        //fout << "Printing Calls to " << symTable.GetSymName(fid) << "\n";
        PrintCalls(iter->second, fout);
    }
}

