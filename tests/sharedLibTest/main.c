#include <stdio.h>
#include <stdlib.h>
#include "vectOps.h"

#define SHLIB

int nElem = 10;
int *srcArr1, *srcArr2, *sumArr, *diffArr;

void initVecs1()
{
#ifdef SHLIB
    initVecs(srcArr1, srcArr2, nElem);
#else
    int i;
    for(i = 0; i < nElem; i++)
    {
        srcArr1[i]=i*5 + 7;
        srcArr2[i]=2*i - 3;
    }
#endif
}

void sumVecs1()
{
#ifdef SHLIB
    sumVecs(sumArr, srcArr1, srcArr2, nElem);
#else
    int i;
    for(i = 0; i < nElem; i++)
    {
        sumArr[i] = srcArr1[i] + srcArr2[i];
    }
#endif
}

void diffVecs1()
{
#ifdef SHLIB    
    diffVecs(diffArr, srcArr1, srcArr2, nElem);
#else
    int i;
    for(i = 0; i < nElem; i++)
    {
        diffArr[i] = (srcArr1[i] - srcArr2[i]);
    }
#endif
}

void printVec1()
{
    printVec(diffArr, nElem);
}

void process()
{
    initVecs1();

//     diffVecs1();
//     diffVecs(diffArr, srcArr1, srcArr2, nElem);
    sumVecs1();
    diffVecs1();
//     diffVecs(diffArr, srcArr1, srcArr2, nElem);

    printVec1();
    printf("\noutput : %d\n",sumArr[1]+diffArr[2]);    
}

int main()
{
    printf("Vector Operations Test.\n");

    int nBytes = nElem*sizeof(int);
    printf("Total bytes : %d\n",nBytes);

    srcArr1 = malloc(nBytes);
    printf("srcArr1 addr : %p\n",srcArr1);

    srcArr2 = malloc(nBytes);
    printf("srcArr2 addr : %p\n",srcArr2);

    sumArr = malloc(nBytes);
    printf("sumArr addr : %p\n",sumArr);

    diffArr = malloc(nBytes);
    printf("diffArr addr : %p\n",diffArr);
    
    process();

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);
    
    return 0;
}
