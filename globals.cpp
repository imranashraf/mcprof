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

#include <cmath>
#include <string>
#include <map>
#include "globals.h"
#include "pin.H"

using namespace std;

IDNoType GlobalID=UnknownID;
char currDir[FILENAME_MAX];

void PrintLogo()
{
    cerr << " __       __   ______   _______   _______    ______   ________ "<<endl;
    cerr << "/  \\     /  | /      \\ /       \\ /       \\  /      \\ /        |"<<endl;
    cerr << "$$  \\   /$$ |/$$$$$$  |$$$$$$$  |$$$$$$$  |/$$$$$$  |$$$$$$$$/ "<<endl;
    cerr << "$$$  \\ /$$$ |$$ |  $$/ $$ |__$$ |$$ |__$$ |$$ |  $$ |$$ |__    "<<endl;
    cerr << "$$$$  /$$$$ |$$ |      $$    $$/ $$    $$< $$ |  $$ |$$    |   "<<endl;
    cerr << "$$ $$ $$/$$ |$$ |   __ $$$$$$$/  $$$$$$$  |$$ |  $$ |$$$$$/    "<<endl;
    cerr << "$$ |$$$/ $$ |$$ \\__/  |$$ |      $$ |  $$ |$$ \\__$$ |$$ |      "<<endl;
    cerr << "$$ | $/  $$ |$$    $$/ $$ |      $$ |  $$ |$$    $$/ $$ |      "<<endl;
    cerr << "$$/      $$/  $$$$$$/  $$/       $$/   $$/  $$$$$$/  $$/       "<<endl;
    cerr << "                                                               "<<endl;

}

bool isEmpty(ifstream& fin)
{
    return fin.peek() == ifstream::traits_type::eof();
}

void OpenInFile(const string& fileName, ifstream& fin)
{
    if (!fileName.empty())
    {
        fin.open(fileName.c_str());
        if ( fin.fail() )
        {
            ECHO("Can not open input file (" <<fileName.c_str() << "). Aborting...");
            Die();
        }
    }
    else
    {
        ECHO("Specify a non empty file name. Aborting ...");
        Die();
    }

    if(isEmpty(fin))
    {
        ECHO("Input file (" <<fileName.c_str()<<") is empty. Aborting...");
        Die();
    }
}

void OpenOutFile(const string& fileName, ofstream& fout)
{
    if (!fileName.empty())
    {
        fout.open(fileName.c_str(), ios::binary);
        if ( fout.fail() )
        {
            ECHO("Can not open output file (" << fileName.c_str() << "). Aborting ...");
            Die();
        }
    }
    else
    {
        ECHO("Specify a non empty file name. Aborting ...");
        Die();
    }
}

bool OpenInFileIfExists(const string& fileName, ifstream& fin)
{
    if ( !fileName.empty() )
    {
        fin.open(fileName.c_str());
        if ( fin.fail() )
        {
            ECHO("Input file (" << fileName.c_str() << ") does not exist ...");
            return false;
        }
    }
    else
    {
        ECHO("Specify a non empty file name. Aborting ...");
        Die();
    }
    return true;
}

const string& Target2RtnName(uptr target)
{
    const string& name = RTN_FindNameByAddress(target);

    if (name == "")
        return *new string("[Unknown routine]");
    else
        return *new string(name);
}

const string& Target2LibName(uptr target)
{
    PIN_LockClient();
    const RTN rtn = RTN_FindByAddress(target);
    static const string _invalid_rtn("[Unknown image]");
    string name;

    if( RTN_Valid(rtn) )
        name = IMG_Name(SEC_Img(RTN_Sec(rtn)));
    else
        name = _invalid_rtn;

    PIN_UnlockClient();
    return *new string(name);
}

inline bool IsPowerOfTwo(uptr x)
{
    return (x & (x - 1)) == 0;
}

inline uptr RoundUpTo(uptr size, uptr boundary)
{
    CHECK(IsPowerOfTwo(boundary));
    return (size + boundary - 1) & ~(boundary - 1);
}

inline uptr RoundDownTo(uptr x, uptr boundary)
{
    return x & ~(boundary - 1);
}

inline bool IsAligned(uptr a, uptr alignment)
{
    return (a & (alignment - 1)) == 0;
}

const string& humanReadableByteCount(u64 bytes, bool si)
{
    string hBytes;
    char cBytes[100];
    u16 unit = si ? 1000 : 1024;
    string punits1 = (si ? "kMGTPE" : "KMGTPE");
    char punits2 = (si ? '\0' : 'i');
    char pre[3];

    if (bytes < unit)
        hBytes = to_string((long long)bytes) + " B";
    else
    {
        int exp = (int) (log((double)bytes) / log((double)unit));
        pre[0] = punits1[exp-1];
        pre[1] = punits2;
        pre[2] = '\0';
        float val = bytes / pow((double)unit, (double)exp);
        sprintf(cBytes, "%.1f %sB", val, pre);
        hBytes = cBytes;
    }

    return *new string(hBytes);
}

const string& hBytes(u64 bytes)
{
    return humanReadableByteCount(bytes, false);
}

void SetCurrDir()
{
    if (!GetCurrentDir(currDir, sizeof(currDir)))
    {
        ECHO("Error setting current dir ");
        Die();
    }
    //cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
    strcat(currDir, "/");
}

void PrintCurrDir()
{
    ECHO("The current working directory is " << currDir);
}

void RemoveSubstrs(string& src, string& toRemove)
{
    int n = toRemove.length();
    size_t i;
    for( i = src.find(toRemove); i != string::npos; i = src.find(toRemove) )
        src.erase(i, n);
}

void RemoveCurrDirFromName(string& src)
{
    string cdir(currDir);
    RemoveSubstrs(src, cdir);
}

// a simple utility function to give persistence appended
// names to functions/zones
void AddNoToNameEnd(string& name, IDNoType id)
{
    name += ( "_" + to_string((long long)id) );
}

void RemoveNoFromNameEnd(string& name, IDNoType& id)
{
    size_t found = name.find_last_of("_");
    if( found != string::npos )
    {
        string nostr = name.substr(found+1);
        // id = stoul( nostr , nullptr , 0);
        id = strtoul( nostr.c_str() , NULL, 0);
        name =  name.substr(0,found);
        D3ECHO(" name: " << name);
        D3ECHO(" id: " << id );
    }
}
