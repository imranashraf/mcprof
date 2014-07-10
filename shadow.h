#ifndef SHADOW_H
#define SHADOW_H

#include "globals.h"

#include <cassert>
#include <cstddef>

// Sizes of tables
// #define L1ENTRIES (1<<16)
// #define L2ENTRIES (1<<16)
// #define L3ENTRIES (1<<16)

#define L1ENTRIES (1<<2)
#define L2ENTRIES (1<<3)
#define L3ENTRIES (1<<4)

// No of bits required to address table entries
#define L1BITS (1>>L1ENTRIES)
#define L2BITS (1>>L2ENTRIES)
#define L3BITS (1>>L3ENTRIES)

// should be greater than max(L1ENTRIES,L2ENTRIES,L3ENTRIES)
#define UNACCESSED NULL
#define UNKNOWN_PRODUCER (0)

class Entry
{
    private:
        FtnNo producer;
    public:
        Entry() { producer=UNKNOWN_PRODUCER;}
        void setProducer(FtnNo p){producer=p;}
        FtnNo getProducer(){return producer;}
};

class L1Table
{
    private:
        Entry Table[L1ENTRIES];
    public:
        L1Table() { }
        FtnNo getProducer(uptr L1Index){
            return Table[L1Index].getProducer();
        }
        void setProducer(uptr L1Index, FtnNo prod){
            Table[L1Index].setProducer(prod);
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
            for(uptr i=0; i<L2ENTRIES; i++){
                Table[i]=UNACCESSED;
            }
        }
        FtnNo getProducer(uptr L1Index, uptr L2Index){
            if(Table[L2Index] == UNACCESSED){
                Table[L2Index] = new L1Table;
            }
            return Table[L2Index]->getProducer(L1Index);
        }
        void setProducer(uptr L1Index, uptr L2Index, FtnNo prod){
            if(Table[L2Index] == UNACCESSED){
                Table[L2Index] = new L1Table;
            }
            Table[L2Index]->setProducer(L1Index, prod);
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
            for(uptr i=0; i<L3ENTRIES; i++){
                Table[i]=UNACCESSED;
            }
        }

        FtnNo getProducer(uptr addr)
        {
            FtnNo prod;

            uptr L1Index=( (L1ENTRIES-1) & addr);
            uptr L2Index=( (L2ENTRIES-1) & (addr>>L1BITS) );
            uptr L3Index=( (L3ENTRIES-1) & (addr>>(L1BITS+L2BITS)) );
            if(Table[L3Index] == UNACCESSED){
                Table[L3Index] = new L2Table;
            }
            prod = Table[L3Index]->getProducer(L1Index, L2Index);
            return prod;
        }

        void setProducer(uptr addr, FtnNo prod)
        {
            uptr L1Index=( (L1ENTRIES-1) & addr);
            uptr L2Index=( (L2ENTRIES-1) & (addr>>L1BITS) );
            uptr L3Index=( (L3ENTRIES-1) & (addr>>(L1BITS+L2BITS)) );
            if(Table[L3Index] == UNACCESSED){
                Table[L3Index] = new L2Table;
            }
            Table[L3Index]->setProducer(L1Index, L2Index, prod);
        }
};

#endif
