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
    u16 top(){return stack.back();}
    bool empty(){return stack.empty();}
    void push(u16 f) { stack.push_back(f); }
    void pop() { stack.pop_back(); }
    void print()
    {
        string stackftns("");
        if( !stack.empty()) stackftns += ID2Name[ stack[0] ];
        for ( u16 ftn=1; ftn < stack.size(); ftn++)
            stackftns += " -> " + ID2Name[ stack[ftn] ] ;
        ECHO("Call Stack : " << stackftns);
    }
};

#endif
