#pragma once

#include <string>
#include <cstdint>
#include <json/json.h>
#include <drogon/orm/Row.h>

// 用户模型 - 对应数据库 users 表
// 手动 ORM 设计：通过 fromResult 从数据库行构造对象，通过 toJson 序列化为 JSON
class User
{
public:
    int64_t getId() const { return _id; }
    void setId(int64_t id) { _id = id; }

    const std::string& getUsername() const { return _username; }
    void setUsername(const std::string& username) { _username = username; }

    const std::string& getPasswordHash() const { return _passwordHash; }
    void setPasswordHash(const std::string& passwordHash) { _passwordHash = passwordHash; }

    const std::string& getNickname() const { return _nickname; }
    void setNickname(const std::string& nickname) { _nickname = nickname; }

    const std::string& getAvatarUrl() const { return _avatarUrl; }
    void setAvatarUrl(const std::string& avatarUrl) { _avatarUrl = avatarUrl; }

    const std::string& getCreatedAt() const { return _createdAt; }
    void setCreatedAt(const std::string& createdAt) { _createdAt = createdAt; }

    const std::string& getUpdatedAt() const { return _updatedAt; }
    void setUpdatedAt(const std::string& updatedAt) { _updatedAt = updatedAt; }

    // 将模型转为 JSON 对象，用于 API 响应（注意：不包含 passwordHash，避免泄露密码）
    Json::Value toJson() const;

    // 从数据库查询结果行构造 User 对象
    static User fromResult(const drogon::orm::Row& row);

private:
    int64_t _id = 0;
    std::string _username;
    std::string _passwordHash;
    std::string _nickname;
    std::string _avatarUrl;
    std::string _createdAt;
    std::string _updatedAt;
};
