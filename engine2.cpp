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

// un-comment the following to print read/write address trace to output
// #define GENRATE_TRACES

// un-comment the following to generate selected read/write trace
// #define GENRATE_SELECTED_TRACES

void RecordWriteEngine2(uptr addr, u32 size)
{
    if(DoTrace)
    {
        IDNoType prod = CallStack.Top();
        IDNoType objid = GetObjectID(addr);

#ifdef GENRATE_TRACES
        for(u32 i=0; i<size; i++)
            cout << "W " << addr+i << endl;
#endif

        D2ECHO("Recording Write of  " << VAR(size) << " by " << FUNC(prod) << " at " << ADDR(addr));

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
        }
        else
        {
            ECHO("Recording comm of " << VAR(size) << " b/w " << FUNC(prod)
                    << " and " << symTable.GetSymName(objid) << dec);
            for(u32 i=0; i<size; i++)
            {
                SetProducer(prod, addr+i);
            }
            ComMatrix.RecordCommunication(prod, objid, size);

            // Write Trace of a selected function to selected objects
#ifdef GENRATE_SELECTED_TRACES
            if(
                    // For canny: tmpimg objects(10) AND gaussian_smooth1(11) function.
                    // For canny: nms(20) object AND non_max_supp1(22) function.
                    (objid==20) && (prod==22)
              )
            {
                cout << "W of "<< size << " to " << objid << " by " << prod << " at " << HEXA(addr) << endl;
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

        IDNoType prod=0;
        IDNoType objid = GetObjectID(addr);
        D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );

#ifdef GENRATE_TRACES
        for(u32 i=0; i<size; i++)
            cout << "R " << addr+i << endl;
#endif

        if( objid == UnknownID || (TrackObjects == false) )
        {
            D2ECHO("Recording comm of " << VAR(size) << " b/w function "
                    << FUNC( GetProducer(addr) ) << " and " << FUNC(cons) << dec);

            for(u32 i=0; i<size; i++)
            {
                prod = GetProducer(addr+i);
                ComMatrix.RecordCommunication(prod, cons, 1);
            }
        }
        else
        {
            ECHO("Recording comm of " << VAR(size) << " b/w object "
                    << symTable.GetSymName(objid) << " and " << FUNC(cons) << dec);

            ComMatrix.RecordCommunication(objid, cons, size);

            // Read Trace by a selected function from selected objects
#ifdef GENRATE_SELECTED_TRACES
            if(
                    // For canny: image(4) OR kernel(9) objects AND gaussian_smooth1(11) function.
                    // For canny: magnitude(18) OR delta_x(13) OR delta_y(14) objects AND non_max_supp1(22) function.
                    (objid==18 || objid==13 || objid==14) && (cons==22)
              )
            {
                cout << "R of "<< size << " from " << objid << " by " << cons << " at " << HEXA(addr) << endl;
            }
#endif
        }
    }
}

