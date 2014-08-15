#ifndef MODE1_H
#define MODE1_H

#include "globals.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>

// if BYTELEVEL is defined than the granularity is Byte
// #define BYTELEVEL

void RecordWrite(FtnNo prod, uptr addr, int size);
void RecordRead(FtnNo cons, uptr addr, int size);

#endif
