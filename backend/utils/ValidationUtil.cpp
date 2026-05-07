#include "ValidationUtil.h"
#include <algorithm>

bool ValidationUtil::isAlphanumericOrUnderscore(const std::string &str) {
    // 检查字符串是否只包含字母、数字和下划线
    return std::all_of(str.begin(), str.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    });
}

bool ValidationUtil::isValidUsername(const std::string &username) {
    // 用户名：3-50位，只允许字母、数字、下划线
    if (username.length() < 3 || username.length() > 50) return false;
    return isAlphanumericOrUnderscore(username);
}

bool ValidationUtil::isValidPassword(const std::string &password) {
    // 密码：6-50位，不做复杂度要求（由前端提示即可）
    return password.length() >= 6 && password.length() <= 50;
}

bool ValidationUtil::isValidNickname(const std::string &nickname) {
    // 昵称：非空，最长50字符（允许中文等 Unicode 字符）
    return !nickname.empty() && nickname.length() <= 50;
}

bool ValidationUtil::isValidImageExt(const std::string &filename) {
    // 检查文件扩展名是否在允许列表中
    static const std::vector<std::string> allowed = {".jpg", ".jpeg", ".png", ".gif"};
    std::string lower = filename;
    // 先转小写再做后缀匹配，避免大小写问题
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    for (const auto &ext : allowed) {
        if (lower.size() >= ext.size() &&
            lower.compare(lower.size() - ext.size(), ext.size(), ext) == 0) {
            return true;
        }
    }
    return false;
}

bool ValidationUtil::isValidTitle(const std::string &title) {
    // 标题：非空，最长200字符
    return !title.empty() && title.length() <= 200;
}

bool ValidationUtil::isValidCommentContent(const std::string &content) {
    // 评论内容：非空，最长1000字符
    return !content.empty() && content.length() <= 1000;
}
