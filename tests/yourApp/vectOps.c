#include <stdio.h>
#include <stdlib.h>

typedef int TYPE;

TYPE* srcArr1;
TYPE* srcArr2;
TYPE* sumArr;
TYPE* diffArr;

TYPE coeff = 2;
int nElem;

void initVecs()
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        srcArr1[i]=i*5 + 7;
        srcArr2[i]=2*i - 3;
    }
}

void sumVecs()
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        sumArr[i] = srcArr1[i] + coeff * srcArr2[i];
    }
}

void diffVecs()
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        diffArr[i] = coeff * (srcArr1[i] - srcArr2[i]);
    }
}

int main()
{
    printf("Vector Operations Test.\n");
    nElem = 100;
    int nBytes = nElem*sizeof(TYPE);

    srcArr1 = malloc(nBytes);
    srcArr2 = malloc(nBytes);
    sumArr  = malloc(nBytes);
    diffArr = malloc(nBytes);
    if(srcArr1 == NULL || srcArr2 == NULL || sumArr == NULL || diffArr == NULL)
    {
        printf("Memory Allocation Error\n");
        return;
    }

    int i;
    initVecs();
    for(i=0; i<5; i++)  sumVecs();
    for(i=0; i<7; i++)  diffVecs();

    printf("output : %d\n",sumArr[1]+diffArr[2]);

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);

    printf("End Vector Operations Test.\n");
    return 0;
}
