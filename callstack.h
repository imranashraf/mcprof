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
    CallStackType() {}
    u16 Top()
    {
        return stack.back();
    }
    bool Empty()
    {
        return stack.empty();
    }
    void Push(u16 f)
    {
        stack.push_back(f);
    }
    void Pop()
    {
        stack.pop_back();
    }
    void Print();
    void Print(ofstream& fout);
};

class CallSiteStackType
{
private:
    vector<u32> sites;  // vector containing location indexes of call sites

public:
    CallSiteStackType() {} // TODO Do we need to insert the first call site as for main()
    bool Empty()
    {
        return sites.empty();
    }
    void Push(u32 locidx)
    {
        sites.push_back(locidx);
    }
    void Pop()
    {
        sites.pop_back();
    }
    u16 Top()
    {
        return sites.back();
    }
    u32 GetCallSites(u32 lastCallLocIndex);
    string GetCallSitesString();
    void Print();
    void Print(ofstream& fout);
};

#endif
