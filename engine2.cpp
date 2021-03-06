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
#include "engine2.h"
#include "symbols.h"
#include "callstack.h"
#include "sparsematrix.h"

extern CallStackType CallStack;
extern SparseMatrix DependMatrix;

extern Symbols symTable;
extern bool TrackObjects;
extern bool DoTrace;
extern u32 LoopIterationCount;

extern void *WriteRecorder;
extern void *ReadRecorder;

void RecordWriteEngine2(uptr addr, u32 size)
{
    if(DoTrace)
    {
        IDNoType prod = LoopIterationCount;
        D1ECHO("Recording Write of  " << VAR(size) << " by " << prod << " at " << ADDR(addr) );
        for(u32 i=0; i<size; i++)
        {
            SetProducer(prod, addr+i);
        }
    }
}

void RecordReadEngine2(uptr addr, u32 size)
{
    if(DoTrace)
    {
        IDNoType prod;
        IDNoType cons = LoopIterationCount;
        D1ECHO("Iteration " << LoopIterationCount << ", Recording comm of " << VAR(size) << " b/w " << GetProducer(addr) << " and " << cons << ADDR(addr) << dec);

        for(u32 i=0; i<size; i++)
        {
            prod = GetProducer(addr+i);
            DependMatrix.RecordCommunication(prod, cons, 1);
        }
    }
}


void RecordWriteEngine2Debug(uptr addr, u32 size, ADDRINT insName, VOID *ip)
{
    if(DoTrace)
    {
        IDNoType prod = LoopIterationCount;

        const char* iname = reinterpret_cast<const char *>(insName);
        string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
        string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);
        ECHO("Recording Write of  " << VAR(size) << " by " << prod << " at " << ADDR(addr) << " ins = " << iname << "; in routine " << rname);

        for(u32 i=0; i<size; i++)
        {
            SetProducer(prod, addr+i);
        }
    }
    else
    {
        D2ECHO("Not Tracing Read as DoTrace is false" );
    }
}

void RecordReadEngine2Debug(uptr addr, u32 size, ADDRINT insName, VOID *ip)
{
    if(DoTrace)
    {
        IDNoType prod;
        IDNoType cons = LoopIterationCount;

        const char* iname = reinterpret_cast<const char *>(insName);
        string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
        string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);
        ECHO("Iteration " << LoopIterationCount << " Recording comm of " << VAR(size) << " b/w " << GetProducer(addr) << " and " << cons << ADDR(addr) << dec << " ins = " << iname << "; in routine " << rname);

        for(u32 i=0; i<size; i++)
        {
            prod = GetProducer(addr+i);
            D2ECHO("Recording comm b/w " << FUNC(prod) << " and " << FUNC(cons) << dec);
            DependMatrix.RecordCommunication(prod, cons, 1);
        }
    }
    else
    {
        D2ECHO("Not Tracing Read as DoTrace is false" );
    }
}

