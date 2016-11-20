#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int *srcArr1, *srcArr2, *sumArr, *diffArr;

void createVect(int** ptr, int n)
{
    printf("Creating Vector\n");
    *ptr = malloc(n);
    if(*ptr == NULL)
    {
        printf("ERROR: Could not allocate memory");
        exit(1);
    }
}

void extendVect(int** ptr, int n)
{
    printf("Extending Vector\n");
    *ptr = realloc(*ptr, n);
    if(*ptr == NULL)
    {
        printf("ERROR: Could not allocate memory");
        exit(1);
    }
}

void initVec(int* arr, int N)
{
    printf("Initializing Vector\n");
    int i;
    for(i = 0; i < N; i++)
    {
        arr[i]=i;
    }
}

int main()
{
    printf("Allocation Dependence Test.\n");
    int nElem=4;
    int nBytes = nElem*sizeof(int);
#if 1
    createVect(&srcArr1, nBytes);
    initVec(srcArr1, nElem/2);
#else
    createVect(&srcArr1, nBytes/2);
    initVec(srcArr1, nElem/2);
    extendVect(&srcArr2, nBytes);
#endif

    initVec(srcArr1+nElem/2, nElem/2);

    printf("output : %d\n", srcArr1[0] );
    free(srcArr1);
    return 0;
}
