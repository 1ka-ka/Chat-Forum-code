#pragma once

#include <string>
#include <vector>

// 校验工具类 - 提供各种输入校验规则
// 在 Service 层调用，确保数据格式合法后再执行数据库操作
class ValidationUtil {
public:
    static bool isValidUsername(const std::string &username);      // 3-50位，字母数字下划线
    static bool isValidPassword(const std::string &password);      // 6-50位
    static bool isValidNickname(const std::string &nickname);      // 非空，最长50
    static bool isValidImageExt(const std::string &filename);      // jpg/jpeg/png/gif
    static bool isValidTitle(const std::string &title);            // 非空，最长200
    static bool isValidCommentContent(const std::string &content); // 非空，最长1000

private:
    static bool isAlphanumericOrUnderscore(const std::string &str);
};
