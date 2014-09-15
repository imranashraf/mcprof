#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USE_MALLOC_WRAPPERS

#include "malloc_wrap.h"

#define SIZE 10
typedef int TYPE;

TYPE *srcArr1;
TYPE *srcArr2;
TYPE *sumArr;
TYPE *diffArr;

TYPE coeff = 2;
int nbytes;

void initVecs()
{
    int i;
    for(i = 0; i < SIZE; i++) {
        srcArr1[i]=i*5 + 7;
        srcArr2[i]=2*i - 3;
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

void createArray(TYPE** ptr, int n)
{
    *ptr = malloc(n);
    if(*ptr == NULL)
    {
        printf("ERROR: Could not allocate memory");
        exit(1);
    }
}

int main()
{
    nbytes = SIZE*sizeof(TYPE);
    printf("Multi Allocation Test.\n");
    printf("Total bytes : %d\n",nbytes);

//     srcArr1 = malloc(nbytes);
    createArray(&srcArr1, nbytes);
    printf("srcArr1 addr after malloc : %p\n",srcArr1);

//     srcArr2 = malloc(nbytes);
    createArray(&srcArr2, nbytes);
    printf("srcArr2 addr after malloc : %p\n",srcArr2);

//     sumArr = malloc(nbytes);
    createArray(&sumArr, nbytes);
    printf("sumArr addr after malloc : %p\n",sumArr);

//     diffArr = malloc(nbytes);
    createArray(&diffArr, nbytes);
    printf("diffArr addr after malloc : %p\n",diffArr);

    initVecs();
    sumVecs();
    diffVecs();

    printf("output : %d\n", sumArr[nbytes/2]+diffArr[nbytes/3]);

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);

    return 0;
}
