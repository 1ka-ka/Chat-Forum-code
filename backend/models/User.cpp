#include "User.h"

Json::Value User::toJson() const
{
    // 序列化为 API 响应 JSON，注意不输出 passwordHash 保护密码安全
    Json::Value val;
    val["id"] = static_cast<Json::Int64>(_id);
    val["username"] = _username;
    val["nickname"] = _nickname;
    val["avatar_url"] = _avatarUrl;
    val["created_at"] = _createdAt;
    val["updated_at"] = _updatedAt;
    return val;
}

User User::fromResult(const drogon::orm::Row& row)
{
    // 从数据库查询结果的一行（drogon::orm::Row）构造 User 对象
    // row["column_name"] 按列名访问，as<T>() 转换为 C++ 类型
    User user;
    user._id = row["id"].as<int64_t>();
    user._username = row["username"].as<std::string>();
    user._passwordHash = row["password_hash"].as<std::string>();
    user._nickname = row["nickname"].as<std::string>();
    user._avatarUrl = row["avatar_url"].as<std::string>();
    user._createdAt = row["created_at"].as<std::string>();
    user._updatedAt = row["updated_at"].as<std::string>();
    return user;
}
