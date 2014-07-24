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

// No of bits required to address table entries
static const u64 L1BITS = (12);
static const u64 L2BITS = (18);
static const u64 L3BITS = (18);

// Sizes of tables
static const u64 L1ENTRIES = (1ULL << L1BITS);
static const u64 L2ENTRIES = (1ULL << L2BITS);
static const u64 L3ENTRIES = (1ULL << L3BITS);

// should be greater than max(L1ENTRIES,L2ENTRIES,L3ENTRIES)
#define UNACCESSED NULL
#define UNKNOWN_PRODUCER (0)

class Entry
{
private:
    FtnNo producer;
public:
    Entry() {
        producer=UNKNOWN_PRODUCER;
    }
    void setProducer(FtnNo p) {
        producer=p;
    }
    FtnNo getProducer() {
        return producer;
    }
};

// struct Entry {
//     FtnNo producer;
// };

class L1Table
{
private:
    Entry Table[L1ENTRIES];
public:
    L1Table() { }
    FtnNo getProducer(uptr L1Index) {
//         return Table[L1Index].producer;
        return Table[L1Index].getProducer();
    }
    void setProducer(uptr L1Index, FtnNo prod) {
//         Table[L1Index].producer = prod;
        Table[L1Index].setProducer(prod);
    }
    void setProducerRange(uptr L1Index, FtnNo prod, int size) {
        for(int i=0; i<size; i++) {
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
    L2Table() {
        // initially all the entries are UNACCESSED
        for(uptr i=0; i<L2ENTRIES; i++) {
            Table[i]=UNACCESSED;
        }
    }
    FtnNo getProducer(uptr L1Index, uptr L2Index) {
        if(Table[L2Index] == UNACCESSED) {
            Table[L2Index] = new L1Table;
        }
        return Table[L2Index]->getProducer(L1Index);
    }
    void setProducer(uptr L1Index, uptr L2Index, FtnNo prod) {
        if(Table[L2Index] == UNACCESSED) {
            Table[L2Index] = new L1Table;
        }
        Table[L2Index]->setProducer(L1Index, prod);
    }
    void setProducerRange(uptr L1Index, uptr L2Index, FtnNo prod, int size) {
        if(Table[L2Index] == UNACCESSED) {
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
    L3Table() {
        // initially all the entries are UNACCESSED
        for(uptr i=0; i<L3ENTRIES; i++) {
            Table[i]=UNACCESSED;
        }
    }

    FtnNo getProducer(uptr addr) {
        FtnNo prod;

        uptr L1Index=( (L1ENTRIES-1) & addr);
        uptr L2Index=( (L2ENTRIES-1) & (addr>>L1BITS) );
        uptr L3Index=( (L3ENTRIES-1) & (addr>>(L1BITS+L2BITS)) );
        if(Table[L3Index] == UNACCESSED) {
            Table[L3Index] = new L2Table;
        }
        prod = Table[L3Index]->getProducer(L1Index, L2Index);
        return prod;
    }

    void setProducer(uptr addr, FtnNo prod) {
        uptr L1Index=( (L1ENTRIES-1) & addr);
        uptr L2Index=( (L2ENTRIES-1) & (addr>>L1BITS) );
        uptr L3Index=( (L3ENTRIES-1) & (addr>>(L1BITS+L2BITS)) );
        if(Table[L3Index] == UNACCESSED) {
            Table[L3Index] = new L2Table;
        }
        Table[L3Index]->setProducer(L1Index, L2Index, prod);
    }

    void setProducerRange(uptr addr, FtnNo prod, int size) {
        uptr L1Index=( (L1ENTRIES-1) & addr);
        uptr L2Index=( (L2ENTRIES-1) & (addr>>L1BITS) );
        uptr L3Index=( (L3ENTRIES-1) & (addr>>(L1BITS+L2BITS)) );
        if(Table[L3Index] == UNACCESSED) {
            Table[L3Index] = new L2Table;
        }
        Table[L3Index]->setProducerRange(L1Index, L2Index, prod, size);
    }

};

void RecordWrite(FtnNo prod, uptr addr, int size);
void RecordRead(FtnNo cons, uptr addr, int size);


/**
 ****************
 **/
#define PAGESIZE (sysconf(_SC_PAGE_SIZE))
class MemMap
{
private:
static const uptr GB   = (1ULL << 30);
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
    MemMap(){
        uptr *retAddr;
        uptr startAddr;
        uptr length;

        startAddr = SM0L; length = M0SIZE;
        retAddr = (uptr *)mmap((void *)startAddr,
                    length,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANON | MAP_FIXED,
                    -1, 0);
        if (retAddr == MAP_FAILED)
            cout<<"mmap Failed"<<endl;

        startAddr = SM1L; length = M1SIZE;
        retAddr = (uptr *)mmap((void *)startAddr,
                    length,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANON | MAP_FIXED,
                    -1, 0);
        if (retAddr == MAP_FAILED)
            cout<<"mmap Failed"<<endl;
    }

    void Print() {
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
    uptr inline Mem2Shadow(uptr addr) {
        return (addr >=M0L && addr <= M0H)*(addr + M0OFFSET) +
               (addr >=M1L && addr <= M1H)*(addr - M1OFFSET) ;
    }

    ~MemMap(){
    int retVal;
    uptr startAddr;
    uptr length;

    startAddr = SM0L; length = M0SIZE;
    retVal = munmap((void *)startAddr, length);
    if( retVal == -1)
        cout<<"munmap Failed"<<endl;

    startAddr = SM1L; length = M1SIZE;
    retVal = munmap((void *)startAddr, length);
    if( retVal == -1)
        cout<<"munmap Failed"<<endl;
    }
};

void PrintShadowMap();
/**
 ****************
 **/

#endif
