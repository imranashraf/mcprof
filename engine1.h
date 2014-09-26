#ifndef ENGINE1_H
#define ENGINE1_H

#include "globals.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>

// if BYTELEVEL is defined than the granularity is Byte
// #define BYTELEVEL

void RecordWriteEngine1(uptr addr, u32 size);
void RecordReadEngine1(uptr addr, u32 size);
void PrintAccesses();

#endif
