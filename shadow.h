#ifndef SHADOW_H
#define SHADOW_H

#include "globals.h"

#include <cassert>
#include <cstddef>

// No of bits required to address table entries
static const u64 L1BITS = (16);
static const u64 L2BITS = (16);
static const u64 L3BITS = (16);

// Sizes of tables
static const u64 L1ENTRIES = (1ULL << L1BITS);
static const u64 L2ENTRIES = (1ULL << L2BITS);
static const u64 L3ENTRIES = (1ULL << L3BITS);

// should be greater than max(L1ENTRIES,L2ENTRIES,L3ENTRIES)
#define UNACCESSED NULL
#define UNKNOWN_PRODUCER (0)

// class Entry
// {
// private:
//     FtnNo producer;
// public:
//     Entry() {
//         producer=UNKNOWN_PRODUCER;
//     }
//     void setProducer(FtnNo p) {
//         producer=p;
//     }
//     FtnNo getProducer() {
//         return producer;
//     }
// };

struct Entry {
    FtnNo producer;
};

class L1Table
{
private:
    Entry Table[L1ENTRIES];
public:
    L1Table() { }
    FtnNo getProducer(uptr L1Index) {
        return Table[L1Index].producer;
    }
    void setProducer(uptr L1Index, FtnNo prod) {
        Table[L1Index].producer = prod;
    }
    void setProducerRange(uptr L1Index, FtnNo prod, int size) {
        for(int i=0; i<size; i++) {
            Table[L1Index + i].producer = prod;
//             Table[L1Index+i].setProducer(prod);
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

#endif
