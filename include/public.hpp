#ifndef PUBLIC_HPP
#define PUBLIC_HPP
/*
    server、client的公共文件
*/

enum EnMsgType{
    LOGIN_MSG = 1,      // 1登录消息
    LOGIN_MSG_ACK,      // 2登录响应消息
    REG_MSG,            // 3注册消息
    REG_MSG_ACK,        // 4注册响应消息
    ONE_CHAT_MSG,       // 5聊天消息
    ADD_FRIEND_MSG,     // 6添加好友消息

    CREATE_GROUP_MSG,   // 7创建群组
    ADD_GROUP_MSG,      // 8加入群组
    GROUP_CHAT_MSG,     // 9群聊消息

    LOGINOUT_MSG,       // 10注销消息
};

#endif