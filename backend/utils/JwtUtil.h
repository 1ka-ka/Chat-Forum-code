#pragma once

#include <string>
#include <cstdint>

// JWT 工具类 - 封装 JWT 令牌的生成、验证和解析
// 使用 jwt-cpp 库实现，算法为 HS256（HMAC-SHA256）
class JwtUtil {
public:
    // 生成 JWT 令牌，payload 中包含 user_id
    static std::string generateToken(int64_t userId, const std::string &secret, int expirationHours);
    // 验证令牌签名和有效期
    static bool verifyToken(const std::string &token, const std::string &secret);
    // 从令牌中提取 user_id（验证通过后调用）
    static int64_t getUserIdFromToken(const std::string &token, const std::string &secret);
};
