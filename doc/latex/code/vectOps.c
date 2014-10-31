int *srcArr1, *srcArr2, *sumArr, *diffArr;
int coeff = 2;
int nElem;

void initVecs(){
    for(int i = 0; i < nElem; i++){
        srcArr1[i]=i*5 + 7;
        srcArr2[i]=2*i - 3;
    }
}
void sumVecs(){
    for(int i = 0; i < nElem; i++)
        sumArr[i] = srcArr1[i] + coeff * srcArr2[i];
}
void diffVecs(){
    for(int i = 0; i < nElem; i++)
        diffArr[i] = coeff * (srcArr1[i] - srcArr2[i]);
}

int main(){
    nElem = 100;

    srcArr1 = malloc(nElem*sizeof(TYPE));
    srcArr2 = malloc(nElem*sizeof(TYPE));
    sumArr  = malloc(nElem*sizeof(TYPE));
    diffArr = malloc(nElem*sizeof(TYPE));

    initVecs();
    sumVecs();
    diffVecs();

    free(srcArr1);
    free(srcArr2);
    free(sumArr);
    free(diffArr);

    return 0;
}
