#include "globals.h"
#include "shadow.h"
#include "engine2.h"
#include "commatrix.h"
#include "callstack.h"

extern CallStackType CallStack;
extern Matrix2D ComMatrix;

void RecordWriteEngine2(uptr addr, u32 size)
{
    IDNoType prod = CallStack.top();
    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    for(u32 i=0; i<size; i++)
    {
        SetProducer(prod, addr+i);
    }
}

void RecordReadEngine2(uptr addr, u32 size)
{
    IDNoType cons = CallStack.top();
    D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
    IDNoType prod;
    for(u32 i=0; i<size; i++)
    {
        prod = GetProducer(addr+i);
        ComMatrix.RecordCommunication(prod, cons, 1);
    }
}
