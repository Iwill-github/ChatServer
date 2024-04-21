# 指定搜索的子目录。编译时，先找到工程下的CMakeLists.txt，
# 然后根据设定的子目录，去子目录下寻找CMakeLists.txt
add_subdirectory(server)
add_subdirectory(client)