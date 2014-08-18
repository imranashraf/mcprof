#include "globals.h"
#include "shadow.h"
#include "commatrix.h"

// Three modes of operation
#define MEMMAP 0
#define TABLES 2
#define HYBRID 1
// Un-comment *ONLY ONE* of the following three to select Mode
// #define MODE MEMMAP
#define MODE TABLES
// #define MODE HYBRID

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
void SetProducer(IDNoType prod, uptr addr)
{
    D2ECHO("Setting " << FUNC(prod) << " as producer of " << ADDR(addr));
    u8* shadowAddr = (u8*) MEM2SHADOW(addr);
    D3ECHO(ADDR(addr) << ADDR(MEM2SHADOW(addr)));
    //TODO check these size loops for 1,2,4,8 and 16 sizes
//     for(int i=0; i<size; i+=SHADOW_GRANULARITY)
    {
        *(shadowAddr ) = prod;
    }
}
#endif

#if (MODE==TABLES)
void SetProducer(IDNoType prod, uptr addr)
{
    D2ECHO("Setting " << FUNC(prod) << " as producer of " << ADDR(addr));
    ShadowTable.setProducer(addr, prod);
}
#endif

#if (MODE==HYBRID)
void SetProducer(IDNoType prod, uptr addr)
{
    D2ECHO("Setting " << FUNC(prod) << " as producer of " << ADDR(addr));
    uptr shadowAddr = ShadowMem.Mem2Shadow(addr);
    D3ECHO(  ADDR(addr) << " -> " << ADDR(shadowAddr));
    if (shadowAddr)
    {
        *( (u8*) (shadowAddr) ) = prod;
    }
    else
    {
        ShadowTable.setProducer(addr, prod);
    }
}
#endif


#if (MODE==MEMMAP)
IDNoType GetProducer(uptr addr)
{
    IDNoType prod;
    u8* shadowAddr = (u8*) MEM2SHADOW(addr);
    D3ECHO(ADDR(addr) << ADDR(MEM2SHADOW(addr)));
    prod = *(shadowAddr);
    D2ECHO("Got producer of " << ADDR(addr) << " as " << FUNC(prod));

    return prod;
}
#endif

#if (MODE==TABLES)
IDNoType GetProducer(uptr addr)
{
    IDNoType prod;
    prod = ShadowTable.getProducer(addr);
    D2ECHO("Got producer of " << ADDR(addr) << " as " << FUNC(prod));

    return prod;
}
#endif

#if (MODE==HYBRID)
IDNoType GetProducer(uptr addr)
{
    IDNoType prod;
    uptr shadowAddr = ShadowMem.Mem2Shadow(addr);
    D3ECHO(  ADDR(addr) << " -> " << ADDR(shadowAddr));

    if (shadowAddr)
    {
        prod = *( (u8*) (shadowAddr));
    }
    else
    {
        prod = ShadowTable.getProducer(addr);
    }
    D2ECHO("Got producer of " << ADDR(addr) << " as " << FUNC(prod));

    return prod;
}
#endif
