#include "globals.h"
#include "shadow.h"
#include "mode1.h"
#include "commatrix.h"

void RecordWrite(FtnNo prod, uptr addr, int size)
{
    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    for(int i=0; i<size; i++)
    {
        SetProducer(prod, addr+i);
    }
}

void RecordRead(FtnNo cons, uptr addr, int size)
{
    D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
    FtnNo prod;

    for(int i=0; i<size; i++)
    {
        prod = GetProducer(addr+i);
        RecordCommunication(prod, cons, 1);
    }
}
