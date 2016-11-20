#include <stdio.h>
#include <stdlib.h>

typedef int TYPE;

TYPE* srcArr1;
TYPE* srcArr2;
TYPE* sumArr;
TYPE* diffArr;

TYPE coeff = 2;

int nBytes;
int nElem;

void initVec(TYPE* arr)
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        arr[i]=i*5 + 7;
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
    nElem = 1000;
    nBytes = nElem*sizeof(TYPE);
    printf("Total bytes : %d\n",nBytes);

    srcArr1 = (TYPE*)malloc(nBytes);
    printf("srcArr1 addr : %p\n",srcArr1);

    srcArr2 = (TYPE*)malloc(nBytes);
    printf("srcArr2 addr : %p\n",srcArr2);

    sumArr = (TYPE*)malloc(nBytes);
    printf("sumArr addr : %p\n",sumArr);

    diffArr = (TYPE*)malloc(nBytes);
    printf("diffArr addr : %p\n",diffArr);

    initVec(srcArr1);
    initVec(srcArr2);
    sumVecs();
    diffVecs();

    printf("output : %d\n",sumArr[1]+diffArr[2]);

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);

    return 0;
}
