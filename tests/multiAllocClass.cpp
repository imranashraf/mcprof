#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Vector.h"

#define SIZE 10

typedef short TYPE;

int coeff = 2;

int nbytes;

void initVecs(Vector<TYPE>& a, Vector<TYPE>& b)
{
    int i;
    for(i = 0; i < SIZE; i++) {
        a.push_back(i*5 + 7);
        b.push_back(2*i - 3);
    }
}

void sumVecs(Vector<TYPE>& a, Vector<TYPE>& b, Vector<TYPE>& c)
{
    int i;
    for(i = 0; i < SIZE; i++)
    {
        c.push_back(a(i)+ coeff * b(i));
    }
}

int main()
{
    printf("Vector Operation Test");

    Vector<TYPE> srcArr1;
    Vector<TYPE> srcArr2;
    Vector<TYPE> sumArr;

    srcArr1.init(SIZE);
    srcArr2.init(SIZE);
    sumArr.init(SIZE);


    initVecs(srcArr1, srcArr2);
    sumVecs(srcArr1, srcArr2, sumArr);

    for (int i=0; i<SIZE; i++)
    {
        printf("%d ", sumArr(i));
    }

    printf("End Vector Operation Test");
    return 0;
}
