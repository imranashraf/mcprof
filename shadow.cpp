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
// MemMap  ShadowMem;
MemMap1to4 ShadowMem;
#endif

#if ( (MODE==HYBRID) || (MODE==TABLES) )
L3Table ShadowTable;
#endif

#if (MODE==MEMMAP)
MemMap8th ShadowMem8th;
#endif

uptr tableCounter=0;
uptr memmapCounter=0;
void PrintShadowMap()
{
#if (MODE==HYBRID)
    ShadowMem.Print();
#elif (MODE==MEMMAP)
    ShadowMem8th.Print();
#else
    ECHO("NOT Using Mem Map as in TABLES Mode");
#endif
    ECHO(VAR(tableCounter));
    ECHO(VAR(memmapCounter));
}

/***************************************************************************/
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

/***************************************************************************/
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

/***************************************************************************/
#if (MODE==HYBRID)
void SetProducer(IDNoType pid, uptr addr)
{
    D2ECHO("Setting " << FUNC(pid) << " as producer of " << ADDR(addr));
    //TODO check this condition again
    if ( (addr < MemMap1to4::M0H) || (addr > MemMap1to4::M1L) )
    {
        uptr shadowAddr = ShadowMem.Mem2Shadow(addr);
        D3ECHO(  ADDR(addr) << " -> " << ADDR(shadowAddr));
        *( (u16*) (shadowAddr) ) = pid;
    }
    else
    {
        Entry* entry = ShadowTable.getEntry(addr);
        entry->funcID = pid;
    }
}
#endif

#if (MODE==HYBRID)
void SetProducers(uptr saddr, u32 size, IDNoType pid)
{
    if ( (saddr < MemMap1to4::M0H) || (saddr > MemMap1to4::M1L) )
    {
        for(uptr addr = saddr; addr < saddr+size; addr++)
        {
            uptr shadowAddr = ShadowMem.Mem2Shadow(addr);
            *( ((u16*) (shadowAddr)) + 1) = pid;
        }
    }
    else
    {
        for(uptr addr = saddr; addr < saddr+size; addr++)
        {
            Entry* entry = ShadowTable.getEntry(addr);
            entry->funcID = pid;
        }
    }
}
#endif

#if (MODE==HYBRID)
void SetObjectIDs(uptr saddr, u32 size, IDNoType oid)
{
    if ( (saddr < MemMap1to4::M0H) || (saddr > MemMap1to4::M1L) )
    {
        for(uptr addr = saddr; addr < saddr+size; addr++)
        {
            uptr shadowAddr = ShadowMem.Mem2Shadow(addr);
            *( ((u16*) (shadowAddr)) + 1 ) = oid;
        }
    }
    else
    {
        for(uptr addr = saddr; addr < saddr+size; addr++)
        {
            Entry* entry = ShadowTable.getEntry(addr);
            entry->objID = oid;
        }
    }
}
#endif

#if (MODE==HYBRID)
IDNoType GetObjectID(uptr addr)
{
    IDNoType oid;
    if ( (addr < MemMap1to4::M0H) || (addr > MemMap1to4::M1L) )
    {
        memmapCounter++;
        uptr shadowAddr = ShadowMem.Mem2Shadow(addr);        
        oid = *( (u16*) (shadowAddr) );
    }
    else
    {
        tableCounter++;
        Entry* entry = ShadowTable.getEntry(addr);
        oid = entry->objID;
    }
    return oid;
}
#endif


#if (MODE==HYBRID)
IDNoType GetProducer(uptr addr)
{
    IDNoType pid;
    D2ECHO(ADDR(addr) << ADDR(MemMap1to4::M0H) << ADDR(MemMap1to4::M1L) );
    if ( (addr < MemMap1to4::M0H) || (addr > MemMap1to4::M1L) )
    {
        uptr shadowAddr = ShadowMem.Mem2Shadow(addr);        
        pid = *( (u16*) (shadowAddr));        
    }
    else
    {
        Entry* entry = ShadowTable.getEntry(addr);
        pid = entry->funcID;
    }
    
    D2ECHO("Got producer of " << ADDR(addr) << " as " << FUNC(pid));
    return pid;
}
#endif
