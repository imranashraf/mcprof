void test(int count){
    printf("Call number %d to test()\n",count);
    if(count)   test(count-1);
    else        return;
}
void initVecs() {
    for(int i = 0; i < nElem; i++)
    {
        srcArr1[i]=i*5 + 7;
        srcArr2[i]=2*i - 3;
    }
}
void sumVecs() {
    initVecs();
    for(int i = 0; i < nElem; i++)
    {
        sumArr[i] = srcArr1[i] + coeff * srcArr2[i];
    }
    initVecs();
}
void diffVecs() {
    test(3);
    for(int i = 0; i < nElem; i++) {
        diffArr[i] = coeff * (srcArr1[i] - srcArr2[i]);
    }
}
void process() {
    initVecs();
    sumVecs();
    diffVecs();
}
int main() {
    nElem = 5000;
    nBytes = nElem*sizeof(TYPE);
    srcArr1 = malloc(nBytes); // similarly other allocations

    int i;
    for(i=0; i<3; i++)
        process();

    printf("output : %d\n",sumArr[1]+diffArr[2]);

    free(srcArr1); // similarly other frees
    return 0;
}
