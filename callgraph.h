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

#ifndef CALLGRAPH_H
#define CALLGRAPH_H

#include "json.h"

#include <set>
#include <list>

extern Symbols symTable;

typedef struct NodeType
{
    IDNoType ID;
    u64 instrCount;
    u64 nCalls;
    NodeType * parent;
    list<NodeType> children;
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
    void UpdateCall(IDNoType callee, u64 rInstrCount)
    {
        if(currNode == NULL)
        {
            head.ID = callee;
            //head.instrCount = 0;
            head.instrCount = rInstrCount;
            head.nCalls = 1;
            head.parent=NULL;
            currNode = &head;
        }
        else
        {
            D1ECHO("Call to " << symTable.GetSymName(callee) << " from " << symTable.GetSymName(currNode->ID) );
            list<Node> & currNodechildren = currNode->children;
            list<Node>::iterator it;
            u32 index;
            for(index=0, it = currNodechildren.begin(); it != currNodechildren.end(); ++it, ++index)
            {
                if( callee == it->ID )
                {
                    D2ECHO( symTable.GetSymName(callee) << " found in the children");
                    // Save rInstrCount of previous function before switching
                    (currNode->instrCount) += rInstrCount;
                    currNode = &(*it);
                    (currNode->nCalls) += 1;
                    break;
                }
            }

            if( it == currNodechildren.end() )  // if not found
            {
                D2ECHO( symTable.GetSymName(callee) << " not found in the children");
                Node n;
                n.ID = callee;
                n.instrCount = 0;
                n.nCalls = 1;
                n.parent = currNode;
                currNodechildren.push_back(n);
                // Save rInstrCount of previous function before switching
                (currNode->instrCount) += rInstrCount;
                //currNode = &( currNodechildren[ currNodechildren.size()-1 ] );
                currNode = &( currNodechildren.back() );
            }
        }

        #if (DEBUG>0)
        ECHO("Updated situation");
        Print();
        #endif
    }

    void UpdateReturn(IDNoType retFrom, u64 rInstrCount)
    {
        D1ECHO("Return from " << symTable.GetSymName(currNode->ID) );
        (currNode->instrCount) += rInstrCount;
        currNode = currNode->parent;
        #if (DEBUG>0)
        ECHO("Updated situation");
        Print();
        #endif
    }

    void TotalInsCount(Node &n, u64 & count)
    {
        count +=  n.instrCount;
        for( auto& child : n.children )
            TotalInsCount( child, count );
    }

    void PrintRec(Node &n, u32 indent, u64 total, ostream & cgout = cout)
    {
        if(indent) cgout << setw(indent) << ' ';
        cgout << symTable.GetSymName(n.ID)
              << "( " << n.nCalls << " , " << floor( 100.0 * (n.instrCount)/total) << "% )" << endl;
//               << "( " << n.nCalls << " , " << n.instrCount << " )" << endl;

        for( auto& child : n.children )
        {
            PrintRec( child, indent+2, total, cgout );
        }
    }

    void PrintText()
    {
        ECHO("Printing callgraph to callgraph.out");
        ofstream cgout;
        OpenOutFile("callgraph.out", cgout);
        u64 totalCount = 0;
        TotalInsCount(head, totalCount);
        PrintRec( head, 0, totalCount, cgout);
        cgout.close();
    }
    void Print()
    {
        ECHO("Printing callgraph");
        u64 totalCount = 0;
        TotalInsCount(head, totalCount);
        PrintRec( head, 0, totalCount);
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

        for( auto& child : n.children )
        {
            PrintCallChainsRec( child, callchain );
        }

        callchain.pop_back();
    }

    void PrintCallChains()
    {
        ECHO("Printing Callchains");
        vector<IDNoType> callchain;
        PrintCallChainsRec( head, callchain); // Printing starts from main
    }

    void PrintJCallChainsRec(Node &n, vector<IDNoType> & callchain, json::Array & jevents)
    {
        json::Object jevent;
        callchain.push_back(n.ID);

        json::Array jcallchain;
        u32 callchainSize = callchain.size();
        for(u32 i=0; i<callchainSize; ++i)
        {
            jcallchain.push_back( callchain[callchainSize-1 - i] );
        }

        jevent["callchain"] = jcallchain;

        json::Array jcosts;
        jcosts.push_back((int) n.instrCount); // cast required because of ambiguity in json class
        jevent["cost"] = jcosts;

        jevent["calls"] = (int) n.nCalls;
    
        jevents.push_back(jevent);

        for( auto& child : n.children )
        {
            PrintJCallChainsRec( child, callchain, jevents );
        }

        callchain.pop_back();
    }

    set<IDNoType> SeenFuncs;
    void PrintJFunctionsRec(Node &n, json::Array & jfuncs)
    {
        if ( SeenFuncs.find(n.ID) == SeenFuncs.end() )
        {
            SeenFuncs.insert(n.ID);
            json::Object jfunc;
            jfunc["name"] = symTable.GetSymName(n.ID);
            jfunc["id"] = n.ID;
            jfuncs.push_back(jfunc);
        }

        for( auto& child : n.children )
        {
            PrintJFunctionsRec( child, jfuncs );
        }

    }

    void PrintJson()
    {
        ECHO("Printing callgraph to callgraph.json");
        ofstream jout;
        OpenOutFile("callgraph.json", jout);

        json::Object myjson;
        myjson["version"] = 0;

        json::Object jcosts;
        jcosts["description"] = "Callgraph Generated by MCProf";
        myjson["costs"] = jcosts;

        // Print functions
        D1ECHO("Printing functions in json.");
        json::Array jfuncs;
        PrintJFunctionsRec( head, jfuncs);
        myjson["functions"] = jfuncs;

        // Print callchains as events
        D1ECHO("Printing call events in json.");
        vector<IDNoType> callchain;
        json::Array jevents;
        PrintJCallChainsRec( head, callchain, jevents);
        myjson["events"] = jevents;

        string str = json::Serialize(myjson);
        jout << str << endl;
        jout.close();
    }
};

#endif
