/* 中括号重载 */
#include <cstring>
#include <iostream>

using namespace std;

struct Person{ //职工基本信息的结构
    double salary;
    char* name;
};

class SalaryManage{
    Person* employ;//存放职工信息的数组
    int max;        //数组下标上界
    int n;          //数组中的实际职工人数

public:
    SalaryManage(int Max = 0){
        max = Max;
        n = 0;
        employ = new Person[max];
    }

    //重载[]，返回引用
    double& operator[](char* Name){
        Person* p;

        //遍历数组，查找指定项
        for(p = employ; p < employ +n ; p++){
            //如果存在处理
            if(strcmp(p->name, Name) == 0){
                return p->salary;
            }
        }

        //不存在处理,将新元素添加到数据结构中
        p = employ + n++;
        p->name = new char[strlen(Name) + 1];
        strcpy(p->name, Name);
        p->salary = 0;
        return p->salary;
    }

    void display(){
        for(int i = 0; i < n; i++){
            cout << employ[i].name << "  " << employ[i].salary <<endl;
        }
    }

    ~SalaryManage(){
        delete[] employ;
    }
};

int main(){
    SalaryManage s(3);
    s["zhang san"] = 2188.88;
    s["li si"] = 1230.07;
    s["wang wu"] = 3200.97;

    cout << "zhang san\t" << s["zhang san"] <<endl;
    cout << "li si\t" << s["li si"] <<endl;
    cout << "wang wu\t" << s["wang wu"] <<endl;

    cout << "-------下为display的输出--------\n\n";
    s.display();
}