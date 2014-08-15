#ifndef OBJECTS_H
#define OBJECTS_H

#include "pin.H"
#include "globals.h"
#include "utility.h"

#include <vector>
#include <string>
#include <fstream>
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
    Object(int l, string f) :
    startAddr(0), size(0), line(l), file(f) {}
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
    bool isSameLine( int l) { return line == l;}
    bool isSameFile(string f) { return file == f;}
};

class Objects
{
private:
    vector<Object> objs;
    string selInstrfilename;

public:
    Objects(){}
    void Insert(Object o) {objs.push_back(o); }
    void Remove(ADDRINT saddr)
    {
        objs.erase(std::remove_if(objs.begin(), objs.end(),
                [saddr](Object & o) { return o.GetStartAddr() == saddr; }),
                objs.end());
    }

    void Print()
    {
        ECHO("Printing Object Table");
        if(objs.empty() )
            ECHO("Object Table Empty");
        else
            for ( auto& o : objs ) { o.Print(); }
    }

    void InitFromFile()
    {
        selInstrfilename = "SelectObjects.txt";
        string ofile;
        int oline;
        ifstream sifin;
        sifin.open(selInstrfilename.c_str());
        if ( sifin.fail() )
        {
            ECHO("Can not open the selected instrumentation object list file ("
            <<selInstrfilename.c_str() << ")... Aborting!");
            Die();
        }
        if(isEmpty(sifin))
        {
            ECHO( "Specified selected instrumentation object list file ("
            << selInstrfilename.c_str()<<") is empty\n"
            << "No object to instrument"
            << "Specify at least 1 object in the list... Aborting!");
            Die();
        }

        // while there are objects in file
        while( (sifin >> ofile) && (sifin >> oline) )
        {
            Insert(Object(oline, ofile));
        }
        sifin.close();

        if(objs.empty() )
        {
            ECHO("No objects inserted in the table.");
            Die();
        }
    }

    bool Find(string f, int l)
    {
        for ( auto& o : objs )
        {
            if ( (o.isSameLine(l)) && (o.isSameFile(f)) )
                return true;
        }
        return false;
    }

    bool Find(string f, int l, Object* ptr)
    {
        for ( auto& o : objs )
        {
            if ( (o.isSameLine(l)) && (o.isSameFile(f)) )
            {
                ptr=&o;
                return true;
            }
        }
        return false;
    }

    bool Find(string f, int l, int& index)
    {
        int i=0;
        for ( auto& o : objs )
        {
            if ( (o.isSameLine(l)) && (o.isSameFile(f)) )
            {
                index = i;
                return true;
            }
            i++;
        }
        return false;
    }

    Object* GetObjectPtr(int index)
    {
        return &(objs.at(index) );
    }

};
#endif
