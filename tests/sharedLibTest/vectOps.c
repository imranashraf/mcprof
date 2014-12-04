#include <stdio.h>
#include <stdlib.h>

#include "vectOps.h"

void initVecs(int* srcArr1, int* srcArr2, int nElem)
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        srcArr1[i]=i*5 + 7;
        srcArr2[i]=2*i - 3;
    }
}

void sumVecs(int* sumArr, int* srcArr1, int* srcArr2, int nElem)
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        sumArr[i] = srcArr1[i] + srcArr2[i];
    }
}

void diffVecs(int* diffArr, int* srcArr1, int* srcArr2, int nElem)
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        diffArr[i] = (srcArr1[i] - srcArr2[i]);
    }
}

void printVec(int* srcArr, int nElem)
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        printf("%d  ", srcArr[i]);
    }
}
