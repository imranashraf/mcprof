#ifndef COMMATRIX_H
#define COMMATRIX_H

#include "globals.h"
#include <vector>
#include <iostream>
#include <map>
#include <string>

using namespace std;


class Matrix2D
{
private:
    vector< vector<double> > Matrix;
    static const int DEFAULT_SIZE=255; //most of the applications have functions less than 256

public:
    Matrix2D() {
        u16 cols=DEFAULT_SIZE;
        u16 rows=DEFAULT_SIZE;
        double value=0.0;

        Matrix.resize( cols , vector<double>( rows , value) );
    }

    Matrix2D(u16 size) {
        u16 cols=size;
        u16 rows=size;
        double value=0.0;

        Matrix.resize( cols , vector<double>( rows , value) );
    }

    void Increment(u16 col, u16 row, u16 size) {
        if( col<Matrix.size() && row < Matrix.size() )
            Matrix[row][col] += size;
    }

    void Print() {
//             for (u16 r=0; r<Matrix.size(); r++) {
        for (u16 r=0; r<10; r++) {
//                 for (u16 c=0; c<Matrix[r].size(); c++) {
            for (u16 c=0; c<10; c++) {
                cout << Matrix[r][c] <<"  ";
            }
            cout<<endl;
        }
    }
    void PrintDot(ostream &dotout, map<u16,string> & ADDtoName, u16 TotalFtns) {
        dotout << "digraph {\ngraph [];"
               << "\nnode [fontcolor=black, style=filled, fontsize=20];"
               << "\nedge [fontsize=14, arrowhead=vee, arrowsize=0.5];"
               << endl;

        if ( TotalFtns > Matrix.size() ) {
            cerr << " TotalFtns > Matrix.size() "<<endl;
            return;
        }

        for (u16 r=0; r<TotalFtns; r++) {
            dotout << "\"" << r << "\"" << " [label=\"" << ADDtoName[r] << "\"];" << endl;
        }

        //fprintf(gfp,"\"%08x\" [label=\"%s\"];\n", (unsigned int)temp->producer , name2.c_str());
        //"\"%08x\" -> \"%08x\"  [label=\" %" PRIu64 " Bytes \\n %" PRIu64 " UnMAs \\n %" PRIu64 " UnDVs \" color=\"#%02x%02x%02x\"]\n",

        for (u16 r=0; r<TotalFtns; r++) {
            for (u16 c=0; c<TotalFtns; c++) {
                u16 prod = c;
                u16 cons = r;
                if(Matrix[r][c] > 0 ) {
                    dotout << "\"" << prod << "\""
                           << "->"
                           << "\"" << cons << "\""
                           << "[label=\"" << Matrix[r][c] <<" Bytes\"]"
                           << endl;
                }
            }
        }

        dotout << "}" << endl;
    }
};

void RecordCommunication(FtnNo prod, FtnNo cons, int size);
void PrintCommunication();
void PrintCommunicationDot(ostream &dotout, map <u16,string> & ADDtoName, u16 TotalFtns);

#endif
