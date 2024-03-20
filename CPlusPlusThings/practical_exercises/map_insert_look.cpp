#include <cstring>
#include <map>

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
    //TODO:
}

