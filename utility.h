#ifndef UTILITY_H
#define UTILITY_H

#include "globals.h"
#include "pin.H"

#include <iostream>
#include <fstream>

using namespace std;

bool isEmpty(std::ifstream& fin);

const string& Target2RtnName(ADDRINT target);
const string& Target2LibName(ADDRINT target);
#endif