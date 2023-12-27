#include <stdio.h>

struct NoBitFiled{
    unsigned int width;
    unsigned int height;
}NoBitFiledObj;

struct BitFiled{
    unsigned int width : 1;
    unsigned int height : 1;
}BitFiledObj;

int main(){
    printf("Memory Size 1: %lu\n", sizeof(NoBitFiledObj));
    printf("Memory Size 2: %lu\n", sizeof(BitFiledObj));
    return 0;
}
