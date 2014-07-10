#include "globals.h"
#include "shadow.h"

#include <iostream>

using namespace std;

int main()
{
    L3Table ShadowMemory;

    cout<<(int)ShadowMemory.getProducer(1)<<endl;
    ShadowMemory.setProducer(127,55);
    cout<<(int)ShadowMemory.getProducer(513)<<endl;
    cout<<(int)ShadowMemory.getProducer(513)<<endl;

    return 0;
}