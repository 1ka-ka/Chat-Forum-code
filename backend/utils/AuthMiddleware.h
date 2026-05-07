#pragma once

#include <drogon/drogon.h>

// 认证中间件 - 验证 JWT 令牌，保护需要登录的接口
// Drogon 中间件机制：继承 HttpMiddleware<T>，实现 invoke 方法
// 在路由声明中通过名称（如 "auth"）引用，请求到达控制器前会先经过中间件
class AuthMiddleware : public drogon::HttpMiddleware<AuthMiddleware> {
public:
    // invoke 是中间件的核心方法：
    // req: 当前请求
    // nextCb: 调用后继续执行后续中间件/控制器
    // mcb: 中间件回调，用于直接返回响应（如鉴权失败时）
    void invoke(const drogon::HttpRequestPtr &req,
                drogon::MiddlewareNextCallback &&nextCb,
                drogon::MiddlewareCallback &&mcb) override;

private:
    std::string getJwtSecret() const;
};
