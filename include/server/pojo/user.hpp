#ifndef USER_HPP

#include <string>
using namespace std;

// 匹配User表的ORM（对象关系映射）类。pojo层
class User{
public:
    User(int id = -1, string name = "", string password = "", string state = "offline"){
        this->id = id;
        this->name = name;
        this->password = password;
        this->state = state;
    };
    ~User(){}

    void setId(int id){ this->id = id; };
    void setName(string name){ this->name = name; };
    void setPassword(string password){ this->password = password; };
    void setState(string state){ this->state = state; };

    int getId() { return this->id; };
    string getName() { return this->name; }
    string getPassword() { return this->password; }
    string getState() { return this->state; }

private:
    int id;
    string name;
    string password;
    string state;
};

#define USER_HPP
#endif