# CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)
# 项目名称和语言（C++）
project(ChatForum LANGUAGES CXX)

# 使用 C++17 标准（需要 optional、filesystem 等特性）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找外部依赖库
# Drogon：Web 框架核心；jwt-cpp：JWT 令牌生成与验证；libmysql：MySQL 客户端
find_package(Drogon CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(unofficial-libmysql CONFIG REQUIRED)

# 头文件搜索路径，使各模块可以互相 include
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/websocket
)

# 递归收集所有 .cpp 文件作为源码
file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# 构建可执行目标
add_executable(ChatForum ${SOURCES})

# 链接依赖库
target_link_libraries(ChatForum PRIVATE
    Drogon::Drogon
    jwt-cpp::jwt-cpp
    unofficial::libmysql::mysqlclient
)
