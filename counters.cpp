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
 * Copyright (c) 2014-2016 TU Delft, The Netherlands.
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

map<IDNoType,u64> funcReads;
map<IDNoType,u64> funcWrites;
map<IDNoType,u64> objReads;
map<IDNoType,u64> objWrites;

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
    ofstream fout;
    OpenOutFile("execProfile.dat", fout);

    multimap<u64, IDNoType> instrCountsSorted = flipMap(instrCounts);
    multimap<u64,IDNoType>::reverse_iterator iter;
    fout << setw(10) << "Total Calls " << setw(15) << "%Exec.Instr." << "\t\t" << "Name" << endl;
    fout << " #==================================================================" <<endl;

    for( iter = instrCountsSorted.rbegin(); iter != instrCountsSorted.rend(); ++iter )
    {
        IDNoType fid =  iter->second;
        if(!ShowUnknown && fid==UnknownID)
            continue;
        else
        {
            fout << setw(10) << GetCallCount(fid)
                 << setw(15) << GetInstrCountPercent(fid)
                 << "\t\t" << symTable.GetSymName(fid) << endl;
        }
    }
    fout.close();
}

// The following prints the map without sorting
// void PrintInstrPercents()
// {
//     ofstream fout;
//     OpenOutFile("execProfile.dat", fout);
//
//     map<IDNoType,u64>::iterator iter;
//     fout << setw(45) << "Function Name" << setw(12) << "% Instr." << endl;
//     for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++ )
//     {
//         IDNoType fid =  iter->first;
//         if(!ShowUnknown && fid==UnknownID)
//             continue;
//         else
//         {
//             fout << setw(45) << symTable.GetSymName(fid) << setw(10) << GetInstrCountPercent(fid) << endl;
//         }
//     }
//     fout.close();
// }

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

u64 GetInstrCount(IDNoType fid)
{
    return instrCounts[fid];
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

void PrintMemAccesses()
{
    ofstream fout;
    OpenOutFile("memProfile.dat", fout);

    fout << " This table can be sorted by Total Accesses (-k2) by using bash command:"<<endl;
    fout << "    tail -n +7 memProfile.dat | sort -k2 -gr" <<endl<<endl;

    fout << " Per Function Flat Memory Profile \n";
    fout << setw(45) << " Function Name " << "\t ================= Accesses  ============  Allocation" <<endl;
    fout << setw(45) << "  " << setw(14) << "Total" << setw(14) << "Reads" << setw(14) << "Writes "<< "      Path" << endl;
    fout << "                         ==========================================================================" <<endl;

    map<IDNoType,u64>::iterator fiter;
    for( fiter = funcReads.begin(); fiter != funcReads.end(); ++fiter)
    {
        auto fid = fiter->first;
        auto freads = fiter->second;
        auto fwrites = funcWrites[fid];
        fout << setw(45) << symTable.GetSymName(fid)
                << setw(14) << freads + fwrites
                << setw(14) << freads
                << setw(14) << fwrites
                << "  "     << symTable.GetSymLocation(fid) << endl;
    }

    fout << "\n\n";
    fout << " Per Object Flat Memory Profile \n";
    fout << setw(45) << " Object Name " << "\t ================= Accesses  ============  Allocation" <<endl;
    fout << setw(45) << "  " << setw(14) << "Total" << setw(14) << "Reads" << setw(14) << "Writes "<< "      Path" << endl;
    fout << "                         ==========================================================================" <<endl;

    map<IDNoType,u64>::iterator oiter;
    for( oiter = objReads.begin(); oiter != objReads.end(); ++oiter)
    {
        auto oid = oiter->first;
        auto oreads = oiter->second;
        auto owrites = objWrites[oid];
        fout << setw(45) << symTable.GetSymName(oid)
                << setw(14) << oreads + owrites
                << setw(14) << oreads
                << setw(14) << owrites
                << "  "     << symTable.GetSymLocation(oid) << endl;
    }

    fout.close();
}
