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

void test(int count)
{
    printf("Call number %d to test()\n",count);
    if(count>1)
        test(count-1);
    else
        return;
}

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
    initVecs();
    for(i = 0; i < nElem; i++)
    {
        sumArr[i] = srcArr1[i] + coeff * srcArr2[i];
    }
    initVecs();
    test(2);
}

void diffVecs()
{
    int i;
    test(3);
    for(i = 0; i < nElem; i++)
    {
        diffArr[i] = coeff * (srcArr1[i] - srcArr2[i]);
    }
}

void process()
{
    initVecs();
    sumVecs();
    diffVecs();
}

int main()
{
    nElem = 5000;
    nBytes = nElem*sizeof(TYPE);
    srcArr1 = malloc(nBytes);
    srcArr2 = malloc(nBytes);
    sumArr = malloc(nBytes);
    diffArr = malloc(nBytes);

    test(4);

    int i;
    for(i=0; i<3; i++)
        process();

    printf("output : %d\n",sumArr[1]+diffArr[2]);

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);

    return 0;
}
