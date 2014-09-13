#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USE_MALLOC_WRAPPERS

#include "malloc_wrap.h"

#define SIZE 10
typedef int uint32_t;

// unsigned int srcArr1[SIZE];
// unsigned int srcArr2[SIZE];
// unsigned int sqrArr[SIZE];
// unsigned int sumArr[SIZE];
// unsigned int diffArr[SIZE];
unsigned int *srcArr1;
unsigned int *srcArr2;
unsigned int *sqrArr;
unsigned int *sumArr;
unsigned int *diffArr;

unsigned int coeff = 2;

int nbytes;

void initVecs()
{
    int i;
#   if 1
    memset(srcArr1, 1, nbytes );
#   else 
    for(i = 0; i < SIZE; i++) {
        srcArr1[i]=i*5 + 7;
//         srcArr2[i]=2*i - 3;
    }
#   endif

#   if 1
    memcpy(srcArr2, srcArr1, nbytes);
#   else
    memmove(srcArr2, srcArr1, nbytes);
#   endif
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

uint32_t SquareRootRounded(uint32_t a_nInput)
{
    uint32_t op  = a_nInput;
    uint32_t res = 0;
    // The second-to-top bit is set: use 1u << 14 for uint16_t type;
    // use 1uL<<30 for uint32_t type
    uint32_t one = 1uL << 30; 

    // "one" starts at the highest power of four <= than the argument.
    while (one > op)
        one >>= 2;

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }

    // Do arithmetic rounding to nearest integer
    if (op > res)
        res++;

    return res;
}

void sqrootVecs()
{
    int i;
    for(i = 0; i < SIZE; i++)
    {
        sqrArr[i] = SquareRootRounded( sumArr[i] );
        sqrArr[i] = SquareRootRounded( sqrArr[i] + diffArr[i] );
    }
}

int main()
{
    char* str1 = malloc(10);

//     char* str1 = "Hello";
#   if 0
    strcpy(str1, "Hello");
#   else
    str1[0] = 'U';
    str1[1] = 'B';
    str1[2] = 'B';
    str1[3] = 'I';
    str1[4] = '\0';
#endif
    char* str2 = strdup(str1);

    nbytes = SIZE*sizeof(unsigned int);
    printf("Vector Operations Test.\n");
    printf("Total bytes : %d\n",nbytes);

//     srcArr1 = malloc(nbytes);
//     printf("srcArr1 addr after malloc : %p\n",srcArr1);

    srcArr1 = calloc(nbytes/2, sizeof(uint32_t) );
    printf("srcArr1 addr after calloc : %p\n",srcArr1);

    srcArr2 = malloc(nbytes);
    printf("srcArr2 addr after malloc : %p\n",srcArr2);

    srcArr1 = realloc(srcArr1, nbytes);
    printf("srcArr1 addr after realloc: %p\n",srcArr1);

    sumArr = malloc(nbytes);
    printf("sumArr addr after malloc : %p\n",sumArr);

#if 0
    diffArr = malloc(nbytes);
    printf("diffArr addr after malloc : %p\n",diffArr);
#else
    // The following realloc behaves like malloc
    diffArr = realloc(NULL, nbytes);
    printf("diffArr addr after realloc : %p\n",diffArr);
#endif
    sqrArr = malloc(nbytes);
    printf("sqrArr addr after malloc : %p\n",sqrArr);

    initVecs();
    sumVecs();
    diffVecs();
    sqrootVecs();

    //printf("output and %s: %d\n", str2, sumArr[1]+diffArr[1]+sqrArr[1]);
    printf("output and : %d\n", sumArr[1]+diffArr[1]+sqrArr[1]);

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);
    free(sqrArr);

    return 0;
}
