/*
         __       __   ______   _______   _______    ______   ________
        /  \     /  | /      \ /       \ /       \  /      \ /        |
        $$  \   /$$ |/$$$$$$  |$$$$$$$  |$$$$$$$  |/$$$$$$  |$$$$$$$$/
        $$$  \ /$$$ |$$ |  $$/ $$ |__$$ |$$ |__$$ |$$ |  $$ |$$ |__
        $$$$  /$$$$ |$$ |      $$    $$/ $$    $$< $$ |  $$ |$$    |
        $$ $$ $$/$$ |$$ |   __ $$$$$$$/  $$$$$$$  |$$ |  $$ |$$$$$/
        $$ |$$$/ $$ |$$ \__/  |$$ |      $$ |  $$ |$$ \__$$ |$$ |
        $$ | $/  $$ |$$    $$/ $$ |      $$ |  $$ |$$    $$/ $$ |
        $$/      $$/  $$$$$$/  $$/       $$/   $$/  $$$$$$/  $$/

                A Memory and Communication Profiler

 * This file is a part of MCPROF.
 * https://bitbucket.org/imranashraf/mcprof
 * 
 * Copyright (c) 2014-2015 TU Delft, The Netherlands.
 * All rights reserved.
 * 
 * MCPROF is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCPROF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MCPROF.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Authors: Imran Ashraf
 *
 */

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "globals.h"
#include "callstack.h"
#include "pin.H"

#include <vector>
// #include <unordered_map>
#include <tr1/unordered_map>
#include <string>
#include <fstream>
#include <algorithm>

extern bool RecordAllAllocations;

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
    string toString()
    {
        return fileName + ":" + to_string((long long)lineNo);
    }

    bool operator==(const Location& loc) const
    {
        return lineNo == loc.lineNo && fileName == loc.fileName;
    }

    bool operator<(const Location& loc) const
    {
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
    LocationList()
    {
        locations.push_back( Location() );
    }

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

    // startAddr and size are used when all allocations are not recorded
    uptr startAddr; // start address of the symbol
    u32 size;       // size of the symbol. In case of multiple allocations,
                    // this holds the size of last allocation

    string name;
    SymType symType;
    u32 symLocIndex;
    CallSiteStackType symCallSite;

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
    {
        if(RecordAllAllocations)
        {
            startAddr2Size[saddr].push_back(size1);
            size=size1;
        }
        else
        {
            startAddr=saddr;
            size=size1;
        }
    }

    Symbol(IDNoType id1, uptr saddr, u32 size1, string n, SymType typ, u32 locidx, CallSiteStackType& callsitestack) :
        id(id1), name(n), symType(typ), symLocIndex(locidx), symCallSite(callsitestack)
    {
        if(RecordAllAllocations)
        {
            startAddr2Size[saddr].push_back(size1);
            size=size1;
        }
        else
        {
            startAddr=saddr;
            size=size1;
        }
    }

    void SetLocIndex(u32 idx)
    {
        symLocIndex = idx;
    }
    u32 GetLocIndex()
    {
        return symLocIndex;
    }
    u32 GetLine()
    {
        return Locations.GetLocation(symLocIndex).lineNo;
    }

    void SetSize(uptr saddr, u32 size1)
    {
        if(RecordAllAllocations)
        {
            startAddr2Size[saddr].push_back(size1);
            size=size1; // size then only holds the last size value
        }
        else
        {
            startAddr=saddr;
            size=size1;
        }
    }
    u32 GetSize(uptr saddr)
    {
        if(RecordAllAllocations)
        {
            auto& sizeVec = startAddr2Size[saddr];
            if( sizeVec.size() )
                return sizeVec.back();
            else
                return 0;
        }
        else
        {
            return size;
        }
    }
    //size of the last allocation only in the case of multiple allocations
    u32 GetSize()
    {
        return size;
    }

    bool isSameLine( u32 l)
    {
        return (Locations.GetLocation(symLocIndex).lineNo == l);
    }
    bool isSameFile( string& f)
    {
        return (Locations.GetLocation(symLocIndex).fileName == f);
    }
    void SetName(string n)
    {
        name = n;
    }
    string& GetName()
    {
        return name;
    }
    void SetID(IDNoType id0)
    {
        id = id0;
    }
    IDNoType GetID()
    {
        return id;
    }
    SymType GetType()
    {
        return symType;
    }
    void SetType(SymType typ)
    {
        symType = typ;
    }
    void Print(ostream& fout=std::cout);
    string GetLocation()
    {
        string f = Locations.GetLocation(symLocIndex).fileName + ":";
        f += to_string( (long long) (Locations.GetLocation(symLocIndex).lineNo) );
        return f;
    }
};

class Symbols
{
private:
    std::tr1::unordered_map<IDNoType,Symbol> _Symbols;

public:
    Symbols() {}
    void InsertMallocCalloc(uptr saddr, u32 locIndex, u32 size);
    void UpdateRealloc(IDNoType id, uptr saddr, u32 locIndex, u32 size);
    void InsertFunction(const string& ftnname);
    string& GetSymName(IDNoType idno);
    u32 GetSymSize(uptr saddr);
    u32 GetSymSize(IDNoType idno);
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
