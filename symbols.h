#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "pin.H"
#include "globals.h"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

extern string selectFtnFileName;
extern string selectObjFileName;

extern map <string,IDNoType> Name2ID;
extern map <IDNoType,string> ID2Name;

extern string MALLOC;
extern string FREE;

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
    // NOTE location index 0 is for unknown ftn
    // TODO Do we also need for unknown obj?
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
// may be first location should be unknown/invalid
// List of all locations of symbols
extern LocationList Locations;

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
    IDNoType GetSymID(uptr addr)
    {
        for ( auto& sym : syms )
        {
            uptr saddr = sym.GetStartAddr();
            int size = sym.GetSize();
            if ( (addr >= saddr) && (addr < saddr+size) )
                return sym.GetID();
        }
        return UnknownID;
    }

    void InsertObject(Symbol sym)
    {
        IDNoType idno = syms.size();
        D1ECHO("Adding Object Symbol " << VAR(idno) << " to Symbol Table");
        sym.SetID(idno);   // id no of symbols is the index in syms vector
        syms.push_back(sym);
    }

    void InsertFunction(string& ftnname)
    {
        D1ECHO("Adding Function Symbol " << ftnname << " to Symbol Table");
        IDNoType idno = syms.size();
        Name2ID[ftnname] = idno;
        syms.push_back( Symbol(idno, ftnname, SymType::FUNC) );
    }

    bool IsSeenFunctionName(string& ftnName)
    {
        if ( Name2ID.find(ftnName) == Name2ID.end() )
            return false;
        else
            return true;
    }

    void Remove(uptr saddr)
    {
        syms.erase(std::remove_if(syms.begin(), syms.end(),
                [saddr](Symbol& sym) { return sym.GetStartAddr() == saddr; }),
                syms.end());
    }

    bool SymIsObj(IDNoType idno)
    {
        return ( syms.at(idno).GetType() == SymType::OBJ );
    }

    bool SymIsFunc(IDNoType idno)
    {
        return ( syms.at(idno).GetType() == SymType::FUNC );
    }

    Symbol* GetSymbolPtr(IDNoType id)
    {
        return &( syms[id] );
    }

    IDNoType GetID(string& f, u32 l)
    {
        IDNoType index=0;   // id is index
        D2ECHO("Finding "<< f << ":" << l << " in Symbol Table");
        for ( auto& sym : syms )
        {
            if ( (sym.isSameLine(l)) && (sym.isSameFile(f)) && (sym.GetType() == SymType::OBJ) )
            {
                D2ECHO("Found");
                return index;
            }
            ++index;
        }
        D2ECHO("Not Found");
        return UnknownID;
    }

    void Print()
    {
        ECHO("Printing Symbol Table");
        if(syms.empty() )
            ECHO("Symbol Table Empty");
        else
            for ( auto& sym : syms ) { sym.Print(); }
    }

    void InitFromObjFile()
    {
        string symfile;
        u32 symline;
        string symname;
        ifstream sifin;
        sifin.open(selectObjFileName.c_str());
        if ( sifin.fail() )
        {
            ECHO("Can not open the selected instrumentation object list file ("
                <<selectObjFileName.c_str() << ")... Aborting!");
            Die();
        }
        if(isEmpty(sifin))
        {
            ECHO( "Specified selected instrumentation object list file ("
                << selectObjFileName.c_str()<<") is empty\n"
                << "No object to instrument"
                << "Specify at least 1 object in the list... Aborting!");
            Die();
        }

        // while there are objects in file
        // TODO what if name is not specified? do we need to check!
        // TODO what if there are multiple allocations on same name, line etc?
        u32 i=0;
        while( (sifin >> symfile) && (sifin >> symline) && (sifin >> symname))
        {
            u16 locindex = Locations.Insert( Location(symline, symfile) );
            IDNoType idno = syms.size();
            D1ECHO("Adding Object Symbol " << symname << "("<< idno << ") to symbol table");
            syms.push_back( Symbol(idno, symname, SymType::OBJ, locindex ) );
            ++i;
        }
        sifin.close();

        if(i==0)
        {
            ECHO("No object inserted in the symbol table.");
            Die();
        }
    }

    // TODO these two init methods may be combined together to read from one file
    // with same format
    void InitFromFtnFile()
    {
        string symname;
        ifstream sifin;
        sifin.open(selectFtnFileName.c_str());
        if ( sifin.fail() )
        {
            ECHO("Can not open the selected instrumentation function list file ("
                 <<selectFtnFileName.c_str() << ")... Aborting!");
            Die();
        }
        if(isEmpty(sifin))
        {
            ECHO( "Specified selected instrumentation function list file ("
                  << selectFtnFileName.c_str()<<") is empty\n"
                  << "No function to instrument"
                  << "Specify at least 1 function in the list... Aborting!");
            Die();
        }

        u32 i=0;
        while(sifin >> symname)   // while there are function names in file
        {
            IDNoType idno = syms.size();
            Name2ID[symname] = idno;
            D1ECHO("Adding Function Symbol " << symname << "("<< idno << ") to symbol table");
            syms.push_back( Symbol(idno, symname, SymType::FUNC) );
            i++;
        }
        sifin.close();

        if(i==0)
        {
            ECHO("No function inserted in the symbol table.");
            Die();
        }

    }

};

#endif
