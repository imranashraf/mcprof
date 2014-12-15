#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int TYPE;

TYPE *srcArr1;
TYPE *srcArr2;
TYPE *sumArr;
TYPE *diffArr;

TYPE coeff = 2;
int nBytes;
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

// *ptr = malloc(n);
// *ptr = calloc(nElem, sizeof(TYPE) );
// *ptr = realloc(NULL, n);

void createArray(TYPE** ptr, int n)
{
    *ptr = malloc(n);
    if(*ptr == NULL)
    {
        printf("ERROR: Could not allocate memory");
        exit(1);
    }
}


void extendArray(TYPE** ptr, int n)
{
    *ptr = realloc(*ptr, n);
    if(*ptr == NULL)
    {
        printf("ERROR: Could not allocate memory");
        exit(1);
    }
}

void process()
{
    nBytes = nElem*sizeof(TYPE);
    printf("Total bytes : %d\n",nBytes);

    //     srcArr1 = malloc(nBytes);
    createArray(&srcArr1, nBytes);
    printf("srcArr1 addr after malloc : %p\n",srcArr1);

    //     srcArr2 = malloc(nBytes);
    createArray(&srcArr2, nBytes);
    printf("srcArr2 addr after malloc : %p\n",srcArr2);

    //     sumArr = malloc(nBytes);
#if 1
    createArray(&sumArr, nBytes);
    printf("sumArr addr after malloc : %p\n",sumArr);
#else
    createArray(&sumArr, nBytes/2);
    printf("sumArr addr after malloc : %p\n",sumArr);
    extendArray(&sumArr, nBytes);
    printf("sumArr addr after realloc : %p\n",sumArr);
#endif
    //     diffArr = malloc(nBytes);
    createArray(&diffArr, nBytes);
    printf("diffArr addr after malloc : %p\n",diffArr);

    initVecs();
    sumVecs();
    diffVecs();

    printf("output : %d\n", sumArr[2]);
    printf("output : %d\n", diffArr[3]);

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);
}

int main()
{
    printf("Multi Allocation Test.\n");

    int i=1;
    for(; i<=1000; i++)
    {
        nElem = 100;
        process();
    }

    return 0;
}
