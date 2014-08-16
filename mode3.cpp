#include "globals.h"
#include "shadow.h"
#include "mode3.h"
#include "commatrix.h"
#include "objects.h"

extern Objects objTable;

// all calls to a single func, index is call no
typedef vector<Call> AllCalls2OneFtnType;

// all calls to all functions, index is the funcID
typedef map<u16,AllCalls2OneFtnType> AllCalls2AllFtnsType;

AllCalls2AllFtnsType AllCalls;

Call* currCall; // pointer to current call
/*
whenever a func is entered, a new call is created
This pointer then points to that new call, hence
it is not required to determine the right Call
on read/write access
*/

void SetCurrCall(string& fname)
{
    ECHO("Setting currCall for " << fname);
    int funcid = Name2ID[fname];
    ECHO(VAR(funcid));
    
    auto it = AllCalls.find(funcid);
    if( it == AllCalls.end() )
    {
        AllCalls[funcid] = AllCalls2OneFtnType();
    }
    
    Call newCall;
    AllCalls[funcid].push_back(newCall); // add a new call
    currCall = &( AllCalls[funcid].back() );
    ECHO(VAR(currCall));
}

void RecordWriteMode3(FtnNo prod, uptr addr, int size)
{
    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    int objid = objTable.GetID(addr);
    D2ECHO( ADDR(addr) << " " << ID2Name[objid] << "(" << objid << ")" );
    
    // TODO check weather we need to some thing special for unknown objects
//     if(objid != UnknownID)
//     {
//         
//     }

    currCall->Writes[objid]+=size;
    for(int i=0; i<size; i++)
    {
        SetProducer(prod, addr+i);
    }
}

void RecordReadMode3(FtnNo cons, uptr addr, int size)
{
    D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
//     FtnNo prod;
    int objid = objTable.GetID(addr);
    D2ECHO( ADDR(addr) << " " << ID2Name[objid] << "(" << objid << ")" );
    currCall->Reads[objid]+=size;
    
    //TODO do we need to record the prod to obj, and obj to cons
    // as separate entities ?
}


// print all calls to a function
void PrintCalls(int funcid)
{
    int totalCalls = AllCalls[funcid].size();
    ECHO("Total Calls to " << ID2Name[funcid] << totalCalls);
}

void PrintAllCalls()
{
    u16 f=0;
    for ( auto calls :AllCalls)
    {
        {
            
            ECHO("Total Calls to " << ID2Name[f] << " : " <<  (calls.second).size() );
//         for ( auto& calls : callsOneFtn )
//         {
//             
//         }
        }
        f++;
    }
}