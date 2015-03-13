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
#include "commatrix.h"

// Three modes of operation
#define TABLES 1
#define HYBRID 2
// Un-comment *ONLY ONE* of the following three to select Mode
#define MODE TABLES
// #define MODE HYBRID

// In hybrid mode ONLY, setting for stack/heap or both should be coverage
#define HYBRID4STACK    1
#define HYBRID4HEAP     2
#define HYBRID4BOTH     3
// Un-comment *ONLY ONE* of the following three to select Mode
// #define HYBRIDTYPE HYBRID4STACK
// #define HYBRIDTYPE HYBRID4HEAP
#define HYBRIDTYPE HYBRID4BOTH



#if (MODE==HYBRID)
MemMap1to4 ShadowMem;
#endif

#if ( (MODE==HYBRID) || (MODE==TABLES) )
L3Table ShadowTable;
#endif

uptr tableCounter=0;
uptr memmapCounter=0;
void PrintShadowMap()
{
#if (MODE==HYBRID)
    ShadowMem.Print();
    ECHO(VAR(tableCounter));
    ECHO(VAR(memmapCounter));
#else
    ECHO("NOT Using Mem Map as in TABLES Mode");
#endif
}


/***************************************************************************/
#if (MODE==TABLES)
void SetProducer(IDNoType fid, uptr addr)
{
    D2ECHO("Setting " << FUNC(fid) << " as producer of " << ADDR(addr));
    Entry* entry = ShadowTable.getEntry(addr);
    entry->funcID = fid;
    //entry->threadID = tid;
}
#endif

#if (MODE==TABLES)
void SetProducers(uptr saddr, u32 size, IDNoType fid)
{
    //TODO we need to use memset, secondly we need to take care if
    // addr does not lie in this table (in current non-optimal way its not a problem)
    for(uptr addr = saddr; addr < saddr+size; addr++)
    {
        Entry* entry = ShadowTable.getEntry(addr);
        entry->funcID = fid;
    }
}
#endif

#if (MODE==TABLES)
void SetObjectIDs(uptr saddr, u32 size, IDNoType id)
{
    //TODO we need to use memset, secondly we need to take care if
    // addr does not lie in this table (in current non-optimal way its not a problem)
    for(uptr addr = saddr; addr < saddr+size; addr++)
    {
        Entry* entry = ShadowTable.getEntry(addr);
        entry->objID = id;
    }
}
#endif

#if (MODE==TABLES)
IDNoType GetProducer(uptr addr)
{
    IDNoType id;
    Entry* entry = ShadowTable.getEntry(addr);
    id = entry->funcID;
    D2ECHO("Got producer of " << ADDR(addr) << " as " << FUNC(id));
    return id;
}
#endif

#if (MODE==TABLES)
IDNoType GetObjectID(uptr addr)
{
    IDNoType oid;
    Entry* entry = ShadowTable.getEntry(addr);
    oid = entry->objID;
    D2ECHO("Got producer of " << ADDR(addr) << " as " << FUNC(oid));
    return oid;
}
#endif

/***************************************************************************/
#if (MODE==HYBRID)
void SetProducer(IDNoType pid, uptr addr)
{
    D2ECHO("Setting " << FUNC(pid) << " as producer of " << ADDR(addr));

    #if(HYBRIDTYPE == HYBRID4STACK)
    if ( addr > MemMap1to4::M1L ) // ONLY STACK
    #endif
    #if(HYBRIDTYPE == HYBRID4HEAP)
    if ( (addr < MemMap1to4::M0H) ) // ONLY HEAP
    #endif
    #if(HYBRIDTYPE == HYBRID4BOTH)
    if ( (addr < MemMap1to4::M0H) || (addr > MemMap1to4::M1L) ) // HEAP+STACK
    #endif
    {
        Entry* entry = (Entry*) ShadowMem.Mem2Shadow(addr);
        entry->funcID = pid;
    }
    else
    {
        Entry* entry = ShadowTable.getEntry(addr);
        entry->funcID = pid;
    }
}
#endif

#if (MODE==HYBRID)
void SetProducers(uptr saddr, u32 size, IDNoType pid)
{
    #if(HYBRIDTYPE == HYBRID4STACK)
    if ( saddr > MemMap1to4::M1L ) // ONLY STACK
    #endif
    #if(HYBRIDTYPE == HYBRID4HEAP)
    if ( (saddr < MemMap1to4::M0H) ) // ONLY HEAP
    #endif
    #if(HYBRIDTYPE == HYBRID4BOTH)
    if ( (saddr < MemMap1to4::M0H) || (saddr > MemMap1to4::M1L) ) // HEAP+STACK
    #endif
    {
        for(uptr addr = saddr; addr < saddr+size; addr++)
        {
            Entry* entry = (Entry*) ShadowMem.Mem2Shadow(addr);
            entry->funcID = pid;
        }
    }
    else
    {
        for(uptr addr = saddr; addr < saddr+size; addr++)
        {
            Entry* entry = ShadowTable.getEntry(addr);
            entry->funcID = pid;
        }
    }
}
#endif

#if (MODE==HYBRID)
void SetObjectIDs(uptr saddr, u32 size, IDNoType oid)
{
    #if(HYBRIDTYPE == HYBRID4STACK)
    if ( saddr > MemMap1to4::M1L ) // ONLY STACK
    #endif
    #if(HYBRIDTYPE == HYBRID4HEAP)
    if ( (saddr < MemMap1to4::M0H) ) // ONLY HEAP
    #endif
    #if(HYBRIDTYPE == HYBRID4BOTH)
    if ( (saddr < MemMap1to4::M0H) || (saddr > MemMap1to4::M1L) ) // HEAP+STACK
    #endif
    {
        for(uptr addr = saddr; addr < saddr+size; addr++)
        {
            Entry* entry = (Entry*) ShadowMem.Mem2Shadow(addr);
            entry->objID = oid;
        }
    }
    else
    {
        for(uptr addr = saddr; addr < saddr+size; addr++)
        {
            Entry* entry = ShadowTable.getEntry(addr);
            entry->objID = oid;
        }
    }
}
#endif

#if (MODE==HYBRID)
IDNoType GetObjectID(uptr addr)
{
    IDNoType oid;

    #if(HYBRIDTYPE == HYBRID4STACK)
    if ( addr > MemMap1to4::M1L ) // ONLY STACK
    #endif
    #if(HYBRIDTYPE == HYBRID4HEAP)
    if ( (addr < MemMap1to4::M0H) ) // ONLY HEAP
    #endif
    #if(HYBRIDTYPE == HYBRID4BOTH)
    if ( (addr < MemMap1to4::M0H) || (addr > MemMap1to4::M1L) ) // HEAP+STACK
    #endif
    {
        memmapCounter++;
        Entry* entry = (Entry*) ShadowMem.Mem2Shadow(addr);
        oid = entry->objID;
    }
    else
    {
        tableCounter++;
        Entry* entry = ShadowTable.getEntry(addr);
        oid = entry->objID;
    }
    return oid;
}
#endif


#if (MODE==HYBRID)
IDNoType GetProducer(uptr addr)
{
    IDNoType pid;
    D2ECHO(ADDR(addr) << ADDR(MemMap1to4::M0H) << ADDR(MemMap1to4::M1L) );

    #if(HYBRIDTYPE == HYBRID4STACK)
    if ( addr > MemMap1to4::M1L ) // ONLY STACK
    #endif
    #if(HYBRIDTYPE == HYBRID4HEAP)
    if ( (addr < MemMap1to4::M0H) ) // ONLY HEAP
    #endif
    #if(HYBRIDTYPE == HYBRID4BOTH)
    if ( (addr < MemMap1to4::M0H) || (addr > MemMap1to4::M1L) ) // HEAP+STACK
    #endif
    {
        Entry* entry = (Entry*) ShadowMem.Mem2Shadow(addr);
        pid = entry->funcID;
    }
    else
    {
        Entry* entry = ShadowTable.getEntry(addr);
        pid = entry->funcID;
    }

    D2ECHO("Got producer of " << ADDR(addr) << " as " << FUNC(pid));
    return pid;
}
#endif
