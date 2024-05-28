cmake_minimum_required(VERSION 3.10)    # 配置最低版本号
project(ChatServer)                     # 配置项目名称

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)     # -g 表示在可执行文件中嵌入调试信息

# 配置最终的可执行文件输出的路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件的搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/server
    ${PROJECT_SOURCE_DIR}/include/server/db_connection_pool
    ${PROJECT_SOURCE_DIR}/include/server/model
    ${PROJECT_SOURCE_DIR}/include/server/pojo
    ${PROJECT_SOURCE_DIR}/include/server/redis
    ${PROJECT_SOURCE_DIR}/thirdparty
)

# 指定搜索的子目录。编译时，先找到工程下的CMakeLists.txt，
# 然后根据设定的子目录，去子目录下寻找CMakeLists.txt
add_subdirectory(src)

