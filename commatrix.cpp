#include "globals.h"
#include "commatrix.h"

Matrix2D ComMatrix;

void RecordCommunication(IDNoType prod, IDNoType cons, u32 size)
{
    D2ECHO("Recording Communication b/w " << FUNC(prod) << " and "
        << FUNC(cons) << " of size: " << size );
    ComMatrix.Increment(prod, cons, size);
}

void PrintCommunication(ostream &fout, IDNoType TotalFtns)
{
    ComMatrix.Print(fout, TotalFtns);
}

void PrintMatrix(ostream &fout, IDNoType TotalFtns)
{
    ComMatrix.PrintMatrix(fout, TotalFtns);
}

void PrintCommunicationDot(ostream &dotout, IDNoType TotalFtns)
{
    ComMatrix.PrintDot(dotout, TotalFtns);
}
