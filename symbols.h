#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "pin.H"
#include "globals.h"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

static string selectFtnFileName("SelectFunctions.txt");
static string selectObjFileName("SelectObjects.txt");

class Location
{
private:
    u32 lineNo;
    string fileName;
public:
    Location() : lineNo(0), fileName("") {}
    Location(u32 l, string f) : lineNo(l), fileName(f) {}
    string toString(){ return fileName + ":" + to_string(lineNo); }
    void SetLocation(u32 l, string f) { lineNo = l; fileName = f; }
    u32 GetLineNo() { return lineNo; }
    string& GetFileName() { return fileName; }
};

// class for List of all locations of symbols
class LocationList
{
private:
    vector<Location> locations;
    // NOTE location index 0 is for unknown location
public:
    LocationList() { locations.push_back( Location() ); }

    // Insert a location and return location index
    u16 Insert(Location loc)
    {
        locations.push_back(loc);
        return locations.size()-1;  // loc index of currently inserted size -1
    }

    Location& GetLocation(u16 index)
    {
        return locations.at(index);
    }
};

// List of all locations of symbols
extern LocationList Locations;
// may be first location should be unknown/invalid

enum SymType { NA, FUNC, OBJ };
static vector<string> SymTypeName { "NA", "FUNC", "OBJ" };

class Symbol
{
private:
    IDNoType id;
    uptr startAddr;
    u32 size;
    string name;
    SymType symType;
    u32 symLocIndex;

public:
    Symbol() :
        id(0), startAddr(0), size(0), name(""),
        symType(SymType::NA), symLocIndex(0) {}

    Symbol(string n, SymType typ) :
        id(0), startAddr(0), size(0), name(n),
        symType(typ), symLocIndex(0) {}

    Symbol(string n, SymType typ, u16 locidx) :
        id(0), startAddr(0), size(0), name(n),
        symType(typ), symLocIndex(locidx) {}

    Symbol(IDNoType id1, string n, SymType typ, u16 locidx) :
        id(id1), startAddr(0), size(0), name(n),
        symType(typ), symLocIndex(locidx) {}

    Symbol(IDNoType id1, string n, SymType typ) :
        id(id1), startAddr(0), size(0), name(n),
        symType(typ), symLocIndex(0) {}

    void Print()
    {
        ECHO(VAR(id) << " " << SymTypeName[symType] << " " << name << " " << ADDR(startAddr) << " " << VAR(size)
            << " at " << VAR(symLocIndex) << " " << Locations.GetLocation(symLocIndex).toString() );
    }

    void SetLocIndex(u32 idx) { symLocIndex = idx; }
    u32 GetLine() {return Locations.GetLocation(symLocIndex).GetLineNo();}
    void SetSize(u32 s) {size = s;}
    u32 GetSize() { return size; }
    void SetStartAddr(uptr a) {startAddr = a;}
    uptr GetStartAddr() { return startAddr; }
    bool isSameLine( u32 l) { return (Locations.GetLocation(symLocIndex).GetLineNo() == l);}
    bool isSameFile( string& f) { return (Locations.GetLocation(symLocIndex).GetFileName() == f);}
    void SetName(string n) {name = n;}
    string& GetName() {return name; }
    void SetID(IDNoType id0) {id = id0;}
    IDNoType GetID() {return id; }
    SymType GetType() {return symType;}
    void SetType(SymType typ) { symType = typ;}
};

class Symbols
{
private:
    vector<Symbol> syms;
    // NOTE index in this vector will be the id of each symbol
    // NOTE object symbols are inserted at call site to malloc
    // NOTE function symbol are inserted at instrumentation time of a routine

public:
    Symbols(){}
    void InsertObject(Symbol sym);
    void InsertFunction(const string& ftnname);
    IDNoType GetID(string& f, u32 l);
    IDNoType GetSymID(uptr addr);
    string& GetSymName(IDNoType idno);
    Symbol* GetSymbolPtr(IDNoType id);
    Symbol* GetSymbolPtrWithStartAddr(uptr saddr1);
    bool IsSeenFunctionName(string& ftnName);
    IDNoType TotalSymbolCount();
    IDNoType TotalFunctionCount(); // count of function symbols only
    void Remove(uptr saddr);
    bool SymIsObj(IDNoType idno);
    bool SymIsFunc(IDNoType idno);
    void Print();
    void InitFromObjFile();
    void InitFromFtnFile();
    void UpdateID2NameForFtnSymbols();
    void UpdateID2NameForObjSymbols();
};

#endif
