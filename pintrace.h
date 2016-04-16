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
        ECHO("Call to " << symTable.GetSymName(callee) );
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
                    cout << " Found in the children " << endl;
                    currNode = &(*it);
                    currNode->nCalls += 1;
                    break;
                }
            }

            if( it == childrenVec.end() )  // if not found
            {
                cout << " Not found in the children " << endl;
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
        //PrintRec( head.children[0], 0); // Printing starts from main
    }

    void PrintCallChainsRec(Node &n, vector<IDNoType> & callchain)
    {
        callchain.push_back(n.ID);
        if(n.children.size() == 0) // leaf
        {
            for(u32 i=0; i<callchain.size(); i++)
            {
                if(i>0) cout << " > ";
                cout<< symTable.GetSymName( callchain[i] );
            }
            cout << endl;
            callchain.pop_back();
        }
        else
        {
            for( u32 i=0; i < n.children.size(); ++i )
            {
                PrintCallChainsRec( n.children[i], callchain );
            }
        }
    }

    void PrintCallChains()
    {
        ECHO("Printing Callchains");
        vector<IDNoType> callchain;
        PrintCallChainsRec( head, callchain); // Printing starts from main
    }

    void PrintJCallChainsRec(Node &n, vector<IDNoType> & callchain, ofstream & jout)
    {
        callchain.push_back(n.ID);

        jout << "    {\n    \"callchain\": [";
        u32 callchainSize = callchain.size();
        for(u32 i=0; i<callchainSize; ++i)
        {
            if(i>0)
            {
                jout << ", ";
            }
            jout << callchain[callchainSize-1 - i];
        }
        jout << "],\n";
        jout << "    \"cost\": [ " << n.instrCount << " ],\n";
        jout << "    \"calls\": " << n.nCalls << " \n";
        jout << "    }";

        u32 nChild = n.children.size();
        if( 0 == nChild ) // leaf
        {
            callchain.pop_back();
        }
        else
        {
            for( u32 i=0; i < nChild; ++i )
            {
                jout << ",\n";
                PrintJCallChainsRec( n.children[i], callchain, jout );
            }
        }
    }

    void PrintJCallChains(ofstream & jout)
    {
        vector<IDNoType> callchain;
        ECHO("Printing call events in json.");
        jout << "\"events\":\n"
             << "[\n";
        PrintJCallChainsRec( head, callchain, jout);
        jout << "\n]\n";
    }

    void PrintJFunctionsRec(Node &n, ofstream & jout)
    {
        jout << "    {" << "\"name\": \"" << symTable.GetSymName(n.ID) << "\""
             << ", \"id\": " << n.ID << " }";
        for( u32 i=0; i < n.children.size(); ++i )
        {
            jout << ",\n";
            PrintJFunctionsRec( n.children[i], jout );
        }
    }

    void PrintJFunctions(ofstream & jout)
    {
        ECHO("Printing functions in json.");
        jout << "\"functions\":\n"  << "[\n";
        PrintJFunctionsRec( head, jout);
        jout << "\n],\n";
    }

    void PrintJson()
    {
        ECHO("Printing callgraph.json");
        ofstream jout;
        OpenOutFile("callgraph.json", jout);

        // Print header
        jout << "{\n"
             << "\"version\": 0,\n"
             << "\"costs\":\n"
             << "[\n"
             << "   { \n"
             << "   \"description\": \"Dynamic Instruction Count\"\n"
             << "   }\n"
             << "],\n";

        // Print functions
        PrintJFunctions(jout);

        // Print callchains as events
        PrintJCallChains(jout);

        jout << "}" << endl;
        jout.close();
    }
};

#endif
