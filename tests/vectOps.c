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
    nBytes = nElem*sizeof(TYPE);
    printf("Total bytes : %d\n",nBytes);

    srcArr1 = malloc(nBytes);
    printf("srcArr1 addr : %p\n",srcArr1);

    srcArr2 = malloc(nBytes);
    printf("srcArr2 addr : %p\n",srcArr2);

    sumArr = malloc(nBytes);
    printf("sumArr addr : %p\n",sumArr);

    diffArr = malloc(nBytes);
    printf("diffArr addr : %p\n",diffArr);

    initVecs();
    sumVecs();
    diffVecs();

    printf("output : %d\n",sumArr[1]+diffArr[2]);

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);

    return 0;
}
