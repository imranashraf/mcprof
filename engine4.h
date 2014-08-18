#ifndef ENGINE4_H
#define ENGINE4_H

#include "globals.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include "callstack.h"

using namespace std;

// a single call to a single func
struct Call
{
    CallStackType CallPath; // call path to this ftn, recorded once on func entry
    map<u8,double> Reads;     // all the  read accesses in this call, updated per access
    map<u8,double> Writes;    // all the write accesses in this call, updated per access
    u64 SeqNo; // TODO use it later for temporal aspect of a call
                // This should be incremented on each func entry, so that
                // each function call has a unique sequence no
};

void SetCurrCall(string& fname);

void RecordWriteEngine4(uptr addr, int size);
void RecordReadEngine4(uptr addr, int size);

void PrintAllCalls();

#endif
