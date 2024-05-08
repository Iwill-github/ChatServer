#include "chat_service.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>


// 获取单例对象的接口函数（懒汉式单例模式，即，创建实例时才会创建单例对象）
ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}


// 注册消息处理回调
ChatService::ChatService(){
    // 用户基本业务管理事件回调
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    // 群组业务管理事件回调
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    /*  
        连接redis服务器成功后，给redis注册回调。
        该回调函数的作用是 redis服务器监听到响应通道事件发生时，给业务层上报消息。
        由于该上报消息服务业务层业务，具体如何上报需要具体的业务层定义，这也是使用回调函数的原因
    */ 
    if(_redis.connect()){
        // 设置上报消息的回调方法
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}


/*
 处理登录业务。ORM（对象关系映射）;    DAO（数据访问对象）
    尽管是数子，但是json对象解析时依然解析为字符串
    {"msgid":1, "id":13, "password":"123456"}    id已登录
    {"msgid":1, "id":14, "password":"123456"}    id或密码错误
    {"msgid":1, "id":18, "password":"123456"}    登录成功
*/
void ChatService::login(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    // LOG_INFO << "do login service!";
    int id = js["id"].get<int>();
    string password = js["password"];

    User user = _userModel.query(id);
    if(user.getId() == id && user.getPassword() == password){
        if(user.getState() == "online"){
            // 该id已经登陆，不允许重复登陆
            json response;
            response["error"] = 1;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errormsg"] = "该id已经登陆，不允许重复登陆";
            conn->send(response.dump());        // 序列化json对象后发送
        }else{
            // 登录成功，记录用户连接信息（多线程环境中，需要考虑线程安全问题）
            // lock_guard类用于 在特定作用域内 管理互斥锁的所有权，并在析构时自动释放所有权。
            {
                lock_guard<mutex> lock(_connMutex); 
                _userConnMap.insert({id, conn});
            }

            // 登录成功后，向redis订阅channel(id)。监听其他用户在消息队列中id通道上推送的消息
            _redis.subscribe(id);

            // 登录成功，更新用户状态信息 state offline=>online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["error"] = 0;
            response["id"] = user.getId();
            response["msgid"] = LOGIN_MSG_ACK;
            response["name"] = user.getName();  // 昵称

            // 查询该用户是否有离线消息
            vector<string> vec =  _offlineMsgModel.query(id);
            if(!vec.empty()){
                response["offlinemsg"] = vec;
                // 离线消息处理完毕，删除
                _offlineMsgModel.remove(id);
            }

            // 查询用户的好友信息并返回
            vector<User> users = _friendModel.query(id);
            if(!users.empty()){
                vector<string> vec_temp;
                for(User user: users){
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec_temp.emplace_back(js.dump());
                }
                response["friends"] = vec_temp;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["groupid"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());   // id、groupname、groupdesc、users
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());        // 序列化json对象后发送 
        }
    }else{
        // 登录失败
        json response;
        response["error"] = 1;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errormsg"] = "id或密码错误";
        conn->send(response.dump());        // 序列化json对象后发送
    }
    conn->send("\n");
}


// 处理注册业务
void ChatService::reg(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    // LOG_INFO << "do reg service!";
    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = _userModel.insert(user);   // 这里进行了主键回填，设置了user的id为数据库中的主键id
    if(state){
        // 注册成功
        json response;
        response["error"] = 0;
        response["id"] = user.getId();
        response["msgid"] = REG_MSG_ACK;
        conn->send(response.dump());        // 序列化json对象后发送
    }else{
        // 注册失败
        json response;
        response["error"] = 1;
        response["msgid"] = REG_MSG_ACK;
        conn->send(response.dump());        // 序列化json对象后发送
    }
    conn->send("\n");
}


// 处理注销业务
void ChatService::loginout(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int userid = js["id"].get<int>();
    
    // 删除该用户连接信息
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end()){
            _userConnMap.erase(it);
        }
    }

    // 取消订阅 redis 通道
    _redis.unsubscribe(userid);

    // 用户状态更新
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}


/*
一对一聊天业务
    {"msgid":1, "id":13, "password":"123456"}       13的登录消息
    {"msgid":1, "id":15, "password":"666666"}       15的登录消息
    {"msgid":5, "id":13, "to_id":15, "msg":"hello"}    13给15的聊天消息
    {"msgid":5, "id":15, "to_id":13, "msg":"hello"}    15给13的聊天消息
*/ 
void ChatService::oneChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int to_id = js["to_id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(to_id);
        if( it != _userConnMap.end() ){             // 在同一台服务器上
            it->second->send(js.dump() + "\n");
            return;
        } 
    }

    // 查询是否在线
    User user = _userModel.query(to_id);
    if(user.getState() == "online"){
        _redis.publish(to_id, js.dump() + "\n");
        return;
    }

    // 不在线，存储离线消息
    _offlineMsgModel.insert(to_id, js.dump() + "\n");
}


/*
添加好友业务
    {"msgid":1, "id":13, "password":"123456"}       13的登录消息
    {"msgid":6, "id":13, "friendid":16}             13添加好友16
*/ 
void ChatService::addFriend(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 添加到friend表
    _friendModel.insert(userid, friendid);
}


/*
创建群组业务
*/
void ChatService::createGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int id = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if(_groupModel.createGroup(group)){
        // 创建群之后，立马加入群组(添加到group_user表)，且身份设置为 creator
        _groupModel.addGroup(id, group.getId(), "creator");     // group.getId()是调用createGroup(group)时主键回填获取到的
    }
}


/*
加入群组业务
*/
void ChatService::addGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    // 添加到group_user表
    _groupModel.addGroup(userid, groupid, "normal");
}


/*
群组聊天业务
*/
void ChatService::groupChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    // 查询userid所在群组groupid的所有用户id（不包括自己），并推送消息
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);     // 这边在群发消息，只能提前下线 or 接收完再下线
    for(int id: useridVec){
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()){
            // 在同一台服务器上，直接转发群消息
            it->second->send(js.dump() + "\n");
        }else{
            // 查询toid是否在线
            User user = _userModel.query(id);
            if(user.getState() == "online"){
                _redis.publish(id, js.dump() + "\n");
            }else{
                // 存储离线消息。存储的离线消息，在用户上线时推送给用户
                _offlineMsgModel.insert(id, js.dump() + "\n");
            }
        }
    }
}


// 处理客户端异常退出
void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr& conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex); 
        for(auto [id, c]: _userConnMap){
            if(c == conn){
                // 从map表删除用户的连接信息
                _userConnMap.erase(id);
                user.setId(id);
                break;
            }
        }
    }

    // 用户状态设置为offline
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);   // 通过id设置state
    }
}


// 从redis消息队列中获取订阅的消息，并进行推送
void ChatService::handleRedisSubscribeMessage(int userid, string msg){
    // LOG_INFO << "receive redis subscribe msg: " << msg;

    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end()){
        it->second->send(msg + "\n");
        return;
    }

    // 存储离线消息
    _offlineMsgModel.insert(userid, msg);
}


// 处理服务器异常，重置方法
void ChatService::reset(){
    // 把online状态的用户，设置为offline
    _userModel.resetState();
}


// 获取消息对应的处理方法
MsgHandler ChatService::getHandler(int msgid){
    // 记录错误日志，msgid没有对应的事件处理方法
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end()){
        return [=](const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){  // lambda表达式
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";
        };
    }
    return _msgHandlerMap[msgid];
}


