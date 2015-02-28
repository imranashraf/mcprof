#include "globals.h"
#include "commatrix.h"
#include "instrcount.h"

extern map <string,IDNoType> FuncName2ID;
extern bool ShowUnknown;

Matrix2D::Matrix2D()
{
    IDNoType cols=DEFAULT_SIZE;
    IDNoType rows=DEFAULT_SIZE;
    float value=0.0f;
    D1ECHO("Default Comm Matrix Size = " << DEFAULT_SIZE);
    Matrix.resize( cols , vector<float>( rows , value) );
}

Matrix2D::Matrix2D(IDNoType size)
{
    IDNoType cols=size;
    IDNoType rows=size;
    float value=0.0f;
    Matrix.resize( cols , vector<float>( rows , value) );
}

float Matrix2D::MaxCommunication(u16 StartID)
{
    float currmax=0.0f;
    // Following iterations can be optimized to TotalSymbols instead of size
    for (IDNoType p=StartID; p<Matrix.size(); p++)
    {
        for (IDNoType c=StartID; c<Matrix.size(); c++)
        {
            currmax = std::max(currmax, Matrix[p][c]);
        }
    }
    return currmax;
}

void Matrix2D::Print(ostream &fout)
{
    IDNoType TotalSymbols = symTable.TotalSymbolCount();
    CHECK_LT(TotalSymbols, Matrix.size());

    for (IDNoType p=0; p<TotalSymbols; p++)
    {
        for (IDNoType c=0; c<TotalSymbols; c++)
        {
            fout << setw(12) << Matrix[p][c] <<" ";
        }
        fout<<endl;
    }
}

// Use the following for properly aligned matrix print for visual inspection
// This can be problematic of width not set properly to be processed by gnuplot script
// #define ALIGNMENT (setw(25))

// Use the following to print tabs which will not be visually appealing but it will
// generate the columns properly for further processing by other tools
#define ALIGNMENT ("\t")

void Matrix2D::PrintMatrix(ostream &fout)
{
    ECHO("Printing communication matrix as table for processing by tool");
    IDNoType TotalSymbols = symTable.TotalSymbolCount();
    CHECK_LT(TotalSymbols, Matrix.size());

    u16 StartID;
    if(ShowUnknown)
        StartID = 0; //use this if you want to print unknown
    else
        StartID = 1; //use this if you dont want to print unknown

    fout << ALIGNMENT << " ";
    for (IDNoType c=StartID; c<TotalSymbols; c++)
    {
        fout << ALIGNMENT << symTable.GetSymName(c);
    }
    fout << endl;

    for (IDNoType p=StartID; p<TotalSymbols; p++)
    {
        fout << ALIGNMENT << symTable.GetSymName(p);

        for (IDNoType c=StartID; c<TotalSymbols; c++)
        {
            fout << ALIGNMENT << Matrix[p][c];
        }
        fout<<endl;
    }
}
#undef ALIGNMENT

#define UNORDERED 0
#define ORDERED 1
#define NODE ORDERED
// #define NODE UNORDERED
void Matrix2D::PrintDot(ostream &dotout)
{
    ECHO("Printing communication in DOT");
    u16 TotalSymbols = GlobalID;
    D1ECHO( VAR(TotalSymbols) );
    CHECK_LT(TotalSymbols, Matrix.size());

    dotout << "digraph {\ngraph [];"
//                << "\nnode [fontcolor=black, style=filled, fontsize=20];"
           << "\nedge [fontsize=14, arrowhead=vee, arrowsize=0.5];"
           << endl;

    string objNodeStyle(" fontcolor=black, shape=box, fontsize=20");
    string ftnNodeStyle(" fontcolor=black, style=filled, fontsize=20");

    u16 StartID;
    if(ShowUnknown)
        StartID = 0; // as 0 is reserved for unknown id
    else
        StartID = 1;

    float maxComm = MaxCommunication(StartID);

    #if (NODE == ORDERED)
    // this sorting of functions is done in the order of insertion to map
    // to print them in order in the graph
    vector<IDNoType> allFunIDs;
    map<IDNoType,string> ID2FunctionName;
    map<string,IDNoType>::iterator iter1;
    for( iter1 = FuncName2ID.begin(); iter1 != FuncName2ID.end(); iter1++)
    {
        const string& symname = iter1->first;
        IDNoType& fid = iter1->second;
        allFunIDs.push_back(fid); // insert in vector
        ID2FunctionName[fid] = symname; // make reverse map
    }

    //sort vector
    sort(allFunIDs.begin(), allFunIDs.end());

    // now print functions in order
    vector<IDNoType>::iterator iter2;
    for( iter2 = allFunIDs.begin(); iter2 != allFunIDs.end(); iter2++)
    {
        IDNoType& fid = *iter2;

        const string& symname = ID2FunctionName[fid];

        if( !symname.empty() && fid >= StartID )
        {
            dotout  << "\"" << (u16)fid << "\""
                    << " [label=\" " << symname
                    << " \\n" << GetInstrCountPercent(fid) << "%"
                    << ", " << GetCallCount(fid) << "\""
                    << ftnNodeStyle 
                    << "];" << endl;
        }
    }
    #endif

    // now print the objects
    for (u16 c=StartID; c<TotalSymbols; c++)
    {
        string symname = symTable.GetSymName(c);
        if( !symname.empty() )
        {
            if ( symTable.SymIsObj(c) )
            {
                dotout  << "\"" << (u16)c << "\"" << " [label=\" " << symname 
                        << " \\n" << hBytes(symTable.GetSymSize(c)) << "\"" 
                        << objNodeStyle 
                        << "];" << endl;
            }
            #if (NODE == UNORDERED)
            else
            {
                dotout  << "\"" << (u16)c << "\""
                        << " [label=\" " << symname
                        << " \\n" << GetInstrCountPercent(c) << "%"
                        << ", " << GetCallCount(c) << "\""
                        << ftnNodeStyle 
                        << "];" << endl;
            }
            #endif
        }
    }

    int color;

    for (u16 p=StartID; p<TotalSymbols; p++)
    {
        for (u16 c=StartID; c<TotalSymbols; c++)
        {
            float comm = Matrix[p][c];
            if(comm > 0 )
            {
                color = (int) (  1023 *  log((float)(comm)) / log((float)maxComm)  );
                dotout << dec
                       << "\"" << (u16)p << "\""
                       << "->"
                       << "\"" << (u16)c << "\""
                       << "[ label=\""
                       << hBytes(comm) <<"\""
                       << " color = \"#"
                       << hex
                       << setw(2) << setfill('0') << max(0, color-768)
                       << setw(2) << setfill('0') << min(255, 512-(int)abs(color-512))
                       << setw(2) << setfill('0') << max(0, min(255,512-color))
                       << "\""
                       << "]"
                       << endl;
            }
        }
    }

    dotout << "}" << endl;
}
