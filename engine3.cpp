#include "globals.h"
#include "shadow.h"
#include "engine3.h"
#include "commatrix.h"
#include "objects.h"
#include "callstack.h"

extern Objects objTable;
extern CallStackType CallStack;

void RecordWriteEngine3(uptr addr, u32 size)
{
    IDNoType prod = CallStack.top();
    
    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    for(u32 i=0; i<size; i++)
    {
        SetProducer(prod, addr+i);
    }
}

void RecordReadEngine3(uptr addr, u32 size)
{
    IDNoType cons = CallStack.top();
    D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
    IDNoType prod;
    IDNoType objid = objTable.GetID(addr);
    D2ECHO( ADDR(addr) << " " << ID2Name[objid] << "(" << objid << ")" );

    if(objid != UnknownID)
    {
        for(u32 i=0; i<size; i++)
        {
            prod = GetProducer(addr+i);
            RecordCommunication(prod, objid, 1);
            RecordCommunication(objid, cons, 1);
        }
    }
    else
    {
        for(u32 i=0; i<size; i++)
        {
            prod = GetProducer(addr+i);
            RecordCommunication(prod, cons, 1);
        }
    }
}
