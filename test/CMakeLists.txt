cmake_minimum_required(VERSION 3.10)    # 配置最低版本号
project(main)                   # 配置项目名称

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)     # -g 表示在可执行文件中嵌入调试信息

# 指定搜索的子目录。编译时，先找到工程下的CMakeLists.txt，
# 然后根据设定的子目录，去子目录下寻找CMakeLists.txt
add_subdirectory(test_muduo)
