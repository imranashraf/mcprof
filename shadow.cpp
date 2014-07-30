#include "globals.h"
#include "shadow.h"
#include "commatrix.h"

// Three modes of operation
#define MEMMAP 0
#define TABLES 2
#define HYBRID 1
// Un-comment *ONLY ONE* of the following three to select Mode
#define MODE MEMMAP
// #define MODE TABLES
// #define MODE HYBRID

// if BYTELEVEL is defined than the granularity is Byte
// #define BYTELEVEL

#if (MODE==HYBRID)
MemMap  ShadowMem;
#endif

#if ( (MODE==HYBRID) || (MODE==TABLES) )
L3Table ShadowTable;
#endif

#if (MODE==MEMMAP)
MemMap8th ShadowMem8th;
#endif

void PrintShadowMap()
{
#if (MODE==HYBRID)
    ShadowMem.Print();
#elif (MODE==MEMMAP)
    ShadowMem8th.Print();
#else
    ECHO("NOT Using Mem Map as in TABLES Mode");
#endif
}

#if (MODE==MEMMAP)
void RecordWrite(FtnNo prod, uptr addr, int size)
{
    u8* shadowAddr = (u8*) MEM2SHADOW(addr);
    D3ECHO(ADDR(addr) << ADDR(MEM2SHADOW(addr)));
    //TODO check these size loops for 1,2,4,8 and 16 sizes
//     for(int i=0; i<size; i+=SHADOW_GRANULARITY)
    {
        *(shadowAddr ) = prod;
        D2ECHO("Write of " << size << " bytes by " << FUNC(prod) );
    }
}
#endif

#if (MODE==TABLES)
void RecordWrite(FtnNo prod, uptr addr, int size)
{
    D2ECHO("Write " << VAR(size) << FUNC(prod) << ADDR(addr));
#ifdef BYTELEVEL
        for(int i=0; i<size; i++) {
            ShadowTable.setProducer(addr+i, prod);
        }
#else
        ShadowTable.setProducerRange(addr, prod, size);
#endif
}
#endif

#if (MODE==HYBRID)
void RecordWrite(FtnNo prod, uptr addr, int size)
{
    D2ECHO("Write " << VAR(size) << FUNC(prod) << ADDR(addr));
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
#endif


#if (MODE==MEMMAP)
void RecordRead(FtnNo cons, uptr addr, int size)
{
    FtnNo prod;
    u8* shadowAddr = (u8*) MEM2SHADOW(addr);
    D3ECHO(ADDR(addr) << ADDR(MEM2SHADOW(addr)));
//     for(int i=0; i<size; i+=SHADOW_GRANULARITY)
    {
        prod = *(shadowAddr);
        RecordCommunication(prod, cons, size);
        D2ECHO("Read of " << size << " bytes by " << FUNC(cons)
                          << "producer was " << FUNC(prod));
    }
}
#endif

#if (MODE==TABLES)
void RecordRead(FtnNo cons, uptr addr, int size)
{
    FtnNo prod;
    D2ECHO("Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
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
#endif

#if (MODE==HYBRID)
void RecordRead(FtnNo cons, uptr addr, int size)
{
    FtnNo prod;
    D2ECHO("Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
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
#endif
