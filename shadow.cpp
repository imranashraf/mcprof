#include "globals.h"
#include "shadow.h"
#include "commatrix.h"

#include <iostream>

using namespace std;

L3Table ShadowTable;

void RecordWrite(FtnNo prod, uptr addr, int size)
{
    for(int i=0; i<size; i++) {
        ShadowTable.setProducer(addr+i, prod);
    }
}

void RecordRead(FtnNo cons, uptr addr, int size)
{
    FtnNo prod;

    for(int i=0; i<size; i++) {
        prod = ShadowTable.getProducer(addr+i);
        RecordCommunication(prod, cons, 1);
        DECHO("Communication b/w " << (int)prod << " and " << (int)cons);
    }
}
