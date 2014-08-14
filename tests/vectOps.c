#include <stdio.h>
#include <stdlib.h>

#define SIZE 10
typedef int uint32_t;

// unsigned int srcArr1[SIZE];
// unsigned int srcArr2[SIZE];
// unsigned int sqrArr[SIZE];
unsigned int *srcArr1;
unsigned int *srcArr2;
unsigned int *sqrArr;
unsigned int sumArr[SIZE];
unsigned int diffArr[SIZE];

unsigned int coeff = 2;

void initVecs()
{
    int i;
    for(i = 0; i < SIZE; i++)
    {
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

uint32_t SquareRootRounded(uint32_t a_nInput)
{
    uint32_t op  = a_nInput;
    uint32_t res = 0;
    uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

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

    /* Do arithmetic rounding to nearest integer */
    if (op > res)
    {
        res++;
    }

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
    printf("Vector Operations Test.\n");
    char *dummy = malloc(1);
    printf("Dummy malloc addr : 0x%x\n", dummy);

    sqrArr = malloc(SIZE * sizeof(uint32_t));
    printf("sqrArr addr : 0x%x\n",sqrArr);
    
    srcArr1 = malloc(SIZE * sizeof(uint32_t));
    printf("srcArr1 addr : 0x%x\n",srcArr1);
    
    srcArr2 = malloc(SIZE * sizeof(uint32_t));
    printf("srcArr2 addr : 0x%x\n",srcArr2);
    
    initVecs();
    sumVecs();
    diffVecs();
//     sqrootVecs();

//     printf("output : %d\n",sumArr[1]+diffArr[1]);
    printf("output : %d\n",sumArr[1]+diffArr[1]);

    free(dummy);
    free(srcArr1);
    free(srcArr2);
    free(sqrArr);

    return 0;
}
