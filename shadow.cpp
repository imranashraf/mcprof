#include "globals.h"
#include "shadow.h"
#include "commatrix.h"


L3Table ShadowTable;
MemMap  ShadowMem;

#define BYTELEVEL
#define SHADOWMEM

void PrintShadowMap()
{
    ShadowMem.Print();
}

void RecordWrite(FtnNo prod, uptr addr, int size)
{
#ifdef SHADOWMEM
    uptr shadowAddr = ShadowMem.Mem2Shadow(addr);
    DECHO(  hex << "0x" << setw(12) << setfill ('0') << addr << " -> "
         << hex << "0x" << setw(12) << setfill ('0') << shadowAddr
         << dec );

    if (shadowAddr)
    {
        for(int i=0; i<size; i++) {
            *( (u8*) (shadowAddr+i) ) = prod;
        }
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
#ifdef SHADOWMEM
    uptr shadowAddr = ShadowMem.Mem2Shadow(addr);
    DECHO(  hex << "0x" << setw(12) << setfill ('0') << addr << " -> "
         << hex << "0x" << setw(12) << setfill ('0') << shadowAddr
         << dec );

    if (shadowAddr)
    {
        for(int i=0; i<size; i++) {
            prod = *( (u8*) (shadowAddr + i ));
            RecordCommunication(prod, cons, 1);
        }
    }
    else
#endif
    {
#ifdef BYTELEVEL
        for(int i=0; i<size; i++) {
            prod = ShadowTable.getProducer(addr+i);
            RecordCommunication(prod, cons, 1);
            //DECHO("Communication b/w " << (int)prod << " and " << (int)cons << " of size: 1" );
        }
#else
        prod = ShadowTable.getProducer(addr);
        RecordCommunication(prod, cons, size);
        //DECHO("Communication b/w " << (int)prod << " and " << (int)cons << " of " << VAR(size) );
#endif
    }
}
