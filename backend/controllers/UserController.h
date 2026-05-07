#pragma once

#include <drogon/drogon.h>

// 用户控制器 - 处理用户注册、登录、个人信息等请求
// 继承 HttpController<T> 使 Drogon 框架能自动识别并注册此控制器
class UserController : public drogon::HttpController<UserController>
{
public:
    // METHOD_LIST_BEGIN / END 是 Drogon 的宏，用于声明路由映射表
    // ADD_METHOD_TO(方法, 路径, HTTP方法..., 中间件名)
    // 最后一个字符串参数是中间件名，"auth" 表示需要经过 AuthMiddleware 鉴权
    // {1} 是路径参数占位符，会作为方法参数传入
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(UserController::registerUser, "/api/auth/register", drogon::Post, drogon::Options);
    ADD_METHOD_TO(UserController::login, "/api/auth/login", drogon::Post, drogon::Options);
    ADD_METHOD_TO(UserController::getProfile, "/api/user/profile", drogon::Get, drogon::Options, "auth");
    ADD_METHOD_TO(UserController::updateProfile, "/api/user/profile", drogon::Put, drogon::Options, "auth");
    ADD_METHOD_TO(UserController::getUserById, "/api/user/{1}", drogon::Get, drogon::Options);
    METHOD_LIST_END

    // Drogon 控制器方法签名：
    // 第1个参数：请求智能指针
    // 第2个参数：响应回调，必须通过调用 callback(resp) 返回响应
    // 第3个参数起：路径参数（如 {1} 对应的 userId）
    void registerUser(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void login(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getProfile(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void updateProfile(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getUserById(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                     int64_t userId);
};
