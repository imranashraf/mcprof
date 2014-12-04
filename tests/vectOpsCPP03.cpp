#include <cstdio>
#include <iostream>
#include <vector>

/*
 * Because of the calls to push_back extra communication is
 * reported because of creation and then initialization, but
 * it still needs detailed explanation
 */

typedef int TYPE;

TYPE coeff = 2;
int nElem;

using namespace std;

void initVecs(vector<TYPE>& srcArr1, vector<TYPE>& srcArr2)
{
    int i;
    for(i = 0; i < nElem; i++) {
        srcArr1.push_back(i*5 + 7);
        srcArr2.push_back(2*i - 3);
    }
}

void sumVecs(vector<TYPE>& srcArr1, vector<TYPE>& srcArr2, vector<TYPE>& sumArr)
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        sumArr.push_back( srcArr1[i] + coeff * srcArr2[i] );
    }
}

void diffVecs(vector<TYPE>& srcArr1, vector<TYPE>& srcArr2, vector<TYPE>& diffArr)
{
    int i;
    for(i = 0; i < nElem; i++)
    {
        diffArr.push_back(coeff * (srcArr1[i] - srcArr2[i]) );
    }
}

int main()
{
    nElem = 10;

    printf("Vector Operations Test.\n");

    vector<TYPE> srcArr1;
    vector<TYPE> srcArr2;
    vector<TYPE> sumArr;
    vector<TYPE> diffArr;

    initVecs(srcArr1, srcArr2);
    sumVecs(srcArr1, srcArr2, sumArr);
    diffVecs(srcArr1, srcArr2, diffArr);

    printf("output : %d\n",sumArr[1]+diffArr[2]);

    return 0;
}
