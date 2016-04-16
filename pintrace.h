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

#ifndef PINTRACE_H
#define PINTRACE_H

#include <set>
#include <json.hpp>

using json = nlohmann::json;

extern Symbols symTable;

void SetupPin(int argc, char *argv[]);

typedef struct NodeType
{
    IDNoType ID;
    u64 instrCount;
    u64 nCalls;
    NodeType * parent;
    vector<NodeType> children;
}Node;

class CallGraph
{
private:
    Node head;
    Node * currNode; // points to currect function
public:
    CallGraph() 
    {
        // TODO another way to avoid condition in UpdateCall() would be to 
        // add the first node here in the constructor. It can be Unknown function
        // or main. To make it properly, this should be done if user wants to
        // print Unknown or not !

//         head.ID = UnknownID;
//         head.instrCount = 0;
//         head.nCalls = 1;
//         head.parent=NULL;
//         currNode = &head;
        currNode=NULL;
    }
    void UpdateCall(IDNoType callee)
    {
        D1ECHO("Call to " << symTable.GetSymName(callee) );
        if(currNode == NULL)
        {
            head.ID = callee;
            head.instrCount = 0;
            head.nCalls = 1;
            head.parent=NULL;
            currNode = &head;
        }
        else
        {
            vector<Node> & childrenVec = currNode->children;
            vector<Node>::iterator it;
            for(it = childrenVec.begin(); it != childrenVec.end(); ++it)
            {
                if( callee == it->ID )
                {
                    D1ECHO("Found in the children");
                    currNode = &(*it);
                    currNode->nCalls += 1;
                    break;
                }
            }

            if( it == childrenVec.end() )  // if not found
            {
                D1ECHO("Not found in the children");
                Node n;
                n.ID = callee;
                n.instrCount = 0;
                n.nCalls = 1;
                n.parent = currNode;
                childrenVec.push_back(n);
                currNode = &( childrenVec[ childrenVec.size()-1 ] );
                //currNode = &( childrenVec.back() );
            }
        }
    }

    void UpdateReturn(IDNoType retFrom, u64 rInstrCount)
    {
        D1ECHO("Return from " << symTable.GetSymName(retFrom) );
        currNode->instrCount += rInstrCount;
        currNode = currNode->parent;
    }

    void PrintRec(Node &n, u32 indent)
    {
        cout << setw(indent) << ' ' << symTable.GetSymName(n.ID) 
             << "( " << n.nCalls << " , " << n.instrCount << " )" << endl;
        for( u32 i=0; i < n.children.size(); ++i )
        {
            PrintRec( n.children[i], indent+4 );
        }
    }

    void Print()
    {
        ECHO("Printing Callgraph");
        PrintRec( head, 0);
    }

    void PrintCallChainsRec(Node &n, vector<IDNoType> & callchain)
    {
        callchain.push_back(n.ID);

        for(u32 i=0; i<callchain.size(); i++)
        {
            if(i>0) cout << " > ";
            cout<< symTable.GetSymName( callchain[i] );
        }
        cout << endl;

        for( u32 i=0; i < n.children.size(); ++i )
        {
            PrintCallChainsRec( n.children[i], callchain );
        }
        callchain.pop_back();
    }

    void PrintCallChains()
    {
        ECHO("Printing Callchains");
        vector<IDNoType> callchain;
        PrintCallChainsRec( head, callchain); // Printing starts from main
    }

    void PrintJCallChainsRec(Node &n, vector<IDNoType> & callchain, json & jevents)
    {
        json jevent;
        callchain.push_back(n.ID);

        vector<IDNoType> rcallchain;
        u32 callchainSize = callchain.size();
        for(u32 i=0; i<callchainSize; ++i)
        {
            rcallchain.push_back( callchain[callchainSize-1 - i] );
        }

        jevent["callchain"] = rcallchain;
        jevent["cost"] = {n.instrCount};
        jevent["calls"] = n.nCalls;
        jevents.push_back(jevent);

        for( u32 i=0; i < n.children.size(); ++i )
        {
            PrintJCallChainsRec( n.children[i], callchain, jevents );
        }
        callchain.pop_back();
    }

    set<IDNoType> SeenFuncs;
    void PrintJFunctionsRec(Node &n, json & jfuncs)
    {
        if ( SeenFuncs.find(n.ID) == SeenFuncs.end() )
        {
            SeenFuncs.insert(n.ID);
            json jfunc; 
            jfunc["name"] = symTable.GetSymName(n.ID);
            jfunc["id"] = n.ID;
            jfuncs.push_back(jfunc);
        }

        for( u32 i=0; i < n.children.size(); ++i )
        {
            PrintJFunctionsRec( n.children[i], jfuncs );
        }
    }

    void PrintJson()
    {
        ECHO("Printing callgraph.json");
        ofstream jout;
        OpenOutFile("callgraph.json", jout);

        json myjson;
        myjson["version"] = 0;

        json jcosts;
        jcosts["description"] = "Callgraph Generated by MCProf";
        myjson["costs"] = jcosts;

        // Print functions
        D1ECHO("Printing functions in json.");
        json jfuncs;
        PrintJFunctionsRec( head, jfuncs);
        myjson["functions"] = jfuncs;

        // Print callchains as events
        D1ECHO("Printing call events in json.");
        vector<IDNoType> callchain;
        json jevents;
        PrintJCallChainsRec( head, callchain, jevents);
        myjson["events"] = jevents;

        jout << myjson.dump(2) << endl;
        jout.close();
    }
};

#endif
