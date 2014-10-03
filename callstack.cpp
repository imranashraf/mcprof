#include "globals.h"
#include "shadow.h"
#include "callstack.h"
#include "commatrix.h"

extern std::map <std::string,IDNoType> FuncName2ID;

extern Symbols symTable;
extern LocationList Locations;

void CallStackType::Print()
{
    string stackftns("");
    if( !stack.empty()) stackftns += symTable.GetSymName(stack[0]);
    for ( u16 ftn=1; ftn < stack.size(); ftn++)
        stackftns += " -> " + symTable.GetSymName(stack[ftn]);
    cout << "Call Stack : " << stackftns << "\n";
    D1ECHO("Call Stack : " << stackftns);
}

void CallStackType::Print(ofstream& fout)
{
    string stackftns("");
    if( !stack.empty()) stackftns += symTable.GetSymName(stack[0]);
    for ( u16 ftn=1; ftn < stack.size(); ftn++)
        stackftns += " -> " + symTable.GetSymName(stack[ftn]);
    fout << "Call Stack : " << stackftns << "\n";
    D1ECHO("Call Stack : " << stackftns);
}

u32 CallSiteStackType::GetCallSites()
{
    u32 callsites=0;
    for ( u32 loc=1; loc < sites.size(); loc++) // first callsite will be of call to main
    {
        callsites += loc*sites[loc];    // arbitrary formula to combine call sites
    }
    return callsites;
}

void CallSiteStackType::Print()
{
    string callsites("");
    if( !sites.empty()) callsites += Locations.GetLocation(sites[0]).toString();
    for ( u16 loc=1; loc < sites.size(); loc++)
        callsites += " -> " + Locations.GetLocation(sites[loc]).toString();
    cout << "Call sites : " << callsites << "\n";
}

void CallSiteStackType::Print(ofstream& fout)
{
    string callsites("");
    if( !sites.empty()) callsites += Locations.GetLocation(sites[0]).toString();
    for ( u16 loc=1; loc < sites.size(); loc++)
        callsites += " -> " + Locations.GetLocation(sites[loc]).toString();
    fout << "Call sites : " << callsites << "\n";
}
