#pragma once

#include <string>
#include <cstdint>
#include <json/json.h>
#include <drogon/orm/Row.h>

// 点赞模型 - 对应数据库 likes 表
// 记录用户对帖子的点赞关系，(post_id, user_id) 联合唯一
class Like
{
public:
    int64_t getId() const { return _id; }
    void setId(int64_t id) { _id = id; }

    int64_t getPostId() const { return _postId; }
    void setPostId(int64_t postId) { _postId = postId; }

    int64_t getUserId() const { return _userId; }
    void setUserId(int64_t userId) { _userId = userId; }

    const std::string& getCreatedAt() const { return _createdAt; }
    void setCreatedAt(const std::string& createdAt) { _createdAt = createdAt; }

    Json::Value toJson() const;

    static Like fromResult(const drogon::orm::Row& row);

private:
    int64_t _id = 0;
    int64_t _postId = 0;
    int64_t _userId = 0;
    std::string _createdAt;
};
