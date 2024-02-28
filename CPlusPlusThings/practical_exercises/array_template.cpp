#include <cstring>
#include <iostream>

using namespace std;

#define MAXSIZE 5

template <class T>
class Array {
    //构造函数
    Array(){
        for(int i=0; i < MAXSIZE; i++){
            array[i] = 0;
        }
    }

    T &operator[](int i);
    void Sort();
private:
    T array[MAXSIZE];
};

template<class T>
T &Array<T>::operator[](int i) {
    if(i < 0 || i > MAXSIZE){
        cout << "数组下标越界!" <<endl;
        exit(0);
    }

    return array[i];
}

template<class T>
void Array<T>::Sort() {
    int p,j;
    for(int i = 0; i < MAXSIZE; i++){
        p = i;
        //找到i之后的最大值
        for(j = i + 1; j < MAXSIZE; j++){
            if(array[p] < array[j])
                p = j;
        }

        //使用临时变量进行替换
        T t;
        t = array[i];
        array[i] = array[p];
        array[p] = t;
    }
}

//模板特化
template <> void Array<char *>::Sort() {
    int p,j;
    for(int i = 0; i < MAXSIZE; i++){
        p = i;
        //找到i之后的最大值
        for(j = i + 1; j < MAXSIZE; j++){
            if(strcmp(array[p], array[j]) < 0){
                p = j;
            }
        }

        //使用临时变量进行替换
        char * t = array[i];
        array[i] = array[p];
        array[p] = t;
    }
}

int main(int argc, char const * argv[]){
    cout << "array template test!" <<endl;
    return 0;
}


