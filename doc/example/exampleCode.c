#define SIZE 100
int *srcArr1, *srcArr2, *sumArr, *diffArr;

void initVecs() {
    for(i = 0; i < SIZE; i++) {
        *(srcArr1+i)=i*5 + 7;
        *(srcArr2+i)=2*i - 3;
    }
}
void sumVecs(){
    for(i = 0; i < SIZE; i++)
        *(sumArr+i) = *(srcArr1+i) + (*(srcArr2+i));
}
void diffVecs(){
    for(i = 0; i < SIZE; i++)
        *(diffArr+i) = *(srcArr1+i) + (*(srcArr2+i));
}
int main() {
    srcArr1 = malloc( SIZE*sizeof(int) );
    //simmilarly, other allocations

    initVecs();
    for(j=0;j<3;j++)    sumVecs();
    for(j=0;j<5;j++)    diffVecs();
    printf("output : %d\n", sumArr[1]+diffArr[1]);

    free(srcArr1);
    // simmilarly, other memory frees
    return 0;
}
