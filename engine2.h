#ifndef ENGINE2_H
#define ENGINE2_H

#include "globals.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>

// if BYTELEVEL is defined than the granularity is Byte
// #define BYTELEVEL

void RecordWriteEngine2(uptr addr, int size);
void RecordReadEngine2(uptr addr, int size);

#endif
