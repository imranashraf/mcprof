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
    if( !stack.empty() )
    {
        stackftns += symTable.GetSymName(stack[0]);
    }

    for ( u16 ftn=1; ftn < stack.size(); ftn++)
        stackftns += " > " + symTable.GetSymName(stack[ftn]);

    D1ECHO("Call Stack: " << stackftns);
}

void CallStackType::Print(ofstream& fout)
{
    #if 1
    string stackftns("");
    if( !stack.empty() )
    {
        stackftns += symTable.GetSymName( stack[ stack.size()-1 ] );
    }
    fout << stackftns;

    #else
    // The following is the detailed printing of callstack
    if( !stack.empty() )
    {
        stackftns += symTable.GetSymName(stack[0]);
    }

    u16 ftn=1;
    while( ftn < stack.size() )
    {
        if( stack[ftn] == stack[ftn-1] ) // check for recursion
        {
            stackftns += " > " + symTable.GetSymName(stack[ftn]) + " > ... > ";
            // do not print all functions in recursion.
            while( stack[ftn] == stack[ftn-1] && ftn < stack.size() )
            {
                ftn++;  // simply increment to skip
            }
            stackftns += symTable.GetSymName(stack[ftn-1]);
            ftn++;
        }
        else
        {
            stackftns += " > " + symTable.GetSymName(stack[ftn]);
            ftn++;
        }
    }

    fout << "Call Stack: " << stackftns << "\n";
    D1ECHO("Call Stack: " << stackftns);
    #endif
}

u32 CallSiteStackType::GetCallSites(u32 lastCallLocIndex)
{
    u32 callsites=0;
    u32 loc;
    for ( loc=1; loc < sites.size(); loc++) // first callsite will be of call to main
    {
        callsites += loc*sites[loc];    // arbitrary formula to combine call sites
    }
    callsites += loc*lastCallLocIndex; // also add the effect of lastCallLocIndex

    return callsites;
}

string CallSiteStackType::GetCallSitesString()
{
    string callsites("");
    if( !sites.empty() )
    {
        callsites += Locations.GetLocation(sites[0]).toString();
    }
    for ( u16 loc=1; loc < sites.size(); loc++)
        callsites += ">" + Locations.GetLocation(sites[loc]).toString();
    return callsites;
}

void CallSiteStackType::Print()
{
    string callsites("");
    if( !sites.empty() )
    {
        callsites += Locations.GetLocation(sites[0]).toString();
    }

    for ( u16 loc=1; loc < sites.size(); loc++)
        callsites += " > " + Locations.GetLocation(sites[loc]).toString();
    D1ECHO("Call sites: " << callsites);
}

void CallSiteStackType::Print(ofstream& fout)
{
    string callsites("");
    if( !sites.empty() )
    {
        callsites += Locations.GetLocation(sites[0]).toString();
    }

    for ( u16 loc=1; loc < sites.size(); loc++)
        callsites += " > " + Locations.GetLocation(sites[loc]).toString();
    fout << "Call sites: " << callsites << "\n";
}
