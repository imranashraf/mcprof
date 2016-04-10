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
    IDNoType currFtnID;
    IDNoType currFtnIDInOrder;
    u64 instrCount;
    u64 nCalls;
    NodeType * parent;
    vector<NodeType> children;
}Node;

static IDNoType currFtnIDInOrderCounter=1;
static map<IDNoType,Node*> FID2NodePtr;

class CallGraph
{
private:
    Node head;
    Node * currNode; // points to currect function
public:
    CallGraph() 
    {
        head.currFtnID = UnknownID;
        head.instrCount = 0;
        head.nCalls = 1;
        head.parent=NULL;
        currNode = &head;
    }
    void UpdateCall(IDNoType callee)
    {
        D1ECHO("Call to " << symTable.GetSymName(callee) );
        vector<Node> & childrenVec = currNode->children;
        vector<Node>::iterator it;
        for(it = childrenVec.begin(); it != childrenVec.end(); ++it)
        {
            if( callee == it->currFtnID )
            {
                currNode = &(*it);
                currNode->nCalls += 1;
                break;
            }
        }

        if( it == childrenVec.end() )  // if not found
        {
            Node n;
            n.currFtnID = callee;
            n.currFtnIDInOrder=currFtnIDInOrderCounter++;
            n.instrCount = 0;
            n.nCalls = 1;
            n.parent = currNode;
            childrenVec.push_back(n);
            currNode = &( childrenVec.back() );
            FID2NodePtr[n.currFtnIDInOrder] = currNode;
            cout << "Adding mapping "
                 << n.currFtnIDInOrder << " -> "
                 << symTable.GetSymName( FID2NodePtr[n.currFtnIDInOrder]->currFtnID ) << endl;
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
        cout << setw(indent) << ' ' << symTable.GetSymName(n.currFtnID) 
             << "( " << n.nCalls << " , " << n.instrCount << " )" << endl;
        for( u32 i=0; i < n.children.size(); ++i )
        {
            PrintRec( n.children[i], indent+4 );
        }
    }

    void Print()
    {
        cout << "Printing Callgraph\n";
        // PrintNode( head, 0);
        PrintRec( head.children[0], 0); // Printing starts from main
    }

    void PrintCallChainsRec(Node &n, vector<IDNoType> & callchain)
    {
        callchain.push_back(n.currFtnID);
//              << "( " << n.nCalls << " , " << n.instrCount << " )" << endl;
        if(n.children.size() == 0) // leaf
        {
            for(u32 i=0; i<callchain.size(); i++)
                cout<< symTable.GetSymName( callchain[i] ) << " > ";
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
        cout << "Printing Callchains\n";
        vector<IDNoType> callchain;
        PrintCallChainsRec( head.children[0], callchain); // Printing starts from main
    }

    void PrintJCallChainsRec(Node &n, vector<IDNoType> & callchain, ofstream & jout)
    {
        callchain.push_back(n.currFtnIDInOrder);

        if(n.children.size() == 0) // leaf
        {
            jout << "    {\n    \"callchain\": [";
            for(u32 i=callchain.size() ; i>0; --i)
            {
                //cout << i << " " << callchain[i-1]-1;
                jout<< callchain[i-1]-1;
                if(i>1) jout << ", ";
            }
            //cout << endl;
            jout << "],\n";
            jout << "    \"cost\": [ " << FID2NodePtr[ callchain[callchain.size() -1] ]->instrCount << " ]\n";
            jout << "    },\n";
            callchain.pop_back();
        }
        else
        {
            for( u32 i=0; i < n.children.size(); ++i )
            {
                PrintJCallChainsRec( n.children[i], callchain, jout );
            }
        }
    }

    void PrintJCallChains(ofstream & jout)
    {
        vector<IDNoType> callchain;
        PrintJCallChainsRec( head.children[0], callchain, jout); // Printing starts from main
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
        jout << "\"functions\":\n"
             << "[\n";

        for(u32 f=0; f<FID2NodePtr.size(); ++f)
        {
            //cout << "Mapping of ordered ID " << f+1 << endl;
            if(f>0)
                jout << ",\n";

            jout << "  {\n"
                 << "    \"name\": \""
                 << symTable.GetSymName( FID2NodePtr[f+1]->currFtnID )
                 << "\"\n"
                 << "  }";
        }
        jout << "\n],\n";

        // Print callchains as events
        jout << "\"events\":\n"
             << "[\n";
        PrintJCallChains(jout);
        jout << "]\n";

        jout << "}" << endl;
        jout.close();
    }
};

#endif
