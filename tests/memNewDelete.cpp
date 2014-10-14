#include <cstdio>
#include <iostream>

typedef int TYPE;

TYPE *srcArr1;
TYPE *srcArr2;
TYPE *sumArr;
TYPE *diffArr;

TYPE coeff = 2;
int nElem;

using namespace std;

void initVecs()
{
    int i;
    for(i = 0; i < nElem; i++) {
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
    nElem = 10;

    TYPE* dummy1 = new TYPE[nElem];
    delete[] dummy1;
    TYPE* dummy2 = new TYPE;
    delete dummy2;

    srcArr1 = new TYPE[nElem];
    printf("srcArr1 addr after calloc : %p\n",srcArr1);

    srcArr2 = new TYPE[nElem];
    printf("srcArr2 addr : %p\n",srcArr2);

    sumArr = new TYPE[nElem];
    printf("sumArr addr : %p\n",sumArr);

    diffArr = new TYPE[nElem];
    printf("diffArr addr : %p\n",diffArr);

    initVecs();
    sumVecs();
    diffVecs();

    printf("output : %d\n",sumArr[1]+diffArr[2]);

    delete[] srcArr1;
    delete[] srcArr2;
    delete[] sumArr;
    delete[] diffArr;

    return 0;
}
