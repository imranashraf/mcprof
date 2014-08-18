#ifndef ENGINE3_H
#define ENGINE3_H

#include "globals.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>

// if BYTELEVEL is defined than the granularity is Byte
// #define BYTELEVEL

void RecordWriteEngine3(uptr addr, int size);
void RecordReadEngine3(uptr addr, int size);

#endif
