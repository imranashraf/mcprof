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
#include "engine2.h"
#include "commatrix.h"
#include "symbols.h"
#include "callstack.h"

extern CallStackType CallStack;
extern Matrix2D ComMatrix;

extern Symbols symTable;
extern bool TrackObjects;
extern bool DoTrace;

extern std::ofstream traceout;
extern UINT64 gInstrCount;

extern map<IDNoType,u64> funcReads;
extern map<IDNoType,u64> funcWrites;
extern map<IDNoType,u64> objReads;
extern map<IDNoType,u64> objWrites;

// un-comment the following to generate read/write traces
// #define GENRATE_TRACES

// un-comment the following to generate selected read/write traces
// #define GENRATE_SELECTED_TRACES

void RecordWriteEngine2(uptr addr, u32 size)
{
    if(DoTrace)
    {
        IDNoType prod = CallStack.Top();
        IDNoType objid = GetObjectID(addr);

        D2ECHO("Recording Write of  " << VAR(size) << " by " << FUNC(prod) << " at " << ADDR(addr));

#ifdef GENRATE_TRACES
        // Generate Write Trace
        switch (size)
        {
            case 1:
                traceout << gInstrCount << " W " << size << " " << HEXA(addr)  << " " << HEXV( *((u8*)addr)) << endl;
            break;
            case 2:
                traceout << gInstrCount << " W " << size << " " << HEXA(addr)  << " " << HEXV( *((u16*)addr)) << endl;
            break;
            case 4:
                traceout << gInstrCount << " W " << size << " " << HEXA(addr)  << " " << HEXV( *((u32*)addr)) << endl;
            break;
            case 8:
                traceout << gInstrCount << " W " << size << " " << HEXA(addr)  << " " << HEXV( *((u64*)addr)) << endl;
            break;
            default:
                ECHO("traceout, write size is : " << size );
            break;
        }
#endif

#if 1
        // Added for allocation dependencies
        // TODO this can be a problem when same stack addresses are reused
        // these will appear as write after write dependencies
        for(u32 i=0; i<size; i++)
        {
            IDNoType prevProd = GetProducer(addr+i);
            ComMatrix.RecordCommunication(prevProd, prod, 1);
            D2ECHO( "AllocDepend " << FUNC(prevProd) << " " << FUNC(prod) << " at " << ADDR(addr) );
        }
#endif

        if( (objid == UnknownID) || (TrackObjects == false) )
        {
            for(u32 i=0; i<size; i++)
            {
                SetProducer(prod, addr+i);
            }

            // Update write memory accesses
            funcWrites[prod] += size;
        }
        else
        {
            D2ECHO("Recording comm of " << VAR(size) << " b/w " << FUNC(prod)
                    << " and " << symTable.GetSymName(objid) << dec);
            for(u32 i=0; i<size; i++)
            {
                SetProducer(prod, addr+i);
            }
            ComMatrix.RecordCommunication(prod, objid, size);

            // Update write memory accesses
            funcWrites[prod] += size;
            objWrites[objid] += size;

#ifdef GENRATE_SELECTED_TRACES
            // Generate Write Trace of a selected function to selected objects
            if(
                    // For canny: tmpimg objects(10) AND gaussian_smooth1(11) function.
                    // For canny: nms(20) object AND non_max_supp1(22) function.
                    (objid==20) && (prod==22)
              )
            {
                traceout << "W of "<< size << " to " << objid << " by " << prod << " at " << HEXA(addr) << endl;
            }
#endif
        }
    }
}

void RecordReadEngine2(uptr addr, u32 size)
{
    if(DoTrace)
    {
        IDNoType cons = CallStack.Top();
        D2ECHO("Recording Read of " << VAR(size) << " by " << FUNC(cons) << " at " << ADDR(addr) << dec);

#ifdef GENRATE_TRACES
        // Generate Read Trace
        switch (size)
        {
            case 1:
                traceout << gInstrCount << " R " << size << " " << HEXA(addr)  << " " << HEXV( *((u8*)addr)) << endl;
            break;
            case 2:
                traceout << gInstrCount << " R " << size << " " << HEXA(addr)  << " " << HEXV( *((u16*)addr)) << endl;
            break;
            case 4:
                traceout << gInstrCount << " R " << size << " " << HEXA(addr)  << " " << HEXV( *((u32*)addr)) << endl;
            break;
            case 8:
                traceout << gInstrCount << " R " << size << " " << HEXA(addr)  << " " << HEXV( *((u64*)addr)) << endl;
            break;
            default:
                ECHO("traceout, write size is : " << size );
            break;
        }
#endif

        IDNoType prod=0;
        IDNoType objid = GetObjectID(addr);
        D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );

        if( objid == UnknownID || (TrackObjects == false) )
        {
            D2ECHO("Recording comm of " << VAR(size) << " b/w function "
                    << FUNC( GetProducer(addr) ) << " and " << FUNC(cons) << dec);

            for(u32 i=0; i<size; i++)
            {
                prod = GetProducer(addr+i);
                ComMatrix.RecordCommunication(prod, cons, 1);
            }

            // Update read memory accesses
            funcReads[cons] += size;
        }
        else
        {
            D2ECHO("Recording comm of " << VAR(size) << " b/w object "
                    << symTable.GetSymName(objid) << " and " << FUNC(cons) << dec);

            ComMatrix.RecordCommunication(objid, cons, size);

            // Update read memory accesses
            funcReads[cons] += size;
            objReads[objid] += size;

#ifdef GENRATE_SELECTED_TRACES
            // Generate Read Trace by a selected function from selected objects
            if(
                    // For canny: image(4) OR kernel(9) objects AND gaussian_smooth1(11) function.
                    // For canny: magnitude(18) OR delta_x(13) OR delta_y(14) objects AND non_max_supp1(22) function.
                    (objid==18 || objid==13 || objid==14) && (cons==22)
              )
            {
                traceout << "R of "<< size << " from " << objid << " by " << cons << " at " << HEXA(addr) << endl;
            }
#endif
        }
    }
}

