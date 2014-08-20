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
    Matrix2D();
    Matrix2D(IDNoType size);
    void RecordCommunication(IDNoType prod, IDNoType cons, u32 size);
    float MaxCommunication();
    void Print(ostream &fout);
    void PrintMatrix(ostream &fout);
    void PrintDot(ostream &dotout);
};

#endif
