#include "AuthMiddleware.h"
#include "JwtUtil.h"
#include "ResponseUtil.h"
#include "ConfigUtil.h"
#include <drogon/drogon.h>

void AuthMiddleware::invoke(const drogon::HttpRequestPtr &req,
                            drogon::MiddlewareNextCallback &&nextCb,
                            drogon::MiddlewareCallback &&mcb) {
    // 1. 从请求头提取 Authorization 字段
    std::string authHeader = std::string(req->getHeader("Authorization"));
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
        LOG_DEBUG << "Auth: missing or invalid Authorization header from " << req->getPeerAddr().toIp();
        mcb(ResponseUtil::unauthorized());  // mcb：直接返回响应，不继续执行后续逻辑
        return;
    }

    // 2. 提取 Bearer 后面的 JWT 令牌
    std::string token = authHeader.substr(7);

    // 3. 验证令牌签名和有效期
    std::string secret = getJwtSecret();
    if (!JwtUtil::verifyToken(token, secret)) {
        LOG_WARN << "Auth: JWT verification failed from " << req->getPeerAddr().toIp();
        mcb(ResponseUtil::unauthorized());
        return;
    }

    // 4. 从令牌中提取用户 ID
    int64_t userId = JwtUtil::getUserIdFromToken(token, secret);
    if (userId <= 0) {
        LOG_WARN << "Auth: invalid userId in token from " << req->getPeerAddr().toIp();
        mcb(ResponseUtil::unauthorized());
        return;
    }

    LOG_DEBUG << "Auth: user " << userId << " authenticated for " << req->getPath();

    // 5. 将 userId 存入请求属性，后续控制器通过 getAttribute 获取
    req->setAttribute("userId", userId);

    // 6. 调用 nextCb 继续执行后续中间件和控制器
    nextCb([mcb = std::move(mcb)](const drogon::HttpResponsePtr &resp) {
        mcb(resp);  // 控制器处理完成后，通过 mcb 返回响应
    });
}

std::string AuthMiddleware::getJwtSecret() const {
    return ConfigUtil::getJwtSecret();
}
