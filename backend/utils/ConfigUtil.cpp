#include "ConfigUtil.h"
#include <drogon/drogon.h>

std::string ConfigUtil::getJwtSecret() {
    // getCustomConfig() 返回 config.json 中 "custom_config" 节点的 JSON 对象
    auto &config = drogon::app().getCustomConfig();
    if (config.isMember("jwt") && config["jwt"].isMember("secret")) {
        return config["jwt"]["secret"].asString();
    }
    // 配置文件中未设置时使用默认密钥（仅用于开发环境，生产环境必须配置）
    return "chatforum-secret-key-change-in-production-2025";
}

int ConfigUtil::getJwtExpiration() {
    auto &config = drogon::app().getCustomConfig();
    if (config.isMember("jwt") && config["jwt"].isMember("expiration_hours")) {
        return config["jwt"]["expiration_hours"].asInt();
    }
    return 24;  // 默认24小时过期
}
