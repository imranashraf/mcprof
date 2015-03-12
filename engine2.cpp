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

// un-comment the following to print read/write address trace to output
// #define GENRATE_TRACES

// un-comment the following to generate selected read/write trace
#define GENRATE_SELECTED_TRACES

void RecordWriteEngine2(uptr addr, u32 size)
{
    IDNoType prod = CallStack.Top();
    IDNoType objid = GetObjectID(addr);

    #ifdef GENRATE_TRACES
    for(u32 i=0; i<size; i++)
        cout << "W " << addr+i << endl;
    #endif

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

        // Write Trace of a selected function to selected objects
        #ifdef GENRATE_SELECTED_TRACES
        if(
            // For canny: tmpimg objects(10) AND gaussian_smooth1(11) function.
            (objid==10) && (prod==11) 
          )
        {
            cout << "W of "<< size << " to " << objid << " by " << prod << " at " << HEXA(addr) << endl;
        }
        #endif
    }
}

void RecordReadEngine2(uptr addr, u32 size)
{
    IDNoType cons = CallStack.Top();
    D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
    IDNoType prod=0;
    IDNoType objid = GetObjectID(addr);
    D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );

    #ifdef GENRATE_TRACES
    for(u32 i=0; i<size; i++)
        cout << "R " << addr+i << endl;
    #endif

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

        // Read Trace by a selected function from selected objects
        #ifdef GENRATE_SELECTED_TRACES
        if( 
            // For canny: image(4) OR kernel(9) objects AND gaussian_smooth1(11) function.
            (objid==4 || objid==9) && (cons==11)
          )
        {
            cout << "R of "<< size << " from " << objid << " by " << cons << " at " << HEXA(addr) << endl;
        }
        #endif
    }
}
