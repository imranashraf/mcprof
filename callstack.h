#ifndef CALLSTACK_H
#define CALLSTACK_H

#include "globals.h"

#include <vector>
#include <cstddef>
#include <iomanip>
#include <iostream>

using namespace std;

class CallStackType
{
private:
    vector<u16> stack;  // vector containing func ids of functions on call stack

public:
    CallStackType(){}
    u16 Top(){return stack.back();}
    bool Empty(){return stack.empty();}
    void Push(u16 f) { stack.push_back(f); }
    void Pop() { stack.pop_back(); }
    void Print(ofstream& fout);
};

#endif
