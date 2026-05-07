#pragma once

#include <string>
#include <functional>
#include <drogon/drogon.h>

// 用户服务层 - 封装用户相关的业务逻辑
// Controller 只负责提取参数和调用 Service，Service 负责数据库操作和业务处理
class UserService
{
public:
    // Callback 类型：用于异步返回 HTTP 响应
    // Drogon 的异步模型要求通过回调返回响应，而不是函数返回值
    using Callback = std::function<void(const drogon::HttpResponsePtr &)>;

    void registerUser(const std::string &username,
                      const std::string &password,
                      const std::string &nickname,
                      Callback &&callback);

    void login(const std::string &username,
               const std::string &password,
               Callback &&callback);

    void getProfile(int64_t userId, Callback &&callback);

    void updateProfile(int64_t userId,
                       const std::string &nickname,
                       const std::string &avatarUrl,
                       Callback &&callback);

    void getUserById(int64_t userId, Callback &&callback);

private:
    std::string hashPassword(const std::string &password);       // bcrypt 哈希
    bool verifyPassword(const std::string &password, const std::string &hash);  // bcrypt 验证
    std::string getJwtSecret();
    int getJwtExpiration();
};
