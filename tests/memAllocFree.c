#include <stdio.h>
#include <stdlib.h>

typedef int TYPE;

TYPE *srcArr1;
TYPE *srcArr2;
TYPE *sumArr;
TYPE *diffArr;

TYPE coeff = 2;
int nElem;

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
    TYPE* dummy1 = malloc(5);
    printf("dummy1 addr : %p\n",dummy1);
    free(dummy1);
    TYPE* dummy2 = calloc(5,sizeof(TYPE));
    printf("dummy2 addr : %p\n",dummy2);
    free(dummy2);
    TYPE* dummy3 = realloc(NULL,5*sizeof(TYPE));
    printf("dummy3 addr : %p\n",dummy3);
    free(dummy3);

    printf("Vector Operations Test.\n");
    nElem = 100;

    srcArr1 = (TYPE*)malloc(nElem * sizeof(TYPE) );
    printf("srcArr1 addr : %p\n",srcArr1);

    srcArr2 = (TYPE*)malloc(nElem * sizeof(TYPE));
    printf("srcArr2 addr : %p\n",srcArr2);

    sumArr = (TYPE*)malloc(nElem * sizeof(TYPE));
    printf("sumArr addr : %p\n",sumArr);

    diffArr = (TYPE*)malloc(nElem * sizeof(TYPE));
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
