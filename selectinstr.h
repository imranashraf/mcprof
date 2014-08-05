#ifndef SELECTINSTR_H
#define SELECTINSTR_H

#include "globals.h"
#include "utility.h"
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <stack>
#include <set>
#include <map>
#include <deque>
// #include <unordered_map>
#include <algorithm>

using namespace std;

class FtnList
{
    private:
        // set<string> fNames;
        vector<string> fNames;
        // deque <string> fNames;
//         unordered_map<string,bool>fNames;
        string selInstrfilename;

    public:
        FtnList()
        {
            Add(UnKnownFtn);
        }
        void Add(string fname)
        {
            fNames.push_back(fname);
            NametoADD[fname]=GlobalFunctionNo;   // create the string -> Number binding
            ADDtoName[GlobalFunctionNo]=fname;   // create the Number -> String binding
            D1ECHO("Adding " << VAR(fname) << "(" << GlobalFunctionNo << ") " << "to SeenFname list");
            GlobalFunctionNo++;      // create a Function Number for this function
        }

        bool Find(string fname)
        {
        //     if( !fNames.count(fname)) { // First time seeing this valid function name
            if(find(fNames.begin(), fNames.end(), fname) == fNames.end())
                return false;
            else
                return true;
        }

        void AddIfNotFound(string fname)
        {
            if( !Find(fname) ) {
                Add(fname);
            }
        }

        void InitFromFile()
        {
            selInstrfilename = "SelecInstrList.txt";
            string fname;
            ifstream sifin;
            sifin.open(selInstrfilename.c_str());
            if ( sifin.fail() ) {
                ECHO("Can not open the selected instrumentation function list file ("
                        <<selInstrfilename.c_str() << ")... Aborting!");
                return;
            }
            if(isEmpty(sifin)){
                ECHO( "Specified selected instrumentation function list file ("
                        << selInstrfilename.c_str()<<") is empty\n"
                        << "No function to instrument"
                        << "Specify at least 1 function in the list... Aborting!");
                return;
            }

            while(sifin >> fname) { // while there are function names in file
                Add(fname);
            }
            sifin.close();
        }
};

#endif