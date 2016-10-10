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
    D2ECHO( VAR( stack.size() ) );
    for ( u16 ftn=1; ftn < stack.size(); ftn++)
        stackftns += symTable.GetSymName(stack[ftn]) + " ";

    ECHO("Call Stack: " << stackftns);
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

void CallSiteStackType::GetCallSites(u32 lastCallLocIndex, string& callsites)
{
    for (u32 loc=1; loc < sites.size(); loc++) // first callsite will be of call to main
        callsites += to_string( (long long) sites[loc]);    // arbitrary formula to combine call sites

    callsites += to_string( (long long) lastCallLocIndex); // also add the effect of lastCallLocIndex
}

string CallSiteStackType::GetCallSitesString()
{
    string callsites("");
    for ( u16 loc=1; loc < sites.size(); loc++)
        callsites += Locations.GetLocation(sites[loc]).toString() + " ";
    return callsites;
}

void CallSiteStackType::Print()
{
    string callsites("");
    D2ECHO( VAR( sites.size() ) );
    for ( u16 loc=1; loc < sites.size(); loc++)
        callsites += Locations.GetLocation(sites[loc]).toString() + " ";
    ECHO("Call sites: " << callsites);
}

void CallSiteStackType::Print(ofstream& fout)
{
    string callsites("");
    for ( u16 loc=1; loc < sites.size(); loc++)
        callsites += Locations.GetLocation(sites[loc]).toString();
    fout << "Call sites: " << callsites << "\n";
}
