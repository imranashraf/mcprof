#include "shadow.h"
#include "symbols.h"

extern map <string,IDNoType> FuncName2ID;

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

void Symbols::InsertMallocCalloc(IDNoType id, uptr saddr, u32 locIndex, u32 size)
{
    D2ECHO("Inserting Malloc/Calloc/Realloc ");
    if(_Symbols.find(id) != _Symbols.end() )
    {
        Symbol& availSym = _Symbols[id];
        availSym.SetSize(saddr, size);
    }
    else
    {
        // Assign some name to this object symbol
        // TODO following can be done later at the end when names are really needed
        string name( "Object" + to_string(id) );
        Symbol newsym(id, saddr, size, name, SymType::OBJ, locIndex);

        ECHO("Adding Object Symbol with id : " << int(id) << " to Symbol Table");
        _Symbols[id] = newsym;
    }
    // we also need to set the object ids in the shadow table/mem for this object
    D2ECHO("Setting object ID as " << id << " on a size " << size);
    InitObjectIDs(saddr, size, id);
}

void Symbols::UpdateRealloc(IDNoType id, uptr saddr, u32 locIndex, u32 size)
{
    D2ECHO("Updating Realloc ");
    Symbol& availSym = _Symbols[id];
    availSym.SetSize(saddr,size);

    // we also need to set the object ids in the shadow table/mem for this object
    D2ECHO("Setting object ID as " << id << " on a size " << size);
    InitObjectIDs(saddr, size, id);
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

u16 Symbols::TotalSymbolCount()
{
    D2ECHO("Getting total Symbol count");
    return _Symbols.size();
}

// only the function count
// NOTE only function symbols are also added in FuncName2ID map
// so size of this map gives total function count
u16 Symbols::TotalFunctionCount()
{
    return FuncName2ID.size();
}

void Symbols::Remove(uptr saddr)
{
    D2ECHO("Removing symbol at Start Address: " << ADDR(saddr) );
    u32 size = GetSymSize(saddr);

    // uncomment the following to remove the objects on free
    // commented it to keep the objects in the table
    /*
    auto it = _Symbols.find(id);
    if(it != _Symbols.end() )
    {
        _Symbols.erase(it);
    }
    */

    // Clear the obj ids for this object, which is same as setting it to UnknownID
    D2ECHO("Clearing object ID to " << UnknownID << " on a size " << size);
    InitObjectIDs(saddr, size, UnknownID);
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

void Symbol::Print(ostream& fout)
{
    fout << "ID: " << (int)id << " " 
         << SymTypeName[symType] << " " << name << " " 
         << VAR(symLocIndex) << " " 
         << Locations.GetLocation(symLocIndex).toString() << endl;
     for(auto& pair : startAddr2Size)
     {
         auto& saddr = pair.first;
         auto& sizes = pair.second;
         fout << "    " << ADDR(saddr) << "(";
         for(auto& size : sizes)
            fout << " " << size;
         fout << ")" << endl;
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
        for ( auto& entry : _Symbols)
        { 
            auto& sym = entry.second;
            sym.Print(fout);
        }
        fout.close();
    }
}
