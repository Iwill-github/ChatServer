
# 配置头文件搜索路径
# include_directories()
# 配置库文件搜索路径
# link_directories()

set(SRC_LST ./muduo_server.cpp)
# 把./指定路径下的所有.cpp文件添加到SRC_LST(源文件列表)中
# aux_source_directory(./ SRC_LST)

# 设置可执行文件最终存储的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# muduo_server由SRC_LST列表中的源文件编译而来
add_executable(server ${SRC_LST})

# muduo_server 可执行程序，需要连接 muduo_net muduo_base pthread 这三个库
target_link_libraries(server muduo_net muduo_base pthread)


