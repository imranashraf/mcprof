#ifndef OBJECTS_H
#define OBJECTS_H

#include "pin.H"
#include "globals.h"

#include <vector>
#include <algorithm>

using namespace std;

class Object
{
private:
    ADDRINT startAddr;
    int size;
    int line;
    string file;

public:
    Object() : startAddr(0), size(0), line(0), file("") {}
    Object(ADDRINT a, int s, int l, string f) :
        startAddr(a), size(s), line(l), file(f) {}
    void Print()
    {
        ECHO(ADDR(startAddr) << " " << VAR(size) << " at " << file << ":" << line);
    }
    void SetLineFile(int l, string f)
    {
        line = l;
        file = f;
    }
    void SetSize(int s) {size = s;}
    void SetAddr(int a) {startAddr = a;}
    ADDRINT GetStartAddr() { return startAddr; }
};

class Objects
{
private:
    vector<Object> objs;

public:
    Objects(){}
    void Insert(Object o) {objs.push_back(o); }
    void Remove(ADDRINT saddr)
    {
        objs.erase(std::remove_if(objs.begin(), objs.end(),
                [saddr](Object & o) { return o.GetStartAddr() == saddr; }),
                objs.end());
    }

    void Print() { ECHO("Object Table"); for ( auto& o : objs ) { o.Print(); } }
};
#endif
