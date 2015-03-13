#include "shadow.h"
#include "symbols.h"
#include "callstack.h"

extern map <string,IDNoType> FuncName2ID;
extern map <u32,IDNoType> CallSites2ID;
extern CallSiteStackType CallSiteStack;
extern bool ShowUnknown;

// List of all locations of symbols
LocationList Locations;

string& Symbols::GetSymName(IDNoType id)
{
    D2ECHO("Getting name of symbol with id: " << id );
    Symbol& sym = _Symbols[id];
    return ( sym.GetName() );
}

u32 Symbols::GetSymSize(uptr saddr)
{
    IDNoType id = GetObjectID(saddr);
    D2ECHO("Getting symbol size for address " << ADDR(saddr) << " " << VAR(id));
    return ( _Symbols[id].GetSize(saddr) );
}

u32 Symbols::GetSymSize(IDNoType id)
{
    D2ECHO("Getting size of symbol with id: " << id );
    Symbol& sym = _Symbols[id];
    return ( sym.GetSize() );
}

string Symbols::GetSymLocation(IDNoType id)
{
    string loc;
    Symbol& sym = _Symbols[id];
    return sym.GetLocation();
}

void Symbols::InsertMallocCalloc(uptr saddr, u32 lastCallLocIndex, u32 size)
{
    D2ECHO("Inserting Malloc/Calloc/Realloc ");

    u32 callsites;
    IDNoType id;

    // get callsites combined with the last lastCallLocIndex
    if( CallSiteStack.Top() != lastCallLocIndex )
        callsites = CallSiteStack.GetCallSites(lastCallLocIndex);
    else
        callsites = CallSiteStack.GetCallSites(0);

    D2ECHO(" callsites " << callsites );

    if(CallSites2ID.find(callsites) != CallSites2ID.end() )
    {
        // use existing id as this call site is already seen
        id = CallSites2ID[callsites];
        D2ECHO(" callsites " << callsites << " using existing id" << id);
    }
    else
    {
        // use a new id for this call site
        id = GlobalID++;
        CallSites2ID[callsites] = id;
        D2ECHO(" callsites " << callsites << " using new id" << id);
    }

    // To check if symbol is already in the table. This is possible because of:
    //      - the list of selected objects provided as input
    //      - multiple allocations from same line
    if(_Symbols.find(id) != _Symbols.end() )
    {
        D2ECHO("Updating address and size of existing Object Symbol with id : " << int(id) );
        Symbol& availSym = _Symbols[id];
        availSym.SetSize(saddr, size);
    }
    else
    {
        // Assign some name to this object symbol
        // TODO following can be done later at the end when names are really needed
        string name( "Obj" + to_string((long long)id) );
        Symbol newsym(id, saddr, size, name, SymType::OBJ, lastCallLocIndex, CallSiteStack);

        D2ECHO("Adding New Object Symbol with id : " << int(id) << " to Symbol Table");
        _Symbols[id] = newsym;
    }

    // we also need to set the object ids in the shadow table/mem for this object
    D2ECHO("Setting object ID as " << id << " on a size " << size);
    SetObjectIDs(saddr, size, id);
}

void Symbols::UpdateRealloc(IDNoType id, uptr saddr, u32 lastCallLocIndex, u32 size)
{
    D2ECHO("Updating Realloc ");
    Symbol& availSym = _Symbols[id];
    availSym.SetSize(saddr,size);

    // we also need to set the object ids in the shadow table/mem for this object
    D2ECHO("Setting object ID as " << id << " on a size " << size);
    SetObjectIDs(saddr, size, id);
}

void Symbols::InsertFunction(const string& ftnname)
{
    D2ECHO("Inserting Function " << ftnname);
    IDNoType id = GlobalID++;
    FuncName2ID[ftnname] = id;
    Symbol sym(id, ftnname, SymType::FUNC);
    D1ECHO("Adding Function Symbol: " << ftnname
           << " with id: " << int(id) << " to Symbol Table");
    _Symbols[id] = sym;
}

// void Symbols::InsertZoneIfNotAvailable(const string& zoneName)
// {
//     D2ECHO("Inserting Zone " << zoneName << " if it is not available" );
//     if ( IsSeenFunctionName(zoneName) == false)
//     {
//         IDNoType zid = GlobalID++;
//         FuncName2ID[zoneName] = zid;
//         Symbol sym(zid, zoneName, SymType::FUNC); // TODO separate type for zones???
//         D1ECHO("Adding Zone Symbol: " << zoneName << " with id: " << int(zid) << " to Symbol Table");
//         _Symbols[zid] = sym;
//     }
// }


// TODO is searching in this map fast enough or do we need different/separate
// data structure for seen function names
bool Symbols::IsSeenFunctionName(string& ftnName)
{
    D2ECHO("Checking if function " << ftnName << " is seen");
    if ( FuncName2ID.find(ftnName) == FuncName2ID.end() )
        return false;
    else
        return true;
}

bool Symbols::IsSeenLocation(Location& loc, u32& locIndex)
{
    D2ECHO("Checking if location " << loc.toString() << " is seen");
    return Locations.GetLocIndexIfAvailable(loc, locIndex);
}

u16 Symbols::TotalSymbolCount()
{
    D2ECHO("Getting total Symbol count");
    return _Symbols.size();
}

// only the function count
// NOTE only function symbols are added in FuncName2ID map
// so size of this map gives total function count
u16 Symbols::TotalFunctionCount()
{
    D2ECHO("Getting total Function count");
    return FuncName2ID.size();
}

void Symbols::Remove(uptr saddr)
{
    D2ECHO("Removing symbol at Start Address: " << ADDR(saddr) );
    u32 size = GetSymSize(saddr);

    // uncomment the following to remove the objects on free.
    // commented it to keep the objects in the table for later use.
    /*
    auto it = _Symbols.find(id);
    if(it != _Symbols.end() )
    {
        _Symbols.erase(it);
    }
    */

    // Clear the obj ids for this object, which is same as setting it to UnknownID
    D2ECHO("Clearing object ID to " << UnknownID << " on a size " << size);
    SetObjectIDs(saddr, size, UnknownID);
}

bool Symbols::SymIsObj(IDNoType id)
{
    D2ECHO("Checking if Symbol "<< id << " is OBJ");
    return ( _Symbols[id].GetType() == SymType::OBJ );
}

bool Symbols::SymIsFunc(IDNoType id)
{
    D2ECHO("Checking if Symbol "<< id << " is FUNC");
    return ( _Symbols[id].GetType() == SymType::FUNC );
}

// TODO May be the following two init methods may be combined together to read from
// one file with same format
void Symbols::InitFromFtnFile()
{
    ifstream sifin;
    OpenInFile(selectFtnFileName, sifin);

    string symname;
    u32 i=0;
    while(sifin >> symname)   // while there are function names in file
    {
        IDNoType id = GlobalID++;
        FuncName2ID[symname] = id;
        D1ECHO("Adding Function Symbol " << symname << "("<< id << ") to symbol table");
        _Symbols[id] = Symbol(id, symname, SymType::FUNC);
        i++;
    }
    sifin.close();

    if(i==0)
    {
        ECHO("No function inserted in the symbol table.");
        Die();
    }
}

void Symbols::InitFromObjFile()
{
    ifstream sifin;
    OpenInFile(selectObjFileName, sifin);

    string symfile;
    u32 symline;
    string symname;

    // while there are objects in file
    // TODO what if name is not specified? do we need to check!
    // TODO what if there are multiple allocations on same name, line etc?
    u32 i=0;
    while( (sifin >> symfile) && (sifin >> symline) && (sifin >> symname))
    {
        Location loc(symline, symfile);
        u16 locindex = Locations.Insert( loc );
        // Get a new id for this NEW location
        IDNoType id = GlobalID++;
        //LocIndex2ID[locindex] = id;
        ECHO("Adding Object Symbol " << symname << "("<< id << ") to symbol table");
        _Symbols[id] = Symbol(id, symname, SymType::OBJ, locindex );
        ++i;
    }
    sifin.close();

    if(i==0)
    {
        ECHO("No object inserted in the symbol table.");
        Die();
    }
}

void Symbol::Print(ostream& fout)
{
    if(!ShowUnknown && id==UnknownID)
        return;

    fout << "ID: " << id << " "
         << SymTypeName[symType] << " " << name << " "
         << VAR(symLocIndex) << " ";

    fout << symCallSite.GetCallSitesString() << ">"
         << Locations.GetLocation(symLocIndex).toString() << endl;

    if(RecordAllAllocations)
    {
        //for(auto& pair : startAddr2Size)
        map<uptr,vector<u32>>::iterator mIter;
        for(mIter=startAddr2Size.begin(); mIter!=startAddr2Size.end(); mIter++)
        {
            auto& saddr = mIter->first;
            auto& sizes = mIter->second;
            fout << "    " << ADDR(saddr) << "(";

            //for(auto& size : sizes)
            vector<u32>::iterator vIter;
            for(vIter=sizes.begin(); vIter!=sizes.end(); vIter++)
                fout << " " << *vIter;
            fout << ")" << endl;
        }
    }
    else
    {
        fout << "    " << ADDR(startAddr) << "("<< size <<")" << endl;
    }
}

void Symbols::Print()
{
    if(_Symbols.empty() )
        ECHO("Symbol Table Empty");
    else
    {
        string fname("symbols.out");
        ofstream fout;
        OpenOutFile(fname.c_str(), fout);
        ECHO("Printing Symbol Table to " << fname );
        //for ( auto& entry : _Symbols)
        std::tr1::unordered_map<IDNoType,Symbol>::iterator iter;
        for(iter=_Symbols.begin(); iter!=_Symbols.end(); iter++)
        {
            auto& sym = iter->second;
            sym.Print(fout);
        }
        fout.close();
    }
}
