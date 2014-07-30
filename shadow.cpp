#include "globals.h"
#include "shadow.h"
#include "commatrix.h"


L3Table ShadowTable;
MemMap  ShadowMem;

// #define BYTELEVEL
#define SHADOWMEM

void PrintShadowMap()
{
    ShadowMem.Print();
}

void RecordWrite(FtnNo prod, uptr addr, int size)
{
    D2ECHO("Write " << VAR(size) << FUNC(prod) << ADDR(addr));
#ifdef SHADOWMEM
    uptr shadowAddr = ShadowMem.Mem2Shadow(addr);
    D3ECHO(  ADDR(addr) << " -> " << ADDR(shadowAddr));
    if (shadowAddr)
    {
#ifdef BYTELEVEL
        for(int i=0; i<size; i++) {
            *( (u8*) (shadowAddr+i) ) = prod;
        }
#else
        *( (u8*) (shadowAddr) ) = prod;
#endif
    }
    else
#endif
    {
#ifdef BYTELEVEL
        for(int i=0; i<size; i++) {
            ShadowTable.setProducer(addr+i, prod);
        }
#else
        ShadowTable.setProducerRange(addr, prod, size);
#endif
    }
}

void RecordRead(FtnNo cons, uptr addr, int size)
{
    FtnNo prod;
    D2ECHO("Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
#ifdef SHADOWMEM
    uptr shadowAddr = ShadowMem.Mem2Shadow(addr);
    D3ECHO(  ADDR(addr) << " -> " << ADDR(shadowAddr));

    if (shadowAddr)
    {
#ifdef BYTELEVEL
        for(int i=0; i<size; i++) {
            prod = *( (u8*) (shadowAddr + i ));
            RecordCommunication(prod, cons, 1);
            D2ECHO("Communication b/w " << FUNC(prod) << " and " << FUNC(cons) << " of size: 1" );
        }
#else
        prod = *( (u8*) (shadowAddr));
        RecordCommunication(prod, cons, size);
        D2ECHO("Communication b/w " << FUNC(prod) << " and " << FUNC(cons) << " of " << VAR(size) );
#endif
    }
    else
#endif
    {
#ifdef BYTELEVEL
        for(int i=0; i<size; i++) {
            prod = ShadowTable.getProducer(addr+i);
            RecordCommunication(prod, cons, 1);
            D2ECHO("Communication b/w " << FUNC(prod) << " and " << FUNC(cons) << " of size: 1" );
        }
#else
        prod = ShadowTable.getProducer(addr);
        RecordCommunication(prod, cons, size);
        D2ECHO("Communication b/w " << FUNC(prod) << " and " << FUNC(cons) << " of " << VAR(size) );
#endif
    }
}
