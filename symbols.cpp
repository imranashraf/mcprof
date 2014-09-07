#include "shadow.h"
#include "symbols.h"

extern map <string,IDNoType> Name2ID;
extern map <IDNoType,string> ID2Name;

map <u32,IDNoType> LocIndex2ID;

// List of all locations of symbols
LocationList Locations;

// TODO may be used ID2Name for this
string& Symbols::GetSymName(IDNoType id)
{
    auto& sym = _Symbols[id];
    return ( sym.GetName() );
}

// returns the symbol ptr with start address as input
Symbol* Symbols::GetSymbolPtr(uptr saddr)
{
    IDNoType id = GetObjectID(saddr);
    if(id != UnknownID)
        return &(_Symbols[id]);
    else
        return nullptr;
}

Symbol* Symbols::InsertAndGetObjectPtr(Symbol& newsym)
{
    uptr saddr = newsym.GetStartAddr();
    IDNoType id = GetObjectID(saddr);
    if(id == UnknownID) // insert
    {
        IDNoType id=GlobalID++;
        newsym.SetID(id);

        //u32 locidx = sym.symLocIndex;
        //LocIndex2ID[locidx] = id;
        
        // Assign some name to this object symbol
        // TODO following can be done later at the end when names are really needed
        newsym.SetName( "Object" + to_string(id) );

        //TODO this can be done at the initialization of new obj in pintrace
        newsym.SetType(SymType::OBJ);

        D1ECHO("Adding Object Symbol " << VAR(id) << " to Symbol Table");    
        _Symbols[id] = newsym;
        
        // we also need to set the object ids in the shadow table/mem for this object
        InitObjectIDs(saddr, newsym.GetSize(), id);
    }
   
    Symbol* symptr = &(_Symbols[id]);
    return symptr;
}

void Symbols::InsertFunction(const string& ftnname)
{
    D1ECHO("Adding Function Symbol " << ftnname << " to Symbol Table");
    IDNoType id = GlobalID++;
    Name2ID[ftnname] = id;
    // ID2Name is populated at the end, as it is needed for output only
    
    _Symbols[id] = Symbol(id, ftnname, SymType::FUNC); //TODO emplace
}

// TODO is searching in this map fast enough or do we need different/separate
// data structure for seen function names
bool Symbols::IsSeenFunctionName(string& ftnName)
{
    if ( Name2ID.find(ftnName) == Name2ID.end() )
        return false;
    else
        return true;
}

u16 Symbols::TotalSymbolCount()
{
    return _Symbols.size();
}

// only the function count
// NOTE only function symbols are also added in Name2ID map
// so size of this map gives total function count
u16 Symbols::TotalFunctionCount()
{
    return Name2ID.size();
}

void Symbols::Remove(uptr saddr)
{
    IDNoType id = GetObjectID(saddr);
    auto it = _Symbols.find(id);
    if(it != _Symbols.end() )
    {
        _Symbols.erase(it);
    }

    // TODO we also need to clear the obj ids for this object
}

bool Symbols::SymIsObj(IDNoType id)
{
    return ( _Symbols[id].GetType() == SymType::OBJ );
}

bool Symbols::SymIsFunc(IDNoType id)
{
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
        Name2ID[symname] = id;
        D1ECHO("Adding Function Symbol " << symname << "("<< idno << ") to symbol table");
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
        u16 locindex = Locations.Insert( Location(symline, symfile) );
        IDNoType id = GlobalID++;
        D1ECHO("Adding Object Symbol " << symname << "("<< id << ") to symbol table");
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

void Symbols::Print()
{
    ECHO("Printing Symbol Table");
    if(_Symbols.empty() )
        ECHO("Symbol Table Empty");
    else
        for ( auto& entry : _Symbols)
        { 
            auto& sym = entry.second;
            sym.Print();
        }
}

// This functions generates the reverse binding of Name2ID, i.e. ID2Name.
// NOTE Name2ID is update are runtime whenever a function is seen, but
// the reverse mapping is not updated to save time
void Symbols::UpdateID2NameForFtnSymbols()
{
    for ( auto& entry : Name2ID )
    {
        const string& ftnname = entry.first;
        const IDNoType& id = entry.second;
        ID2Name[id] = ftnname;
    }
}

void Symbols::UpdateID2NameForObjSymbols()
{
    for ( auto& entry : _Symbols )
    {
        auto& sym = entry.second;
        const string& objname = sym.GetName();
        const IDNoType& id = sym.GetID();
        ID2Name[id] = objname;
    }
}
