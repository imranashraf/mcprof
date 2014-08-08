#ifndef COMMATRIX_H
#define COMMATRIX_H

#include "globals.h"
#include <vector>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <iomanip>

using namespace std;


class Matrix2D
{
private:
    vector< vector<double> > Matrix;
    static const int DEFAULT_SIZE=255; //most of the applications have functions less than 256

public:
    Matrix2D()
    {
        u16 cols=DEFAULT_SIZE;
        u16 rows=DEFAULT_SIZE;
        double value=0.0;

        Matrix.resize( cols , vector<double>( rows , value) );
    }

    Matrix2D(u16 size)
    {
        u16 cols=size;
        u16 rows=size;
        double value=0.0;

        Matrix.resize( cols , vector<double>( rows , value) );
    }

    void Increment(u16 prod, u16 cons, u16 size)
    {
        if( prod<Matrix.size() && cons < Matrix.size() )
            Matrix[prod][cons] += size;
    }

    double MaxCommunication()
    {
        double currmax=0;
        // Following iterations can be optimized to TotalFtns instead of size
        for (u16 p=0; p<Matrix.size(); p++)
        {
            for (u16 c=0; c<Matrix.size(); c++)
            {
                currmax = std::max(currmax, Matrix[p][c]);
            }
        }
        return currmax;
    }

    void Print(ostream &fout, u16 TotalFtns)
    {
        for (u16 p=0; p<TotalFtns; p++)
        {
            for (u16 c=0; c<TotalFtns; c++)
            {
                fout << setw(12) << Matrix[p][c] <<" ";
            }
            fout<<endl;
        }
    }

    void PrintMatrix(ostream &fout, u16 TotalFtns)
    {
        fout << setw(25) << " ";
        for (u16 c=0; c<TotalFtns; c++)
        {
            fout << setw(25) << ADDtoName[c];
        }
        fout << endl;

        for (u16 p=0; p<TotalFtns; p++)
        {
            fout << setw(25) << ADDtoName[p];
            for (u16 c=0; c<TotalFtns; c++)
            {
                fout << setw(25) << Matrix[p][c];
            }
            fout<<endl;
        }
    }

    void PrintDot(ostream &dotout, u16 TotalFtns)
    {
        dotout << "digraph {\ngraph [];"
               << "\nnode [fontcolor=black, style=filled, fontsize=20];"
               << "\nedge [fontsize=14, arrowhead=vee, arrowsize=0.5];"
               << endl;

        CHECK_LT(TotalFtns, Matrix.size());

        for (u16 c=0; c<TotalFtns; c++)
        {
            dotout << "\"" << c << "\"" << " [label=\"" << ADDtoName[c] << "\"];" << endl;
        }

        int color;
        double maxComm = MaxCommunication();

        for (u16 p=0; p<TotalFtns; p++)
        {
            for (u16 c=0; c<TotalFtns; c++)
            {
                double comm = Matrix[p][c];
                if(comm > 0 )
                {
                    color = (int) (  1023 *  log((double)(comm)) / log((double)maxComm)  );
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

void RecordCommunication(FtnNo prod, FtnNo cons, int size);
void PrintCommunication(ostream &fout, u16 TotalFtns);
void PrintMatrix(ostream &fout, u16 TotalFtns);
void PrintCommunicationDot(ostream &dotout, u16 TotalFtns);

#endif
