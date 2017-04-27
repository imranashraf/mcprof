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

#ifndef SHADOW_H
#define SHADOW_H

#include "globals.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>

using namespace std;

static const uptr MB   = (1ULL << 20);
static const uptr GB   = (1ULL << 30);
static const uptr TB   = (1ULL << 40);

// No of bits required to address table entries
static const u64 L1BITS = (16);
static const u64 L2BITS = (16);
static const u64 L3BITS = (16);

// Sizes of tables
static const u64 L1ENTRIES = (1ULL << L1BITS);
static const u64 L2ENTRIES = (1ULL << L2BITS);
static const u64 L3ENTRIES = (1ULL << L3BITS);

// #define UNACCESSED nullptr
#define UNACCESSED NULL

void SetProducer(IDNoType fid, uptr addr);
IDNoType GetProducer(uptr addr);
IDNoType GetObjectID(uptr addr);
void SetObjectIDs(uptr saddr, u32 size, IDNoType id);
void SetProducers(uptr saddr, u32 size, IDNoType fid);
void PrintShadowMap();

// (AE/PE)
void SetLastConsumer(IDNoType cid, uptr addr);
IDNoType GetLastConsumer(uptr addr);
void SetLastConsumers(uptr saddr, u32 size, IDNoType cid);
void GetAEPECount(uptr saddr, u32 size, u64 &ae, u64 &pe);

struct Entry
{
    IDNoType prodID;
    IDNoType objID;
    IDNoType lastConsID; // (AE/PE)
    //IDNoType prodThreadID;
};

class L1Table
{
private:
    Entry Table[L1ENTRIES];
public:
    L1Table() { }
    Entry* getEntry(uptr L1Index)
    {
        return &Table[L1Index];
    }
};

class L2Table
{
private:
    L1Table* Table[L2ENTRIES];
public:
    L2Table()
    {
        // initially all the entries are UNACCESSED
        for(u64 i=0; i<L2ENTRIES; i++)
        {
            Table[i]=UNACCESSED;
        }
    }

    Entry* getEntry(uptr L1Index, uptr L2Index)
    {
        if(Table[L2Index] == UNACCESSED)
        {
            Table[L2Index] = new L1Table;
        }
        return Table[L2Index]->getEntry(L1Index);
    }
};

class L3Table
{
private:
    L2Table* Table[L3ENTRIES];
public:
    L3Table()
    {
        // initially all the entries are UNACCESSED
        for(u64 i=0; i<L3ENTRIES; i++)
        {
            Table[i]=UNACCESSED;
        }
    }

    Entry* getEntry(uptr addr)
    {
        u64 L1Index=( (L1ENTRIES-1) & addr);
        u64 L2Index=( (L2ENTRIES-1) & (addr>>L1BITS) );
        u64 L3Index=( (L3ENTRIES-1) & (addr>>(L1BITS+L2BITS)) );
        if(Table[L3Index] == UNACCESSED)
        {
            Table[L3Index] = new L2Table;
        }
        return Table[L3Index]->getEntry(L1Index, L2Index);
    }
};


/**
 * ***************
 **/
// 1-4 mapping
class MemMap1to4
{
// private:
public:
    static const uptr SCALE = 4;
    static const uptr LOGSCALE =2; //log2(SCALE); //OLD compilers complain log2

    static const uptr M0SIZE = 2*GB;
    static const uptr M0L = 0ULL;
    static const uptr M0H = M0L + M0SIZE -1;
    //Following May obtained at runtime !!!
    static const uptr SM0SIZE = M0SIZE*SCALE;
    static const uptr SM0L = 0x400000000000ULL;
    static const uptr SM0H = SM0L + SM0SIZE -1;
    
    static const uptr M1SIZE = 2*GB;
    static const uptr M1H = (1ULL<<47) -1;
    static const uptr M1L = M1H - M1SIZE +1;
    //Following May obtained at runtime !!!
    static const uptr SM1SIZE = M1SIZE*SCALE;
    static const uptr SM1L = 0x600000000000ULL;
    static const uptr SM1H = SM1L + SM1SIZE -1;

public:
    MemMap1to4()
    {
        uptr *retAddr;
        uptr startAddr;
        uptr length;

        startAddr = SM0L; length = SM0SIZE;
        retAddr = (uptr *)mmap((void *)startAddr,
                               length,
                               PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANON | MAP_FIXED,
                               -1, 0);
        if (retAddr == MAP_FAILED)
            cout<<"mmap Failed"<<endl;

        startAddr = SM1L; length = SM1SIZE;
        retAddr = (uptr *)mmap((void *)startAddr,
                               length,
                               PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANON | MAP_FIXED,
                               -1, 0);
        if (retAddr == MAP_FAILED)
            cout<<"mmap Failed"<<endl;
    }

    void Print()
    {
        cout << hex;
        cout << "================ 0x" << setw(12) << setfill ('0') << M1H << endl;
        cout << dec << "| M1 = "<< M1SIZE/GB <<" GB    |" <<endl << hex;
        cout << "================ 0x" << setw(12) << setfill ('0') << M1L << endl<<endl<<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << SM1H << endl;
        cout << dec << "| SM1 = "<< SM1SIZE/MB <<" MB   |" <<endl << hex;
        cout << "================ 0x" << setw(12) << setfill ('0') << SM1L << endl<<endl<<endl<<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << SM0H << endl;
        cout << dec << "| SM0 = "<< SM0SIZE/MB <<" MB   |" <<endl << hex;
        cout << "================ 0x" << setw(12) << setfill ('0') << SM0L << endl<<endl<<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << M0H << endl;
        cout << dec <<"| M0 = "<< M0SIZE/GB <<" GB    |" <<endl << hex;
        cout << "================ 0x" << setw(12) << setfill ('0') << M0L << endl;
        cout << dec;
    }
    uptr inline Mem2Shadow(uptr addr)
    {
        return ( ((addr & M0H)<<LOGSCALE) + (addr & (SM1L + SM0L)) + SM0L);    
    }

    ~MemMap1to4()
    {
        int retVal;
        uptr startAddr;
        uptr length;

        startAddr = SM0L; length = SM0SIZE;
        retVal = munmap((void *)startAddr, length);
        if( retVal == -1)
            cout<<"munmap Failed"<<endl;

        startAddr = SM1L; length = SM1SIZE;
        retVal = munmap((void *)startAddr, length);
        if( retVal == -1)
            cout<<"munmap Failed"<<endl;
    }
};

#endif
