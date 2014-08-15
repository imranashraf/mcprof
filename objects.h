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

static int localObjId=0;

class Object
{
private:
    ADDRINT startAddr;
    int size;
    int line;
    string file;
    int id;
    string name;

public:
    Object() : startAddr(0), size(0), line(0), file("") , name(""){
    }

    Object(ADDRINT a, int s, int l, string f, string n) :
        startAddr(a), size(s), line(l), file(f), name(n) {
    }

    Object(int l, string f, string n) :
    startAddr(0), size(0), line(l), file(f), name(n) {}

    void Print()
    {
        ECHO(id << " " << name << " " << ADDR(startAddr) << " " << VAR(size) << " at " << file << ":" << line);
    }

    void SetLineFile(int l, string f)
    {
        line = l;
        file = f;
    }

    void SetSize(int s) {size = s;}
    int GetSize() { return size; }
    void SetAddr(int a) {startAddr = a;}
    ADDRINT GetStartAddr() { return startAddr; }
    bool isSameLine( int l) { return line == l;}
    bool isSameFile(string f) { return file == f;}
    string GetName() {return name; }
    void SetID(int id0) {id = id0;}
    int GetID() {return id; }
};

class Objects
{
private:
    vector<Object> objs;
    string selInstrfilename;

public:
    Objects(){}
    void Insert(Object o)
    {
        o.SetID(localObjId);
        localObjId++;
        string oname = o.GetName();

        // update global name to id map
        Name2ID[oname]=GlobalID;   // create the string -> Number binding
        ID2Name[GlobalID]=oname;   // create the Number -> String binding
        D1ECHO("Adding " << VAR(oname) << "(" << GlobalID << ") " << "to Map");
        GlobalID++;      // Increment the Global ID for the next function/object

        objs.push_back(o);
    }
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
        string oname;
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
        while( (sifin >> ofile) && (sifin >> oline) && (sifin >> oname))
        {
            Insert(Object(oline, ofile, oname));
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

    string GetName(ADDRINT addr)
    {
        for ( auto& o : objs )
        {
            ADDRINT saddr = o.GetStartAddr();
            int size = o.GetSize();
            if ( (addr >= saddr) && (addr < saddr+size) )
                return o.GetName();
        }
        return UnknownObj;
    }

    int GetID(ADDRINT addr)
    {
        for ( auto& o : objs )
        {
            ADDRINT saddr = o.GetStartAddr();
            int size = o.GetSize();
            if ( (addr >= saddr) && (addr < saddr+size) )
                return o.GetID();
        }
        return 0; // TODO: 0 or id of unknown object 
    }

    Object* GetObjectPtr(int index)
    {
        return &(objs.at(index) );
    }

};
#endif
