#include "globals.h"
#include "shadow.h"
#include "commatrix.h"

#include <iostream>

using namespace std;

L3Table ShadowTable;

// #define BYTELEVEL

void RecordWrite(FtnNo prod, uptr addr, int size)
{
#ifdef BYTELEVEL
    for(int i=0; i<size; i++) {
        ShadowTable.setProducer(addr+i, prod);
    }
#else
    ShadowTable.setProducerRange(addr, prod, size);
#endif
}

void RecordRead(FtnNo cons, uptr addr, int size)
{
    FtnNo prod;
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
