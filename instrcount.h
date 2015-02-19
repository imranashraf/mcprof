#ifndef INSTRCOUNT_H
#define INSTRCOUNT_H

#include "globals.h"

#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>

void PrintInstrCount();
void PrintInstrPercents();
u64 TotalInstrCount();
float GetInstrCountPercent(IDNoType fid);

#endif
