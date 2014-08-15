#include "globals.h"
#include "shadow.h"
#include "mode2.h"
#include "commatrix.h"
#include "objects.h"

extern Objects objTable;

void RecordWriteMode2(FtnNo prod, uptr addr, int size)
{
    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    for(int i=0; i<size; i++)
    {
        SetProducer(prod, addr+i);
    }
}

void RecordReadMode2(FtnNo cons, uptr addr, int size)
{
    D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
    FtnNo prod;
    int objid = objTable.GetID(addr);

    for(int i=0; i<size; i++)
    {
        prod = GetProducer(addr+i);
        RecordCommunication(prod, objid, 1);
        RecordCommunication(objid, cons, 1);
    }
}
