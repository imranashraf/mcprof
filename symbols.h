#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "globals.h"
#include "pin.H"

#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

static string selectFtnFileName("selectedfunctions.in");
static string selectObjFileName("selectedobjects.in");

// change this class to struct
struct Location
{
    u32 lineNo;
    string fileName;

    Location() : lineNo(0), fileName("") {}
    Location(u32 l, string f) : lineNo(l), fileName(f) {}
    string toString(){ return fileName + ":" + to_string(lineNo); }

    bool operator==(const Location& loc) const {
        return lineNo == loc.lineNo && fileName == loc.fileName;
    }

    bool operator<(const Location& loc) const {
        return lineNo < loc.lineNo && fileName  < loc.fileName;
    }
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

    bool GetLocIndexIfAvailable(Location& loc, u32& locIndex)
    {
        for(u32 i=0; i<locations.size(); ++i)
        {
            if( loc.lineNo == locations[i].lineNo &&
                loc.fileName == locations[i].fileName )
            {
                locIndex=i;
                return true;
            }
        }

        return false;
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
    // Following map is used to record multiple allocations from 
    // same location (which will have same id). Secondly, multiple
    // allocations can have same address but the size can be different.
    // so a vector is used to preserve the allocation, where the last
    // element in the vector to store size with be the one currently used
    map<uptr,vector<u32>>startAddr2Size;
    string name;
    SymType symType;
    u32 symLocIndex;

public:
    Symbol() :
        id(0), name(""), symType(SymType::NA), symLocIndex(0) {}

    Symbol(IDNoType id1, string n, SymType typ) :
        id(id1), name(n), symType(typ), symLocIndex(0) {}

    Symbol(IDNoType id1, string n, SymType typ, u16 locidx) :
        id(id1), name(n), symType(typ), symLocIndex(locidx) {}

    Symbol(string n, SymType typ, u16 locidx) :
        name(n), symType(typ), symLocIndex(locidx) {}

    Symbol(IDNoType id1, uptr saddr, u32 size1, string n, SymType typ, u32 locidx) :
        id(id1), name(n), symType(typ), symLocIndex(locidx) 
            {startAddr2Size[saddr].push_back(size1);}

    void SetLocIndex(u32 idx) { symLocIndex = idx; }
    u32 GetLocIndex() { return symLocIndex; }
    u32 GetLine() {return Locations.GetLocation(symLocIndex).lineNo;}
    void SetSize(uptr saddr, u32 size) {startAddr2Size[saddr].push_back(size);}
    u32 GetSize(uptr saddr)
    {
        auto& sizeVec = startAddr2Size[saddr];
        if( sizeVec.size() )
            return sizeVec.back();
        else
            return 0;
    }
    bool isSameLine( u32 l) { return (Locations.GetLocation(symLocIndex).lineNo == l);}
    bool isSameFile( string& f) { return (Locations.GetLocation(symLocIndex).fileName == f);}
    void SetName(string n) {name = n;}
    string& GetName() {return name; }
    void SetID(IDNoType id0) {id = id0;}
    IDNoType GetID() {return id; }
    SymType GetType() {return symType;}
    void SetType(SymType typ) { symType = typ;}
    void Print(ostream& fout=std::cout);
    string GetLocation()
    {
        string f = Locations.GetLocation(symLocIndex).fileName + ":";
        f += to_string(Locations.GetLocation(symLocIndex).lineNo );
        return f;
    }
};

class Symbols
{
private:
    unordered_map<IDNoType,Symbol> _Symbols;

public:
    Symbols(){}
    void InsertMallocCalloc(IDNoType id, uptr saddr, u32 locIndex, u32 size);
    void UpdateRealloc(IDNoType id, uptr saddr, u32 locIndex, u32 size);
    void InsertFunction(const string& ftnname);
    string& GetSymName(IDNoType idno);
    u32 GetSymSize(uptr saddr);
    string GetSymLocation(IDNoType idno);
    bool IsSeenFunctionName(string& ftnName);
    bool IsSeenLocation(Location& loc, u32& locIndex);
    u16 TotalSymbolCount();
    u16 TotalFunctionCount(); // count of function symbols only
    void Remove(uptr saddr);
    bool SymIsObj(IDNoType idno);
    bool SymIsFunc(IDNoType idno);
    void Print();
    void InitFromObjFile();
    void InitFromFtnFile();
};

#endif
