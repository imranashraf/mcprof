#ifndef COMMATRIX_H
#define COMMATRIX_H

#include "globals.h"
#include "symbols.h"

#include <vector>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <iomanip>

extern Symbols symTable;

using namespace std;

class Matrix2D
{
private:
    vector< vector<float> > Matrix;
    static const int DEFAULT_SIZE=255; //most of the applications have functions less than 256

public:
    Matrix2D()
    {
        IDNoType cols=DEFAULT_SIZE;
        IDNoType rows=DEFAULT_SIZE;
        float value=0.0f;

        Matrix.resize( cols , vector<float>( rows , value) );
    }

    Matrix2D(IDNoType size)
    {
        IDNoType cols=size;
        IDNoType rows=size;
        float value=0.0;

        Matrix.resize( cols , vector<float>( rows , value) );
    }

    void RecordCommunication(IDNoType prod, IDNoType cons, u32 size)
    {
        D2ECHO("Recording Communication b/w " << FUNC(prod) << " and "
            << FUNC(cons) << " of size: " << size );
        
        if( prod < Matrix.size() && cons < Matrix.size() )
            Matrix[prod][cons] += size;
    }

    float MaxCommunication()
    {
        float currmax=0.0f;
        // Following iterations can be optimized to TotalFtns instead of size
        for (IDNoType p=0; p<Matrix.size(); p++)
        {
            for (IDNoType c=0; c<Matrix.size(); c++)
            {
                currmax = std::max(currmax, Matrix[p][c]);
            }
        }
        return currmax;
    }

    void Print(ostream &fout)
    {
        IDNoType TotalFtns = symTable.TotalSymbolCount();
        for (IDNoType p=0; p<TotalFtns; p++)
        {
            for (IDNoType c=0; c<TotalFtns; c++)
            {
                fout << setw(12) << Matrix[p][c] <<" ";
            }
            fout<<endl;
        }
    }

    void PrintMatrix(ostream &fout)
    {
        IDNoType TotalFtns = symTable.TotalSymbolCount();
        fout << setw(25) << " ";
        for (IDNoType c=0; c<TotalFtns; c++)
        {
            if ( symTable.SymIsFunc(c) )
                fout << setw(25) << symTable.GetSymName(c);
        }
        fout << endl;

        for (IDNoType p=0; p<TotalFtns; p++)
        {
            if ( symTable.SymIsFunc(p) )
            {
                fout << setw(25) << symTable.GetSymName(p);
            
                for (IDNoType c=0; c<TotalFtns; c++)
                {
                    if ( symTable.SymIsFunc(c) )
                        fout << setw(25) << Matrix[p][c];
                }
                fout<<endl;
            }
        }
    }

    void PrintDot(ostream &dotout)
    {
        IDNoType TotalFtns = symTable.TotalSymbolCount();
        dotout << "digraph {\ngraph [];"
//                << "\nnode [fontcolor=black, style=filled, fontsize=20];"
               << "\nedge [fontsize=14, arrowhead=vee, arrowsize=0.5];"
               << endl;

        CHECK_LT(TotalFtns, Matrix.size());

        string objNodeStyle("fontcolor=black, shape=box, fontsize=20");
        string ftnNodeStyle("fontcolor=black, style=filled, fontsize=20");
        for (IDNoType c=0; c<TotalFtns; c++)
        {
            if ( symTable.SymIsObj(c) )
                dotout << "\"" << c << "\"" << " [label=\"" << symTable.GetSymName(c) << "\"" << objNodeStyle << "];" << endl;
            else
                dotout << "\"" << c << "\"" << " [label=\"" << symTable.GetSymName(c) << "\"" << ftnNodeStyle << "];" << endl;
        }

        int color;
        float maxComm = MaxCommunication();

        for (IDNoType p=0; p<TotalFtns; p++)
        {
            for (IDNoType c=0; c<TotalFtns; c++)
            {
                float comm = Matrix[p][c];
                if(comm > 0 )
                {
                    color = (int) (  1023 *  log((float)(comm)) / log((float)maxComm)  );
                    dotout << dec
                           << "\"" << p << "\""
                           << "->"
                           << "\"" << c << "\""
                           << "[label=\""
                           << comm <<" Bytes\""
                           << "color = \"#"
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
};

#endif
