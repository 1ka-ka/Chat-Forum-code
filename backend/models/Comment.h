#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include <json/json.h>
#include <drogon/orm/Row.h>

// 评论模型 - 对应数据库 comments 表
class Comment
{
public:
    int64_t getId() const { return _id; }
    void setId(int64_t id) { _id = id; }

    int64_t getPostId() const { return _postId; }
    void setPostId(int64_t postId) { _postId = postId; }

    int64_t getUserId() const { return _userId; }
    void setUserId(int64_t userId) { _userId = userId; }

    // parent_comment_id 为 NULL 表示顶级评论，有值表示回复某条评论（楼中楼）
    // 使用 optional<int64_t> 表示数据库中可能为 NULL 的字段
    const std::optional<int64_t>& getParentCommentId() const { return _parentCommentId; }
    void setParentCommentId(const std::optional<int64_t>& parentCommentId) { _parentCommentId = parentCommentId; }

    const std::string& getContent() const { return _content; }
    void setContent(const std::string& content) { _content = content; }

    const std::string& getCreatedAt() const { return _createdAt; }
    void setCreatedAt(const std::string& createdAt) { _createdAt = createdAt; }

    Json::Value toJson() const;

    static Comment fromResult(const drogon::orm::Row& row);

private:
    int64_t _id = 0;
    int64_t _postId = 0;
    int64_t _userId = 0;
    std::optional<int64_t> _parentCommentId;  // 可能为 NULL
    std::string _content;
    std::string _createdAt;
};
