
# 编译命令：
    g++ -o server muduo_server.cpp -lmuduo_net -lmuduo_base -lpthread
    # 注意，因为库的依赖关系，上述顺序不可改变

# telnet问题：
    telnet 一个端口来确定是否通畅时，有时我们telnet后就退不出来了ctrl+c也不管用，此时可以 ctl+] 切换 然后quit退出.

# 编译
## 方法1：使用vscode编译问题：
    配置头文件、库的搜索路径:
        按F1、搜索 edit configuration、选择 c/c++ (编译配置json)打开 c_cpp_properties.json
    
        g++ -I头文件搜索路径 -L库文件搜索路径 -l库的名称
            默认的头文件搜索路径：/usr/include、/usr/local/include
            默认的库文件搜索路径：/usr/lib、/usr/local/lib

    配置编译选项:
        ctrl + shift + b、选择右侧齿轮、打开tasks.json文件，添加编译选项
            "-lmuduo_net",
            "-lmuduo_base",
            "-lpthread"

## 方法2：使用CMake编译问题（使用cmake生成Makefile文件，然后执行make命令进行编译）：
    远程CMake配置中，设置 Cmake: Build Environment
        cmake.cmakePath、/usr/local/bin/cmake
    编写CMakeLists.txt文件、执行命令 cmake . (生成Makefile文件)、执行命令 make (进行编译)
        cd build
        cmake ..    # 在上级目录寻找CMakeLists.txt文件，将生成的中间文件放在build目录下
        make        # 编译

## 方法3：vscode 的CMake工具

# mysql
    sudo netstat -tanp | grep mysql
    sudo service mysql start
    sudo service mysql stop
    sudo service mysql restart

