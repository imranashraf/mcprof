#ifndef COUNTERS_H
#define COUNTERS_H

#include "globals.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>

void PrintInstrCount();
void PrintInstrPercents();
u64 TotalInstrCount();
float GetInstrCountPercent(IDNoType fid);

u64 GetCallCount(IDNoType fid);

#endif
