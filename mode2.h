#ifndef MODE2_H
#define MODE2_H

#include "globals.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>

// if BYTELEVEL is defined than the granularity is Byte
// #define BYTELEVEL

void RecordWriteMode2(FtnNo prod, uptr addr, int size);
void RecordReadMode2(FtnNo cons, uptr addr, int size);

#endif
