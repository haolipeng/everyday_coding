
#include <stdio.h>

#define ARRAY_SIZE 10

//如果将数组首地址作为函数参数传递，则视为指针
int DoSomething(int array[]){
    int totalSize = sizeof(array);
    int unitSize = sizeof(array[0]);
    int nEle = sizeof(array) / sizeof(array[0]);
    printf("total size: %d, unit size: %d, element count:%d\n", totalSize, unitSize, nEle);

    //malloc memory by array size
    return 0;
}

int main(){
    /////////////////////////业务代码/////////////////////////////
    int array[ARRAY_SIZE] = {1,2,3,4,5,6,7,8,9};
    DoSomething(array);
    /////////////////////////业务代码/////////////////////////////
    return 0;
}
