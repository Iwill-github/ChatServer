
# 定义一个 SRC_LIST 列表，包含该目录下的所有源文件
aux_source_directory(. SRC_LIST)


# 指定生成可执行文件
# muduo_server由SRC_LST、DB_LIST列表中的源文件编译而来
add_executable(ChatClient ${SRC_LIST})


# 指定可执行文件需要依赖的库文件
target_link_libraries(ChatClient
    pthread
)
