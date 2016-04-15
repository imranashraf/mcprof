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
    IDNoType GID;
    IDNoType LID;
    u64 instrCount;
    u64 nCalls;
    NodeType * parent;
    vector<NodeType> children;
}Node;

static IDNoType LIDCounter=0;
static map<IDNoType,IDNoType> GID2LID;
static map<IDNoType,IDNoType> LID2GID;
static map<IDNoType,Node*> LID2NodePtr;

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

//         head.GID = UnknownID;
//         head.instrCount = 0;
//         head.nCalls = 1;
//         head.parent=NULL;
//         currNode = &head;
//         head.LID = LIDCounter++;
//         GID2LID[head.GID] = head.LID;
//         LID2GID[head.LID] = head.GID;
//         LID2NodePtr[head.LID] = currNode;
        currNode=NULL;
    }
    void UpdateCall(IDNoType callee)
    {
        ECHO("Call to " << symTable.GetSymName(callee) );
        if(currNode == NULL)
        {
            head.GID = callee;
            head.instrCount = 0;
            head.nCalls = 1;
            head.parent=NULL;
            currNode = &head;
            head.LID = LIDCounter++;
            GID2LID[head.GID] = head.LID;
            LID2GID[head.LID] = head.GID;
            LID2NodePtr[head.LID] = currNode;
            ECHO("Adding first mapping " << head.LID << " -> " << symTable.GetSymName( LID2NodePtr[head.LID]->GID ) );
        }
        else
        {
            vector<Node> & childrenVec = currNode->children;
            vector<Node>::iterator it;
            for(it = childrenVec.begin(); it != childrenVec.end(); ++it)
            {
                if( callee == it->GID )
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
                n.GID = callee;
                n.instrCount = 0;
                n.nCalls = 1;
                n.parent = currNode;
                childrenVec.push_back(n);
                currNode = &( childrenVec[ childrenVec.size()-1 ] );
                //currNode = &( childrenVec.back() );

                auto it = GID2LID.find(n.GID);
                if( it == GID2LID.end() )
                {
                    n.LID=LIDCounter++;
                    LID2GID[n.LID] = n.GID;
                    GID2LID[n.GID] = n.LID;
                    LID2NodePtr[n.LID] = currNode;
                    //ECHO("Adding new mapping " << n.LID << " -> " << symTable.GetSymName( LID2NodePtr[n.LID]->GID ) );
                    ECHO("Adding new mapping " << n.LID << " -> " << symTable.GetSymName( currNode->GID ) );
                }
                else
                {
                    ECHO("using existing mapping " << n.LID << " -> " << symTable.GetSymName( LID2NodePtr[n.LID]->GID ) );
                    n.LID = GID2LID[n.GID];
                }
            }
        }
        cout << " Current mappings are : \n";
        for(u32 lid=0; lid < ( LID2NodePtr.size() ); ++lid)
        {
            cout << "LID(" << lid << ") -> " 
                 << symTable.GetSymName( LID2NodePtr[lid]->GID ) 
                 << "( " << LID2NodePtr[lid] <<" )"
                 << endl;
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
        cout << setw(indent) << ' ' << symTable.GetSymName(n.GID) 
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
        callchain.push_back(n.GID);
        if(n.children.size() == 0) // leaf
        {
            for(u32 i=0; i<callchain.size(); i++)
            {
                if(i>0) cout << " > ";
                cout<< symTable.GetSymName( callchain[i] );
                cout<< "(" << GID2LID[ callchain[i] ] << ") ";
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
        cout << "\nPrintJCallChainsRec for " 
             << symTable.GetSymName( LID2NodePtr[n.LID]->GID ) << endl;

        callchain.push_back(n.LID);

        jout << "    {\n    \"callchain\": [";
        u32 callchainSize = callchain.size();
        cout << "callchain size = " << callchainSize << endl;
        cout << "callchain : ";
        for(u32 i=0; i<callchainSize; ++i)
        {
            if(i>0)
            {
                jout << ", ";
                cout << ", ";
            }
            cout << callchain[callchainSize-1 - i];
            jout << callchain[callchainSize-1 - i];
        }
        cout << endl;
        jout << "],\n";
        jout << "    \"cost\": [ " << LID2NodePtr[ callchain[callchainSize-1] ]->instrCount << " ]\n";
        jout << "    }";

        u32 nChild = n.children.size();
        cout << "Number of children = " << nChild << endl;
        if( 0 == nChild ) // leaf
        {
            cout << "Leaf, so removing from callchain." << endl;
            callchain.pop_back();
        }
        else
        {
            for( u32 i=0; i < nChild; ++i )
            {
                cout << "Non Leaf, Going to call PrintJCallChainsRec for " 
                     //<< symTable.GetSymName( (n.children[i]).GID ) << endl;
                     << symTable.GetSymName( LID2NodePtr[ (n.children[i]).LID ]->GID ) << endl;
                jout << ",\n";
                PrintJCallChainsRec( n.children[i], callchain, jout );
            }
        }
    }

    void PrintJCallChains(ofstream & jout)
    {
        vector<IDNoType> callchain;
        // PrintJCallChainsRec( head.children[0], callchain, jout);
        PrintJCallChainsRec( head, callchain, jout);
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
        ECHO("Printing functions.");
        jout << "\"functions\":\n"
             << "[\n";

        for(u32 lid=0; lid < ( LID2NodePtr.size() ); ++lid)
        {
            if(lid>0)
            {
                jout << ",\n";
            }

            cout << "LID(" << lid << ") -> " << symTable.GetSymName( LID2NodePtr[lid]->GID ) << endl;

            jout << "  {\n"
                 << "    \"name\": \""
                 << symTable.GetSymName( LID2NodePtr[lid]->GID )
                 << "( " << LID2NodePtr[lid] <<" )"
                 << "\"\n"
                 << "  }";
        }
        jout << "\n],\n";
        cout << "\n";

        // Print callchains as events
        ECHO("Printing events.");
        jout << "\"events\":\n"
             << "[\n";
        PrintJCallChains(jout);
        jout << "\n]\n";

        jout << "}" << endl;
        jout.close();
    }
};

#endif
