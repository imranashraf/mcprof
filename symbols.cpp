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
 * Copyright (c) 2014-2016 TU Delft, The Netherlands.
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

// #include <gelf.h>
#include <cstring>
#include <cctype>
#include "shadow.h"
#include "symbols.h"
#include "callstack.h"
#include "commatrix.h"

extern map <string,IDNoType> FuncName2ID;
extern map <string,IDNoType> CallSites2ID;
extern CallStackType CallStack;
extern CallSiteStackType CallSiteStack;
extern Matrix2D ComMatrix;
extern bool ShowUnknown;
extern bool TrackLoopDepend;
extern bool TrackTasks;

// AE/PE
extern map<IDNoType,double> objTotalACount;
extern map<IDNoType,double> objTotalPCount;

std::string locsFileName("locations.dat");

// List of all locations of symbols
LocationList Locations;

bool GetAvailableORNewID(IDNoType& id, u32 lastCallLocIndex)
{
    D2ECHO("Getting GetAvailableORNewID");
    bool result;
    string callsites("");
    CallSiteStack.GetCallSites(lastCallLocIndex, callsites);
    D2ECHO("callsites " << callsites);
    if(CallSites2ID.find(callsites) != CallSites2ID.end() )
    {
        // use existing id as this call site is already seen
        id = CallSites2ID[callsites];
        D1ECHO("callsites " << callsites << ", using existing id " << id);
        result = true;
    }
    else
    {
        // use a new id for this call site
        id = GlobalID++;
        CallSites2ID[callsites] = id;
        D1ECHO("callsites " << callsites << ", using new id " << id);
        result = false;
    }
    return result;
}

IDNoType GetNewID()
{
    return GlobalID++;
}


void LocationList::InitFromFile()
{
    ifstream locin;
    u32 counter=0;

    if ( OpenInFileIfExists(locsFileName, locin) )
    {
        string line;
        while ( getline(locin, line) )
        {
            //the following line trims white space from the beginning of the string
            line.erase(line.begin(), find_if(line.begin(), line.end(), not1(ptr_fun<int, int>(isspace))));

            // ignore empty lines and lines starting with #
            if (line.length() == 0 || line[0] == '#')
                continue;

            string filename;
            u32 lno;
            istringstream iss(line);
            if (!(iss >> filename >> lno))
            {
                break;    // error
            }
            Insert( Location(lno, filename) );
            ++counter;
        }

        locin.close();
    }

    ECHO("Initialized " << counter << " locations from file");
}

void LocationList::Print()
{
    std::ofstream locout;
    D1ECHO("Writing locations to " << locsFileName);
    // remove( locsFileName.c_str() ); // delete file TODO is it needed?
    OpenOutFile(locsFileName, locout);
    locout << "# list of locations in order" << endl;
    for(u32 i=0; i<locations.size(); ++i)
    {
        locout << locations[i].toString() << endl;
    }
    locout.close();
}

string& Symbols::GetSymName(IDNoType id)
{
    //D2ECHO("Getting name of symbol with id: " << id );
    Symbol& sym = _Symbols[id];
    return ( sym.GetName() );
}

u32 Symbols::GetSymSize(uptr saddr)
{
    IDNoType id = GetObjectID(saddr);
    D2ECHO("Getting symbol size for address " << ADDR(saddr) << " " << VAR(id));
    Symbol& sym = _Symbols[id];
    return ( sym.GetSize(saddr) );
}

u32 Symbols::GetSymSize(IDNoType id)
{
    D2ECHO("Getting total size of symbol with id: " << id );
    Symbol& sym = _Symbols[id];
    return ( sym.GetLastSize() );
}

u32 Symbols::GetTotalSymSize(IDNoType id)
{
    D2ECHO("Getting total size of symbol with id: " << id );
    Symbol& sym = _Symbols[id];
    return ( sym.GetTotalSize() );
}

string Symbols::GetSymLocation(IDNoType id)
{
    string loc;
    Symbol& sym = _Symbols[id];
    return sym.GetLocation();
}

void Symbols::InsertMallocCalloc(uptr saddr, u32 lastCallLocIndex, u32 size)
{
    D2ECHO("Inserting Malloc/Calloc ");

    IDNoType id=0;
    GetAvailableORNewID(id, lastCallLocIndex); //ignoring return value

    // To check if symbol is already in the table. This is possible because of:
    //      - the list of selected objects provided as input
    //      - multiple allocations from same line
    if( _Symbols.find(id) != _Symbols.end() )
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

    // set last consumer to UnknownID as its just allocated (AE/PE)
    SetLastConsumers(saddr, size, UnknownID);

    // Added for allocation dependencies.
    // also set the function in which this allocation is taking place
    // as the producer of this object. This should not be done when
    // tracking loop dependencies as it will cause extra communication/dependencies
    if( TrackTasks && !TrackLoopDepend )
    {
        IDNoType prod = CallStack.Top();
        for(u32 i=0; i<size; i++)
        {
            SetProducer(prod, saddr+i);
        }
    }
}

void Symbols::UpdateRealloc(IDNoType id, uptr prevSAddr, uptr saddr, u32 lastCallLocIndex, u32 size)
{
    D2ECHO("Updating Realloc ");
    Symbol& availSym = _Symbols[id];
    u32 prevSize = availSym.GetSize(prevSAddr);
    availSym.SetSize(saddr, size);

    // set the object ID of previous address range to unknown
    SetObjectIDs(prevSAddr, prevSize, UnknownID);
    // Now set the object ids in the shadow table/mem for this object
    SetObjectIDs(saddr, size, id);

    // (AE/PE)
    // get last consumer of previous address range
    IDNoType prevLastCons = GetLastConsumer(prevSAddr);
    // and copy it to new address range
    SetLastConsumers(saddr, size, prevLastCons);
    // Now set last consumer of previous address to UnknownID 
    SetLastConsumers(prevSAddr, prevSize, UnknownID);

    // Added for allocation dependencies
    if( TrackTasks && !TrackLoopDepend )
    {
        // set producer of previous address range to Unknown
        for(u32 i=0; i<prevSize; i++)
        {
            SetProducer( UnknownID, prevSAddr+i );
        }
        // now set the current producer to the new address range
        IDNoType prod = CallStack.Top();
        for(u32 i=0; i<size; i++)
        {
            SetProducer( prod, saddr+i );
        }
    }
}

void Symbols::InsertFunction(const string& ftnname, IDNoType id, u32 lastCallLocIndex)
{
    D2ECHO("Inserting Function " << ftnname);
    FuncName2ID[ftnname] = id;
    Symbol sym(id, ftnname, SymType::FUNC, lastCallLocIndex);
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
    // TODO:How do you deal with objects with multiple allocations
    // from the same location
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

    // AE/PE
    u64 ac, pc; // counts
    GetAEPECount(saddr, size, ac, pc);
    IDNoType oid = GetObjectID(saddr);
    D2ECHO("OBJ ID: " << oid << " size: " << size << " current ACount: " << ac << "  current PCount: " << pc);
    objTotalACount[oid] += ac; objTotalPCount[oid] = pc;

    // set last consumer to UnknownID as its freed now (AE/PE)
    SetLastConsumers(saddr, size, UnknownID);

    // Clear the obj ids for this object, which is same as setting it to UnknownID
    D2ECHO("Clearing object ID to " << UnknownID << " on a size " << size);
    SetObjectIDs(saddr, size, UnknownID);

    // Added for allocation dependencies.
    // Record the communication to show dependencies between
    // the last producer and the function freeing it.
    // Also clear the producer to Unknown to avoid future
    // recording of communication.
    if( TrackTasks && !TrackLoopDepend )
    {
        IDNoType prod = CallStack.Top();
        for(u32 i=0; i<size; i++)
        {
            IDNoType prevProd = GetProducer(saddr+i);
            ComMatrix.RecordCommunication(prevProd, prod, 1);
            SetProducer(UnknownID, saddr+i);
        }
    }
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

const char* StripPath(const char * path)
{
    const char * file = strrchr(path,DELIMITER_CHAR);
    if (file)
        return file+1;
    else
        return path;
}

bool ValidObjName(string name)
{
    return
        !(
            name.c_str()[0]=='_' ||
            name.c_str()[0]=='?' ||
            !name.compare("SOMEOTHERNAME")
        );
}

void Symbols::InsertStaticSymbols(int argc, char **argv)
{
    char fullBinName[500];
    char binName[500];
    // parse the command line arguments for the binary name
    for (int i=1; i<argc-1; i++)
    {
        if (!strcmp(argv[i],"--"))
        {
            strcpy(fullBinName,argv[i+1]);
            break;
        }
    }
    // strcpy(binName, StripPath(fullBinName));
    strcpy(binName, fullBinName );
    ECHO("Binary Name = "<< binName);

#if 1
    ECHO("Info: Support for static symbols with Pin 3.0 will be back soon");
    Die();
#else
    int elf_fd;
    if (( elf_fd  = open( binName, O_RDONLY, 0)) < 0)
    {
        printf("ERROR: Failed to open binary file:\n");
    }

    Elf* elf;
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        printf("ERROR: ELF library initialization failed: %s\n",elf_errmsg(-1));
    }

    elf = elf_begin(elf_fd, ELF_C_READ, NULL);

    if(elf==NULL)
    {
        printf("ERROR: ELF loading failed: %s\n", elf_errmsg(-1));
    }
    else
    {
        Elf_Scn *scn;
        int symbol_count, i;
        Elf_Data *edata =NULL;
        GElf_Shdr shdr;
        GElf_Sym sym;
        scn = NULL;

        while ((scn = elf_nextscn(elf, scn)) != NULL)
        {
            if (gelf_getshdr(scn, &shdr) != &shdr)
                printf( "getshdr() failed: %s.", elf_errmsg(-1));
            if(shdr.sh_type == SHT_SYMTAB)
            {
                edata = elf_getdata(scn, edata);
                symbol_count = shdr.sh_size / shdr.sh_entsize;

                // loop through to grab all symbols
                for(i = 0; i < symbol_count; i++)
                {
                    // libelf grabs the symbol data using gelf_getsym()
                    gelf_getsym(edata, i, &sym);

                    if(ELF32_ST_BIND(sym.st_info)==STB_GLOBAL &&
                            ELF32_ST_TYPE(sym.st_info)==STT_OBJECT && sym.st_size>0)
                    {
                        string sName( elf_strptr(elf, shdr.sh_link, sym.st_name) );
                        if( ValidObjName(sName) )
                        {
                            u64 sAddr = sym.st_value;
                            u32 sSize = sym.st_size;
                            IDNoType id = GlobalID++;
                            // create a new symbol
                            Symbol newsym(id, sAddr, sSize, sName, SymType::OBJ);
                            // insert this new symbol in symbol table
                            D1ECHO ( "Adding ELF Symbol " << sName << " ID " << int(id) << " start address " << ADDR(sAddr) << " size " << sSize);                        
                            _Symbols[id] = newsym;
                            // we also need to set the object ids in the shadow table/mem for this object
                            SetObjectIDs(sAddr, sSize, id);
                        }
                    }
                }
            }
        }
    }
    close(elf_fd);
#endif

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
        D2ECHO("Adding Object Symbol " << symname << "("<< id << ") to symbol table");
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
    D2ECHO("Printing Symbol with ID: " << id);
    if(!ShowUnknown && id==UnknownID)
        return;

    fout << "ID: " << id << " "
         << SymTypeName[symType] << " " << name << " ";

    string callsitestring = symCallSite.GetCallSitesString();
    fout << "callsitestring : " << callsitestring << endl;
    if(callsitestring == "" )
        fout << Locations.GetLocation(symLocIndex).toString() << endl;
    else
        fout << symCallSite.GetCallSitesString() << ">"
             << Locations.GetLocation(symLocIndex).toString() << endl;

    if(RecordAllAllocations)
    {
        map<uptr,vector<u32>>::iterator mIter;
        for(mIter=startAddr2Size.begin(); mIter!=startAddr2Size.end(); ++mIter)
        {
            auto& saddr = mIter->first;
            auto& sizes = mIter->second;
            fout << "    " << ADDR(saddr) << "(";

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
        string fname("symbols.dat");
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
    D1ECHO("Printed Symbol Table");
}
