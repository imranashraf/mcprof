#include "symbols.h"

extern map <string,IDNoType> Name2ID;
extern map <IDNoType,string> ID2Name;

// List of all locations of symbols
LocationList Locations;

string& Symbols::GetSymName(IDNoType idno)
{
    return ( syms.at(idno).GetName() );
}

Symbol* Symbols::GetSymbolPtr(IDNoType id)
{
    return &( syms[id] );
}

// returns the symbol ptr with start address as input
Symbol* Symbols::GetSymbolPtrWithStartAddr(uptr saddr1)
{
    for ( auto& sym : syms )
    {
        uptr saddr = sym.GetStartAddr();
        if ( saddr == saddr1 )
            return &( syms[ sym.GetID() ] );
    }
    return nullptr;
}

IDNoType Symbols::GetSymID(uptr addr)
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

IDNoType Symbols::GetID(string& f, u32 l)
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

void Symbols::InsertObject(Symbol sym)
{
    IDNoType idno = syms.size();
    D1ECHO("Adding Object Symbol " << VAR(idno) << " to Symbol Table");
    sym.SetID(idno);   // id no of symbols is the index in syms vector
    sym.SetName( "Object" + to_string(idno) );   // Assign some name to this object symbol
    syms.push_back(sym);
}

void Symbols::InsertFunction(const string& ftnname)
{
    D1ECHO("Adding Function Symbol " << ftnname << " to Symbol Table");
    IDNoType idno = syms.size();
    Name2ID[ftnname] = idno;
    syms.push_back( Symbol(idno, ftnname, SymType::FUNC) );
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

IDNoType Symbols::TotalSymbolCount()
{
    return syms.size();
}

// only the function count
// NOTE only function symbols are also added in Name2ID map
// so size of this map gives total function count
IDNoType Symbols::TotalFunctionCount()
{
    return Name2ID.size();
}

void Symbols::Remove(uptr saddr)
{
    syms.erase(std::remove_if(syms.begin(), syms.end(),
            [saddr](Symbol& sym) { return sym.GetStartAddr() == saddr; }),
            syms.end());
}

bool Symbols::SymIsObj(IDNoType idno)
{
    return ( syms.at(idno).GetType() == SymType::OBJ );
}

bool Symbols::SymIsFunc(IDNoType idno)
{
    return ( syms.at(idno).GetType() == SymType::FUNC );
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

void Symbols::Print()
{
    ECHO("Printing Symbol Table");
    if(syms.empty() )
        ECHO("Symbol Table Empty");
    else
        for ( auto& sym : syms ) { sym.Print(); }
}

// This functions generates the reverse binding of Name2ID, i.e. ID2Name.
// NOTE Name2ID is update are runtime whenever a function is seen, but
// the reverse mapping is not updated to save time
void Symbols::UpdateID2NameForFtnSymbols()
{
    for ( auto& entry : Name2ID )
    {
        const string& ftnname = entry.first;
        const IDNoType& ftnid = entry.second;
        ID2Name[ftnid] = ftnname;
    }
}

void Symbols::UpdateID2NameForObjSymbols()
{
    for ( auto& sym : syms )
    {
        const string& objname = sym.GetName();
        const IDNoType& objid = sym.GetID();
        ID2Name[objid] = objname;
        // TODO Do we also need Name2ID mapping for object symbols?
        // Name2ID[objname] = objid;
    }
}
