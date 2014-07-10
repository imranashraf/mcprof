#include "globals.h"
#include "commatrix.h"

Matrix2D ComMatrix;

void RecordCommunication(FtnNo prod, FtnNo cons, int size)
{
    ComMatrix.Increment(prod, cons, size);
}

void PrintCommunication()
{
    ComMatrix.Print();
}