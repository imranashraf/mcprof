#include "globals.h"
#include "shadow.h"
#include "engine2.h"
#include "commatrix.h"
#include "symbols.h"
#include "callstack.h"

extern CallStackType CallStack;
extern Matrix2D ComMatrix;

extern Symbols symTable;
extern bool TrackObjects;
extern bool NoseDown;

void RecordWriteEngine2(uptr addr, u32 size)
{
    IDNoType prod = CallStack.Top();
    IDNoType objid = GetObjectID(addr);

    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    if( objid == UnknownID )
    {
        for(u32 i=0; i<size; i++)
        {
            SetProducer(prod, addr+i);
        }
    }
    else
    {
        D2ECHO("Recording comm of " << VAR(size) << " b/w " << FUNC(prod)
               << " and " << symTable.GetSymName(objid) << dec);
        for(u32 i=0; i<size; i++)
        {
            SetProducer(prod, addr+i);
        }
        ComMatrix.RecordCommunication(prod, objid, size);
    }
}

void RecordReadEngine2(uptr addr, u32 size)
{
    IDNoType cons = CallStack.Top();
    D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
    IDNoType prod=0;
    IDNoType objid = GetObjectID(addr);
    D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );
//     cout << addr/4096 << endl; // for address tracing
    if( objid == UnknownID )
    {
        for(u32 i=0; i<size; i++)
        {
            prod = GetProducer(addr+i);
            ComMatrix.RecordCommunication(prod, cons, 1);
        }
    }
    else
    {
        D2ECHO("Recording comm of " << VAR(size) << " b/w "
               << symTable.GetSymName(objid) << " and " << FUNC(cons) << dec);

            ComMatrix.RecordCommunication(objid, cons, size);
    }
}
