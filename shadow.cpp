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
void SetObjectIDs(uptr saddr, u32 size, IDNoType id)
{
    //TODO we need to use memset, secondly we need to take care if
    // addr does not lie in this table (in current non-optimal way its not a problem)
    for(uptr addr = saddr; addr < saddr+size; addr++)
    {
        Entry* entry = ShadowTable.getEntry(addr);
        entry->objID = id;
    }
}
#endif

#if (MODE==TABLES)
void SetProducer(IDNoType fid, uptr addr)
{
    D2ECHO("Setting " << FUNC(fid) << " as producer of " << ADDR(addr));
    Entry* entry = ShadowTable.getEntry(addr);
    entry->funcID = fid;
    //entry->threadID = tid;
}
#endif

#if (MODE==TABLES)
void SetProducers(uptr saddr, u32 size, IDNoType fid)
{
    //TODO we need to use memset, secondly we need to take care if
    // addr does not lie in this table (in current non-optimal way its not a problem)
    for(uptr addr = saddr; addr < saddr+size; addr++)
    {
        Entry* entry = ShadowTable.getEntry(addr);
        entry->funcID = fid;
    }
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
    IDNoType id;
    Entry* entry = ShadowTable.getEntry(addr);
    id = entry->funcID;
    D2ECHO("Got producer of " << ADDR(addr) << " as " << FUNC(id));
    return id;
}
#endif

#if (MODE==TABLES)
IDNoType GetObjectID(uptr addr)
{
    IDNoType oid;
    Entry* entry = ShadowTable.getEntry(addr);
    oid = entry->objID;
    D2ECHO("Got producer of " << ADDR(addr) << " as " << FUNC(oid));
    return oid;
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
