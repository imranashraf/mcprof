#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "markers.h"

#define SIZE 10
typedef int TYPE;

TYPE *srcArr1;
TYPE *srcArr2;
TYPE *sumArr;
TYPE *diffArr;
TYPE *prodArr;

TYPE coeff = 2;

int nbytes;

void initVec(TYPE* arr)
{
    int i;
    for(i = 0; i < SIZE; i++)
    {
        arr[i]=i*5 + 7;
    }
}

void sumVecs()
{
    int i;
    for(i = 0; i < SIZE; i++)
    {
        sumArr[i] = srcArr1[i] + coeff * srcArr2[i];
    }
}

void diffVecs()
{
    int i;
    for(i = 0; i < SIZE; i++)
    {
        diffArr[i] = coeff * (srcArr1[i] - srcArr2[i]);
    }
}

void prodVecs()
{
    int i;
    for(i = 0; i < SIZE; i++)
    {
        prodArr[i] = sumArr[i] * diffArr[i] ;
    }
}

void sumdiffVecs()
{
    int i;
    MCPROF_ZONE_ENTER(1);
    for(i = 0; i < SIZE; i++)
    {
        sumArr[i] = srcArr1[i] + coeff * srcArr2[i];
    }
    MCPROF_ZONE_EXIT(1);

    MCPROF_ZONE_ENTER(2);
    for(i = 0; i < SIZE; i++)
    {
        diffArr[i] = coeff * (srcArr1[i] - srcArr2[i]);
    }
    MCPROF_ZONE_EXIT(2);
}

void process()
{
    initVec(srcArr1);
    initVec(srcArr2);

    #if 0
    sumVecs();
    diffVecs();
    #else
    sumdiffVecs();
    #endif
    prodVecs();
}

int main()
{
    nbytes = SIZE*sizeof(TYPE);
    printf("Vector Operations Test.\n");
    printf("Total bytes : %d\n",nbytes);

    srcArr1 = malloc(nbytes);
    printf("srcArr1 addr after malloc : %p\n",srcArr1);

    srcArr2 = malloc(nbytes);
    printf("srcArr2 addr after malloc : %p\n",srcArr2);

    sumArr = malloc(nbytes);
    printf("sumArr addr after malloc : %p\n",sumArr);

    diffArr = malloc(nbytes);
    printf("diffArr addr after malloc : %p\n",diffArr);

    prodArr = malloc(nbytes);
    printf("prodArr addr after malloc : %p\n",prodArr);

    MCPROF_START();
    process();
    printf("output : %d\n", sumArr[1]+diffArr[1]+prodArr[1]);
    MCPROF_STOP();

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);
    free(prodArr);

    return 0;
}
