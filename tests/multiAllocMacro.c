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
int Size;

void initVecs()
{
    int i;
    for(i = 0; i < Size; i++) {
        srcArr1[i]=i*5 + 7;
        srcArr2[i]=2*i - 3;
    }
}

void sumVecs()
{
    int i;
    for(i = 0; i < Size; i++)
    {
        sumArr[i] = srcArr1[i] + coeff * srcArr2[i];
    }
}

void diffVecs()
{
    int i;
    for(i = 0; i < Size; i++)
    {
        diffArr[i] = coeff * (srcArr1[i] - srcArr2[i]);
    }
}

// void createArray(TYPE** ptr, int n)
// {
//     *ptr = malloc(n);
//     if(*ptr == NULL)
//     {
//         printf("ERROR: Could not allocate memory");
//         exit(1);
//     }
// }

// *ptr = malloc(n);                               
// *ptr = calloc(Size, sizeof(TYPE) );
// *ptr = realloc(NULL, n);                 

#define createArray(ptr, n)                         \
{                                                   \
    *ptr = malloc(n);                 \
    if(*ptr == NULL)                                \
    {                                               \
        printf("ERROR: Could not allocate memory"); \
        exit(1);                                    \
    }                                               \
}

#define extendArray(ptr, n)                         \
{                                                   \
    *ptr = realloc(*ptr, n);                 \
    if(*ptr == NULL)                                \
    {                                               \
        printf("ERROR: Could not allocate memory"); \
        exit(1);                                    \
    }                                               \
}

void process()
{
    nBytes = Size*sizeof(TYPE);
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
    for(; i<=10; i++)
    {
        Size = i*10;
        process();
    }

    return 0;
}
