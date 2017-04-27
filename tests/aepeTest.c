#include <stdio.h>
#include <stdlib.h>

int nElem;
typedef int TYPE;

TYPE *in1, *in2;
TYPE *out1, *out2;

void init(TYPE* arr)
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        arr[i]=i*5 + 7;
    }
}

void process(TYPE *in, TYPE *out)
{
    int i;
    for(i = 0; i < nElem/2; i++)
    {
        out[i] = 2*in[i];
    }
}

int main()
{
    printf("AE/PE Test.\n");
    nElem = 64;
    int nBytes = nElem*sizeof(TYPE);

    in1  = (TYPE*)malloc(nBytes);
    in2  = (TYPE*)malloc(nBytes);
    out1 = (TYPE*)malloc(nBytes);
    out2 = (TYPE*)malloc(nBytes);

    init(in1);
    init(in2);
    process(in1, out1);
    process(in2, out2);

    free(in1);
    free(in2);
    free(out1);
    free(out2);

    return 0;
}
