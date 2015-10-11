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

#ifndef CALLSTACK_H
#define CALLSTACK_H

#include "globals.h"

#include <vector>
#include <cstddef>
#include <iomanip>
#include <iostream>

using namespace std;

class CallStackType
{
private:
    vector<u16> stack;  // vector containing func ids of functions on call stack

public:
    CallStackType() {}
    u16 Top()
    {
        return stack.back();
    }
    bool Empty()
    {
        return stack.empty();
    }
    void Push(u16 f)
    {
        stack.push_back(f);
    }
    void Pop()
    {
        stack.pop_back();
    }
    void Print();
    void Print(ofstream& fout);
};

class CallSiteStackType
{
private:
    vector<u32> sites;  // vector containing location indexes of call sites

public:
    CallSiteStackType() {} // TODO Do we need to insert the first call site as for main()
    bool Empty()
    {
        return sites.empty();
    }
    void Push(u32 locidx)
    {
        sites.push_back(locidx);
    }
    void Pop()
    {
        sites.pop_back();
    }
    u16 Top()
    {
        return sites.back();
    }
    void GetCallSites(u32 lastCallLocIndex, string& callsites);
    string GetCallSitesString();
    void Print();
    void Print(ofstream& fout);
};

#endif
