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
    //most of the applications have functions less than 256
    static const u32 DEFAULT_SIZE = 1024; //15000

public:
    Matrix2D();
    Matrix2D(IDNoType size);
    //void RecordCommunication(IDNoType prod, IDNoType cons, u32 size);
    void inline RecordCommunication(IDNoType prod, IDNoType cons, u32 size)
    {
        D2ECHO("Recording Communication b/w " << FUNC(prod) << " and "
            << FUNC(cons) << " of size: " << size );

        if( prod < Matrix.size() && cons < Matrix.size() )
            Matrix[prod][cons] += size;
    }

    float MaxCommunication();
    void Print(ostream &fout);
    void PrintMatrix(ostream &fout);
//     void PrintMatrixFancy(ostream &fout);
    void PrintDot(ostream &dotout);
};

#endif
