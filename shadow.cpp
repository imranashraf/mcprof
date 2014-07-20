#include "globals.h"
#include "shadow.h"
#include "commatrix.h"

#include <iostream>

using namespace std;

L3Table ShadowMemory;

void RecordWrite(FtnNo prod, uptr addr, int size)
{
    for(int i=0; i<size; i++) {
        ShadowMemory.setProducer(addr+i,prod);
    }
}

void RecordRead(FtnNo cons, uptr addr, int size)
{
    FtnNo prod;

    for(int i=0; i<size; i++) {
        prod = ShadowMemory.getProducer(addr+i);
        //RecordCommunication
        RecordCommunication(prod,cons,1);
        dout<<"Communication b/w "<<(int)prod<<" and "<<(int)cons<<endl;
    }
}
