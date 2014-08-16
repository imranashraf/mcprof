#ifndef MODE3_H
#define MODE3_H

#include "globals.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

// a single call to a single func
struct Call
{
    string CallPath;    // main to this ftn, call path
                        // better to be an int representing func id !!!
                        // should also be update on func entry
    map<u8,double> Reads;     // all the  read accesses in this call, updated per access
    map<u8,double> Writes;    // all the write accesses in this call, updated per access
    u64 SeqNo; // TODO use it later for temporal aspect of a call
                // This should be incremented on each func entry, so that
                // each function call has a unique sequence no
};

void SetCurrCall(string& fname);
void RecordWriteMode3(FtnNo prod, uptr addr, int size);
void RecordReadMode3(FtnNo cons, uptr addr, int size);

void PrintAllCalls();

#endif
