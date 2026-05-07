#pragma once

#include <string>

// 配置工具类 - 从 Drogon 自定义配置中读取应用参数
// 配置文件为 config.json 中的 "custom_config" 节点
class ConfigUtil {
public:
    static std::string getJwtSecret();     // 获取 JWT 签名密钥
    static int getJwtExpiration();         // 获取 JWT 过期时间（小时）
};
