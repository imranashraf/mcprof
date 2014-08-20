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
static const u64 L1BITS = (18);
static const u64 L2BITS = (16);
static const u64 L3BITS = (14);

// Sizes of tables
static const u64 L1ENTRIES = (1ULL << L1BITS);
static const u64 L2ENTRIES = (1ULL << L2BITS);
static const u64 L3ENTRIES = (1ULL << L3BITS);

void SetProducer(IDNoType prod, uptr addr);
IDNoType GetProducer(uptr addr);

#define UNACCESSED nullptr

class Entry
{
private:
    IDNoType producer;
public:
    Entry() : producer(UnknownID) {}
    void setProducer(IDNoType p)
    {
        producer=p;
    }
    IDNoType getProducer()
    {
        return producer;
    }
};

// struct Entry {
//     IDNoType producer;
// };

class L1Table
{
private:
    Entry Table[L1ENTRIES];
public:
    L1Table() { }
    IDNoType getProducer(uptr L1Index)
    {
//         return Table[L1Index].producer;
        return Table[L1Index].getProducer();
    }
    void setProducer(uptr L1Index, IDNoType prod)
    {
//         Table[L1Index].producer = prod;
        Table[L1Index].setProducer(prod);
    }
    void setProducerRange(uptr L1Index, IDNoType prod, u32 size)
    {
        for(u32 i=0; i<size; i++)
        {
//             Table[L1Index + i].producer = prod;
            Table[L1Index+i].setProducer(prod);
        }
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
    IDNoType getProducer(uptr L1Index, uptr L2Index)
    {
        if(Table[L2Index] == UNACCESSED)
        {
            Table[L2Index] = new L1Table;
        }
        return Table[L2Index]->getProducer(L1Index);
    }
    void setProducer(uptr L1Index, uptr L2Index, IDNoType prod)
    {
        if(Table[L2Index] == UNACCESSED)
        {
            Table[L2Index] = new L1Table;
        }
        Table[L2Index]->setProducer(L1Index, prod);
    }
    void setProducerRange(uptr L1Index, uptr L2Index, IDNoType prod, int size)
    {
        if(Table[L2Index] == UNACCESSED)
        {
            Table[L2Index] = new L1Table;
        }
        Table[L2Index]->setProducerRange(L1Index, prod, size);
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

    IDNoType getProducer(uptr addr)
    {
        IDNoType prod;

        u64 L1Index=( (L1ENTRIES-1) & addr);
        u64 L2Index=( (L2ENTRIES-1) & (addr>>L1BITS) );
        u64 L3Index=( (L3ENTRIES-1) & (addr>>(L1BITS+L2BITS)) );
        if(Table[L3Index] == UNACCESSED)
        {
            Table[L3Index] = new L2Table;
        }
        prod = Table[L3Index]->getProducer(L1Index, L2Index);
        return prod;
    }

    void setProducer(uptr addr, IDNoType prod)
    {
        u64 L1Index=( (L1ENTRIES-1) & addr);
        u64 L2Index=( (L2ENTRIES-1) & (addr>>L1BITS) );
        u64 L3Index=( (L3ENTRIES-1) & (addr>>(L1BITS+L2BITS)) );
        if(Table[L3Index] == UNACCESSED)
        {
            Table[L3Index] = new L2Table;
        }
        Table[L3Index]->setProducer(L1Index, L2Index, prod);
    }

    void setProducerRange(uptr addr, IDNoType prod, int size)
    {
        u64 L1Index=( (L1ENTRIES-1) & addr);
        u64 L2Index=( (L2ENTRIES-1) & (addr>>L1BITS) );
        u64 L3Index=( (L3ENTRIES-1) & (addr>>(L1BITS+L2BITS)) );
        if(Table[L3Index] == UNACCESSED)
        {
            Table[L3Index] = new L2Table;
        }
        Table[L3Index]->setProducerRange(L1Index, L2Index, prod, size);
    }

};


/**
 ****************
 **/

class MemMap
{
private:
    static const uptr M0SIZE = 2*GB;
    static const uptr M0L = 0ULL;
    static const uptr M0H = M0L + M0SIZE -1;
//Following May obtained at runtime !!!
    static const uptr SM0L = 0x400000000000ULL;
    static const uptr SM0H = SM0L + M0SIZE -1;
    static const uptr M0OFFSET = SM0L-M0L;

    static const uptr M1SIZE = 2*GB;
    static const uptr M1H = (1ULL<<47) -1;
    static const uptr M1L = M1H - M1SIZE +1;
//Following May obtained at runtime !!!
    static const uptr SM1L = 0x500000000000ULL;
    static const uptr SM1H = SM1L + M1SIZE -1;
    static const uptr M1OFFSET = M1L-SM1L;

public:
    MemMap()
    {
        uptr *retAddr;
        uptr startAddr;
        uptr length;

        startAddr = SM0L;
        length = M0SIZE;
        retAddr = (uptr *)mmap((void *)startAddr,
                               length,
                               PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANON | MAP_FIXED,
                               -1, 0);
        if (retAddr == MAP_FAILED)
            cout<<"mmap Failed"<<endl;

        startAddr = SM1L;
        length = M1SIZE;
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
        cout << "| M1 = "<< M1SIZE/GB <<" GB    |" <<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << M1L << endl<<endl<<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << SM1H << endl;
        cout << "| SM1 = "<< M1SIZE/GB <<" GB   |" <<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << SM1L << endl<<endl<<endl<<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << SM0H << endl;
        cout << "| SM0 = "<< M0SIZE/GB <<" GB   |" <<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << SM0L << endl<<endl<<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << M0H << endl;
        cout << "| M0 = "<< M0SIZE/GB <<" GB    |" <<endl;
        cout << "================ 0x" << setw(12) << setfill ('0') << M0L << endl;
        cout << dec;
    }
    uptr inline Mem2Shadow(uptr addr)
    {
        return (addr >=M0L && addr <= M0H)*(addr + M0OFFSET) +
               (addr >=M1L && addr <= M1H)*(addr - M1OFFSET) ;
    }

    ~MemMap()
    {
        int retVal;
        uptr startAddr;
        uptr length;

        startAddr = SM0L;
        length = M0SIZE;
        retVal = munmap((void *)startAddr, length);
        if( retVal == -1)
            cout<<"munmap Failed"<<endl;

        startAddr = SM1L;
        length = M1SIZE;
        retVal = munmap((void *)startAddr, length);
        if( retVal == -1)
            cout<<"munmap Failed"<<endl;
    }
};

void PrintShadowMap();
/**
 ****************
 **/

// Shadow mapping on Linux/x86_64 with SHADOW_OFFSET == 0x00007fff8000:
// || `[0x10007fff8000, 0x7fffffffffff]` || HighMem    ||
// || `[0x02008fff7000, 0x10007fff7fff]` || HighShadow ||
// || `[0x00008fff7000, 0x02008fff6fff]` || ShadowGap  ||
// || `[0x00007fff8000, 0x00008fff6fff]` || LowShadow  ||
// || `[0x000000000000, 0x00007fff7fff]` || LowMem     ||

#include <unistd.h>

#define PAGESIZE (sysconf(_SC_PAGE_SIZE))
#define SHADOW_SCALE  (3)
#define SHADOW_GRANULARITY (8)
#define SHADOW_OFFSET (0x00007fff8000ULL)
#define MEM2SHADOW(mem) (((mem) >> SHADOW_SCALE) + (SHADOW_OFFSET))

class MemMap8th
{
private:
    static const uptr M0L = 0ULL;
    static const uptr M0H = 0x00007fff7fffULL;
    static const uptr M0SIZE=(M0H-M0L);

    static const uptr SM0L = 0x00007fff8000ULL;
    static const uptr SM0H = 0x00008fff6fffULL;
    static const uptr SM0SIZE=(SM0H-SM0L);

    static const uptr SGL = 0x00008fff7000ULL;
    static const uptr SGH = 0x02008fff6fffULL;
    static const uptr SGSIZE=(SGH-SGL);

    static const uptr SM1L = 0x02008fff7000ULL;
    static const uptr SM1H = 0x10007fff7fffULL;
    static const uptr SM1SIZE=(SM1H-SM1L);

    static const uptr M1L = 0x10007fff8000ULL;
    static const uptr M1H = 0x7fffffffffffULL;
    static const uptr M1SIZE=(M1H-M1L);

public:
    MemMap8th()
    {
        uptr *retAddr;
        uptr startAddr;
        uptr length;
        D1ECHO(VAR(PAGESIZE));

        // Reserve low shadow mem
        startAddr = SM0L;
        length = (SM0H-SM0L);
        startAddr = startAddr & ~(PAGESIZE -1);
        length = RoundUpTo(length, PAGESIZE);
        D1ECHO(ADDR(startAddr) << " " << VAR(length));
        retAddr = (uptr *)mmap((void *)(startAddr),
                               length,
                               PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANON | MAP_FIXED | MAP_NORESERVE,
                               -1, 0);
        if (retAddr == MAP_FAILED)
        {
            ECHO("mmap Failed");
            Die();
        }

        // Protect the gap
        startAddr = SGL;
        length = (SGH-SGL);
        startAddr = startAddr & ~(PAGESIZE -1);
        length = RoundUpTo(length, PAGESIZE);
        D1ECHO(ADDR(startAddr) << " " << VAR(length));
        retAddr = (uptr *)mmap((void *)(startAddr),
                               length,
                               PROT_NONE,
                               MAP_PRIVATE | MAP_ANON | MAP_FIXED | MAP_NORESERVE,
                               -1, 0);
        if (retAddr == MAP_FAILED)
        {
            ECHO("mmap Failed");
            Die();
        }

        // Reserve high shadow mem
        startAddr = SM1L;
        length = (SM1H-SM1L);
        startAddr = startAddr & ~(PAGESIZE -1);
        length = RoundUpTo(length, PAGESIZE);
        D1ECHO(ADDR(startAddr) << " " << VAR(length));
        retAddr = (uptr *)mmap((void *)(startAddr),
                               length,
                               PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANON | MAP_FIXED | MAP_NORESERVE,
                               -1, 0);
        if (retAddr == MAP_FAILED)
        {
            ECHO("mmap Failed");
            Die();
        }
    }

    void Print()
    {
        cout << hex << "================ 0x" << setw(16) << setfill ('0') << M1H << endl;
        cout << dec << "| M1 = "<< M1SIZE/TB <<" TB    |" <<endl;
        cout << hex << "================ 0x" << setw(16) << setfill ('0') << M1L << endl<<endl<<endl;

        cout << hex << "================ 0x" << setw(16) << setfill ('0') << SM1H << endl;
        cout << dec << "| SM1 = "<< SM1SIZE/TB <<" TB   |" <<endl;
        cout << hex << "================ 0x" << setw(16) << setfill ('0') << SM1L << endl<<endl<<endl<<endl;

        cout << hex << "================ 0x" << setw(16) << setfill ('0') << SGH << endl;
        cout << dec << "| SG = "<< SGSIZE/TB <<" TB   |" <<endl;
        cout << hex << "================ 0x" << setw(16) << setfill ('0') << SGL << endl<<endl<<endl<<endl;

        cout << hex << "================ 0x" << setw(16) << setfill ('0') << SM0H << endl;
        cout << dec << "| SM0 = "<< SM0SIZE/MB <<" MB   |" <<endl;
        cout << hex << "================ 0x" << setw(16) << setfill ('0') << SM0L << endl<<endl<<endl;

        cout << hex << "================ 0x" << setw(16) << setfill ('0') << M0H << endl;
        cout << dec << "| M0 = "<< M0SIZE/GB <<" GB    |" <<endl;
        cout << hex << "================ 0x" << setw(16) << setfill ('0') << M0L << endl;
        cout << dec;
    }

    ~MemMap8th()
    {
        int retVal;
        uptr startAddr;
        uptr length;

        startAddr = SM0L;
        length = SM0SIZE;
        retVal = munmap((void *)startAddr, length);
        if( retVal == -1)
            ECHO("munmap Failed");

        startAddr = SGL;
        length = SGSIZE;
        retVal = munmap((void *)startAddr, length);
        if( retVal == -1)
            ECHO("munmap Failed");

        startAddr = SM1L;
        length = SM1SIZE;
        retVal = munmap((void *)startAddr, length);
        if( retVal == -1)
            ECHO("munmap Failed");

    }

};

/**
 ****************
 **/

#endif
