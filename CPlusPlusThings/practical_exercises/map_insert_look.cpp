#include <cstring>
#include <map>
#include <iostream>

using namespace std;

int main(int argc, char const* argv[]){
    map<const char*, const char*> mp;
    map<const char*, const char* >::iterator iter;

    const char key[3][20] = {"img", "system", "ip"};
    const char value[3][20] = {"d:/a.img", "win7", "193.68.6.3"};

    // make_pair 插入
    for(int i = 0; i < 2; i++){
        mp.insert(make_pair(key[i], value[i]));
    }

    //pair<const char* , const char*> 插入
    mp.insert(pair<const char*, const char*>(key[2], value[2]));

    //数组插入方式
    mp["addr"] = "中国";

    //循环取出元素
    for(iter = mp.begin(); iter != mp.end(); iter++){
        cout << iter->first << "\t" << iter->second <<endl;
    }

    char key1[20];
    cout << "please input key for find";
    cin.getline(key1, 20);
    //查找元素
    for(iter = mp.begin(); iter != mp.end(); iter++){
        if(strcmp(iter->first, key1) == 0){
            cout << iter->first <<"found it!" << "it's value :" << iter->second <<endl;
        }
    }

    //第一种删除方式
    iter = mp.find("addr");
    if(iter != mp.end()){
        cout << iter->first << "found by key !" << "It's value :" << iter->second <<endl;
        cout << "begin delete element!" << endl;
        mp.erase(iter);
    }
}

