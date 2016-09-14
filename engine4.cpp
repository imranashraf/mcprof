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
#include "engine4.h"
#include "commatrix.h"
#include "symbols.h"
#include "callstack.h"

extern CallStackType CallStack;
extern Matrix2D ComMatrix;

extern Symbols symTable;
extern bool TrackObjects;
extern bool NoseDown;
extern u32 LoopIterationCount;

extern void *WriteRecorder;
extern void *ReadRecorder;

void RecordWriteEngine4(uptr addr, u32 size)
{
    IDNoType prod = CallStack.Top();
    D2ECHO("Recording Write of  " << VAR(size) << " by " << prod << " at " << ADDR(addr) );
    for(u32 i=0; i<size; i++)
    {
        SetProducer(prod, addr+i);
    }
}

void RecordReadEngine4(uptr addr, u32 size)
{
    IDNoType prod;
    IDNoType cons = CallStack.Top();
    D2ECHO("Recording comm of " << VAR(size) << " b/w " << GetProducer(addr) << " and " << cons << ADDR(addr) << dec);

    for(u32 i=0; i<size; i++)
    {
        prod = GetProducer(addr+i);
        ComMatrix.RecordCommunication(prod, cons, 1);
    }
}


void RecordWriteEngine4Debug(uptr addr, u32 size, ADDRINT insName, VOID *ip)
{
    if( ReadRecorder == RecordReadEngine4 ) // to instrument when analysis engine is selected
    {
        IDNoType prod = CallStack.Top();

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
        D2ECHO("Not Instrumenting Read as dummy recorder " );
    }
}

void RecordReadEngine4Debug(uptr addr, u32 size, ADDRINT insName, VOID *ip)
{
    if( WriteRecorder == RecordWriteEngine4 ) // to instrument when analysis engine is selected
    {
        IDNoType prod;
        IDNoType cons = CallStack.Top();

        const char* iname = reinterpret_cast<const char *>(insName);
        string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
        string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);
        ECHO("Recording comm of " << VAR(size) << " b/w " << GetProducer(addr) << " and " << cons << ADDR(addr) << dec << " ins = " << iname << "; in routine " << rname);

        for(u32 i=0; i<size; i++)
        {
            prod = GetProducer(addr+i);
            D2ECHO("Recording comm b/w " << FUNC(prod) << " and " << FUNC(cons) << dec);
            ComMatrix.RecordCommunication(prod, cons, 1);
        }
    }
    else
    {
        D2ECHO("Not Instrumenting Read as dummy recorder " );
    }
}
