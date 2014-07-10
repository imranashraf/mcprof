#ifndef COMMATRIX_H
#define COMMATRIX_H

#include "globals.h"
#include <vector>
#include <iostream>

using namespace std;


class Matrix2D
{
    private:
        vector< vector<double> > Matrix;
        static const int DEFAULT_SIZE=5; //most of the applications have functions less than 256

    public:
        Matrix2D()
        {
            int cols=DEFAULT_SIZE;
            int rows=DEFAULT_SIZE;
            double value=0.0;

            Matrix.resize( cols , vector<double>( rows , value) );
        }

        Matrix2D(int size)
        {
            int cols=size;
            int rows=size;
            double value=0.0;

            Matrix.resize( cols , vector<double>( rows , value) );
        }

        void Increment(int col, int row, int size)
        {
            if( col<Matrix.size() && row < Matrix.size() )
                Matrix[row][col] += size;
        }

        void Print()
        {
            for (int r=0; r<Matrix.size(); r++) {
                for (int c=0; c<Matrix[r].size(); c++) {
                    cout << Matrix[r][c] <<"  ";
                }
                cout<<endl;
            }
        }

};

void RecordCommunication(FtnNo prod, FtnNo cons, int size);
void PrintCommunication();

#endif