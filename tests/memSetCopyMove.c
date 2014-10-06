#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
typedef int TYPE;

unsigned int *srcArr1;
unsigned int *srcArr2;
unsigned int *sumArr;
unsigned int *diffArr;
unsigned int *sqrArr;

unsigned int coeff = 2;

int nbytes;

void initVecs()
{
    int i;
#   if 1
    memset(srcArr1, 1, nbytes );
    memcpy(srcArr2, srcArr1, nbytes);
    //memmove(srcArr2, srcArr1, nbytes);
#   else 
    for(i = 0; i < SIZE; i++) {
        srcArr1[i]=i*5 + 7;
        srcArr2[i]=2*i - 3;
    }
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

void sqrootVecs()
{
    int i;
    for(i = 0; i < SIZE; i++)
    {
        sqrArr[i] =  sumArr[i] * diffArr[i] ;
    }
}

int main()
{
    nbytes = SIZE*sizeof(unsigned int);
    printf("Vector Operations Test.\n");
    printf("Total bytes : %d\n",nbytes);

    srcArr1 = malloc(nbytes);
    printf("srcArr1 addr after malloc : %p\n",srcArr1);


    srcArr2 = malloc(nbytes);
    printf("srcArr2 addr after malloc : %p\n",srcArr2);

    sumArr = malloc(nbytes);
    printf("sumArr addr after malloc : %p\n",sumArr);


    diffArr = malloc(nbytes);
    printf("diffArr addr after malloc : %p\n",diffArr);

    sqrArr = malloc(nbytes);
    printf("sqrArr addr after malloc : %p\n",sqrArr);

    initVecs();
    sumVecs();
    diffVecs();
    sqrootVecs();

    printf("output : %d\n", sumArr[2]+diffArr[3]+sqrArr[4]);

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);
    free(sqrArr);

    return 0;
}
