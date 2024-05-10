
# 定义一个 SRC_LIST 列表，包含该目录下的所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db_connection_pool DB_CONNECTION_POOL_LIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)


# 指定生成可执行文件
# muduo_server由SRC_LST、DB_LIST列表中的源文件编译而来
add_executable(ChatServer ${SRC_LIST} ${DB_CONNECTION_POOL_LIST} ${MODEL_LIST} ${REDIS_LIST})


# 指定可执行文件需要依赖的库文件
target_link_libraries(ChatServer
    muduo_net 
    muduo_base 
    mysqlclient 
    hiredis
    pthread
)
