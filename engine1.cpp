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
#include "engine1.h"
#include "commatrix.h"
#include "callstack.h"
#include "symbols.h"

#include <algorithm>
#include <iomanip>

extern CallStackType CallStack;
extern Symbols symTable;

class Access
{
public:
    float Reads;
    float Writes;
    float Total;
    Access() : Reads(0), Writes(0), Total(0) {}
};

bool _sortByReads (const Access &lhs, const Access &rhs)
{
    return lhs.Reads > rhs.Reads;
}
bool _sortByWrites(const Access &lhs, const Access &rhs)
{
    return lhs.Writes > rhs.Writes;
}
bool _sortByTotal (const Access &lhs, const Access &rhs)
{
    return lhs.Total > rhs.Total;
}

class Accesses
{
private:
    map<IDNoType,Access> _Accesses;

public:
    Accesses() {}
    void UpdateWrites(IDNoType prod, u32 size)
    {
        _Accesses[prod].Writes += size;   // on later accesses, increment the writes by size
    }
    void UpdateReads(IDNoType cons, u32 size)
    {
        _Accesses[cons].Reads += size;   // on later accesses, increment the writes by size
    }
    void UpdateTotal()
    {
    //for(auto& pair : _Accesses)
    map<IDNoType,Access>::iterator iter;
    for( iter = _Accesses.begin(); iter != _Accesses.end(); iter++)
//     for(Access& pair : _Accesses)
        {
            auto& elem = iter->second;
            elem.Total = elem.Reads + elem.Writes;
        }
    }

    // this bash command can be used to sort by total access:
    //      tail -n +7 memProfile.out | sort -k2 -gr
//     void SortByTotal()
//     {
//         sort(_Accesses.begin(), _Accesses.end(), _sortByTotal);
//     }

    void Print(ofstream& fout)
    {
        fout << " This table can be sorted by Total Accesses (-k2) by using bash command:"<<endl;
        fout << "    tail -n +7 memProfile.out | sort -k2 -gr" <<endl<<endl;

        fout << setw(45) << "Function Name " << "\t ================= Accesses  ============  Allocation" <<endl;
        fout << setw(45) << "  " << setw(14) << "Total" << setw(14) << "Reads" << setw(14) << "Writes "<< "      Path" << endl;
        fout << "                         ==========================================================================" <<endl;

        //for(auto& pair : _Accesses)
        map<IDNoType,Access>::iterator iter;
        for( iter = _Accesses.begin(); iter != _Accesses.end(); iter++)
        {
            auto& id = iter->first;
            auto& elem = iter->second;
            fout << setw(45) << symTable.GetSymName(id)
                 << setw(14) << elem.Total
                 << setw(14) << elem.Reads
                 << setw(14) << elem.Writes
                 << "  "     << symTable.GetSymLocation(id) << endl;
        }
    }
};

// This will hold all the reads and writes for all functions
Accesses TotalAccesses;

void RecordWriteEngine1(uptr addr, u32 size)
{
    IDNoType prod = CallStack.Top();
    IDNoType oid = GetObjectID(addr);
    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    D2ECHO("Recording Write:  " << VAR(size) << FUNC(oid) << ADDR(addr));
    TotalAccesses.UpdateWrites(prod, size);
    TotalAccesses.UpdateWrites(oid, size);
}

void RecordReadEngine1(uptr addr, u32 size)
{
    IDNoType cons = CallStack.Top();
    IDNoType oid = GetObjectID(addr);
    D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
    D2ECHO("Recording Read " << VAR(size) << FUNC(oid) << ADDR(addr) << dec);
    TotalAccesses.UpdateReads(cons, size);
    TotalAccesses.UpdateReads(oid, size);
}

void PrintAccesses()
{
    ofstream fout;
    OpenOutFile("memProfile.out", fout);
    TotalAccesses.UpdateTotal();
//     TotalAccesses.SortByTotal();
    TotalAccesses.Print(fout);
    fout.close();
}

