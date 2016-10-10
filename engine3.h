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

#ifndef ENGINE3_H
#define ENGINE3_H

#include "globals.h"
#include "callstack.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

// a single call to a single func
struct Call
{
    CallStackType CallPath;     // call path to this ftn, recorded once on func entry
    map<IDNoType,float> Reads;  // all the  read accesses in this call, updated per access
    map<IDNoType,float> Writes; // all the write accesses in this call, updated per access
    u64 SeqNo;                  // TODO use it later for temporal aspect of a call
    // This should be incremented on each func entry, so that
    // each function call has a unique sequence no
};

void SetCurrCallOnEntry();
void SetCurrCallOnExit(IDNoType lastCallID);

void RecordWriteEngine3(uptr addr, u32 size);
void RecordReadEngine3(uptr addr, u32 size);

void PrintAllCalls(ofstream& fout);

#endif
