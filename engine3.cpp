#include "globals.h"
#include "shadow.h"
#include "engine3.h"
#include "commatrix.h"
#include "symbols.h"
#include "callstack.h"

extern CallStackType CallStack;
extern Matrix2D ComMatrix;

extern Symbols symTable;

void RecordWriteEngine3(uptr addr, u32 size)
{
    IDNoType prod = CallStack.Top();
    IDNoType objid = GetObjectID(addr);

    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    if(objid == UnknownID)
    {
        for(u32 i=0; i<size; i++)
        {
            SetProducer(prod, addr+i);
        }
    }
    else
    {
        for(u32 i=0; i<size; i++)
        {
            SetProducer(prod, addr+i);
            ComMatrix.RecordCommunication(prod, objid, 1);
        }
    }
}

void RecordReadEngine3(uptr addr, u32 size)
{
    IDNoType cons = CallStack.Top();
    D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
    IDNoType prod;
    IDNoType objid = GetObjectID(addr);
    D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );

    //TODO may be UnknownObjID
    if(objid == UnknownID)
    {
        for(u32 i=0; i<size; i++)
        {
            prod = GetProducer(addr+i);
            ComMatrix.RecordCommunication(prod, cons, 1);
        }
    }
    else
    {
        for(u32 i=0; i<size; i++)
        {
            ComMatrix.RecordCommunication(objid, cons, 1);
        }
    }
}
