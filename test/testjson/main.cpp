
#include <iostream>
#include <vector>
#include <string>
#include <map>
using namespace std;

#include "json.hpp"
using nlohmann::json;


string func(){
    // 序列化容器示例
    json js;
    vector<int> vec = {1, 2, 3, 4, 5};
    unordered_map<string, int> map = {{"1", 100}, {"2", 200}};

    js["list"] = vec;
    js["map"] = map;
    
    // cout << js.dump() << endl;  // 将 json 对象序列化成 json 字符串
    return js.dump();
}


int main(){
    cout << "Hello World!" << endl;

    string recvBuf = func();
    json jsBuf = json::parse(recvBuf);
    cout << jsBuf["list"] << endl;
    cout << jsBuf["map"] << endl;

    return 0;
}
