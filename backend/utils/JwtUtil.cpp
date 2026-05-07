#include "JwtUtil.h"
#include <jwt-cpp/jwt.h>
#include <chrono>

std::string JwtUtil::generateToken(int64_t userId, const std::string &secret, int expirationHours) {
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::hours(expirationHours);

    // 使用 jwt-cpp 的建造者模式构建令牌
    return jwt::create()
        .set_issuer("chatforum")                                    // 签发者标识
        .set_type("JWT")                                            // 令牌类型
        .set_payload_claim("user_id", jwt::claim(std::to_string(userId)))  // 自定义声明：用户 ID
        .set_issued_at(now)                                         // 签发时间
        .set_expires_at(exp)                                        // 过期时间
        .sign(jwt::algorithm::hs256{secret});                       // 使用 HS256 算法签名
}

bool JwtUtil::verifyToken(const std::string &token, const std::string &secret) {
    try {
        auto decoded = jwt::decode(token);  // 解码令牌（不验证）
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{secret})  // 指定验证算法
                            .with_issuer("chatforum");                       // 验证签发者
        verifier.verify(decoded);  // 验证签名和有效期，失败会抛异常
        return true;
    } catch (const std::exception &) {
        return false;  // 任何验证失败都返回 false
    }
}

int64_t JwtUtil::getUserIdFromToken(const std::string &token, const std::string &secret) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{secret})
                            .with_issuer("chatforum");
        verifier.verify(decoded);  // 先验证再提取，确保令牌有效
        auto userIdStr = decoded.get_payload_claim("user_id").as_string();
        return std::stoll(userIdStr);  // 字符串转 int64_t
    } catch (const std::exception &) {
        return -1;  // 解析失败返回 -1
    }
}
