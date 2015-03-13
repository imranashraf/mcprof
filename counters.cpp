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
#include "counters.h"
#include "commatrix.h"
#include "symbols.h"
#include "callstack.h"

map<IDNoType,u64> instrCounts;
map<IDNoType,u64> callCounts;
extern Symbols symTable;
extern bool ShowUnknown;

void PrintInstrCount()
{
    map<IDNoType,u64>::iterator iter;
    cout << setw(30) << "Function Name" << setw(12) << "# Instr." << endl;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++ )
    {
        IDNoType fid =  iter->first;
        if(!ShowUnknown && fid==UnknownID)
            continue;
        else
        {
            u64 instrs = iter->second;
            cout << setw(30) << symTable.GetSymName(fid) << setw(10) << instrs << endl;
        }
    }
    cout << setw(30) << "Total" << setw(10) << TotalInstrCount() << endl;
}

void PrintInstrPercents()
{
    map<IDNoType,u64>::iterator iter;
    cout << setw(30) << "Function Name" << setw(12) << "% Instr." << endl;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++ )
    {
        IDNoType fid =  iter->first;
        if(!ShowUnknown && fid==UnknownID)
            continue;
        else
        {
            cout << setw(30) << symTable.GetSymName(fid) << setw(10) << GetInstrCountPercent(fid) << endl;
        }
    }
}

u64 TotalInstrCount()
{
    map<IDNoType,u64>::iterator iter;
    u64 total = 0;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++)
    {
        IDNoType fid = iter->first;
        if(!ShowUnknown && fid==UnknownID)
            continue;
        else
            total += (iter->second);
    }
    return total;
}

u64 MaxInstrCount()
{
    map<IDNoType,u64>::iterator iter;
    u64 max = 0;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++)
    {
        IDNoType fid = iter->first;
        if(!ShowUnknown && fid==UnknownID)
            continue;
        else
        {
            if( max < (iter->second) )
                max = iter->second;
        }
    }
    return max;
}

float GetInstrCountPercent(IDNoType fid)
{
    u64 count = instrCounts[fid];
    return floor( 100.0 * count / TotalInstrCount() );
}


u64 GetCallCount(IDNoType fid)
{
    //TODO what if the entry for an fid does not exist?
    return callCounts[fid];
}
