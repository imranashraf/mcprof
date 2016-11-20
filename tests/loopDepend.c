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
    MCPROF_ZONE_ENTER(1);
    for(i = 0; i < SIZE; i++)
    {
        MCPROF_LOOPBODY_ENTER(1);
        arr[i]=i*5 + 7;
        MCPROF_LOOPBODY_EXIT(1);
    }
    MCPROF_ZONE_EXIT(1);
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

// void prodVecs()
// {
//     int i;
//     MCPROF_ZONE_ENTER(2);
//     for(i = 0; i < SIZE; i++)
//     {
//         MCPROF_LOOPBODY_ENTER(2);
//         prodArr[i] = sumArr[i] * diffArr[i] ;
//         MCPROF_LOOPBODY_EXIT(2);
//     }
//     MCPROF_ZONE_EXIT(2);
// }

void prodVecs(TYPE* a, TYPE* b, TYPE* c)
{
    int i;
    MCPROF_ZONE_ENTER(2);
    for(i = 1; i < SIZE; i++)
    {
        MCPROF_LOOPBODY_ENTER(2);
        c[i] = a[i] * b[i] ;
        MCPROF_LOOPBODY_EXIT(2);
    }
    MCPROF_ZONE_EXIT(2);
}

void sumdiffVecs()
{
    int i;

    MCPROF_ZONE_ENTER(3);
    for(i = 0; i < SIZE; i++)
    {
        MCPROF_LOOPBODY_ENTER(3);
        sumArr[i] = srcArr1[i] + coeff * srcArr2[i];
        MCPROF_LOOPBODY_EXIT(3);
    }
    MCPROF_ZONE_EXIT(3);

    MCPROF_ZONE_ENTER(4);
    for(i = 1; i < SIZE; i++)
    {
        MCPROF_LOOPBODY_ENTER(4);
        diffArr[i] = coeff * (srcArr1[i] - srcArr2[i]) + diffArr[i-1];
        MCPROF_LOOPBODY_EXIT(4);
    }
    MCPROF_ZONE_EXIT(4);
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
    prodVecs(sumArr, diffArr, prodArr);
//     prodVecs();
}

int main()
{
    MCPROF_START();
    nbytes = SIZE*sizeof(TYPE);
    printf("Vector Operations Test.\n");

    srcArr1 = malloc(nbytes);
    srcArr2 = malloc(nbytes);
    sumArr = malloc(nbytes);
    diffArr = malloc(nbytes);
    prodArr = malloc(nbytes);

    process();
    printf("output : %d\n", sumArr[0]+diffArr[0]+prodArr[0]);

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);
    free(prodArr);

    MCPROF_STOP();
    return 0;
}
