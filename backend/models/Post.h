#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include <json/json.h>
#include <drogon/orm/Row.h>

// 帖子模型 - 对应数据库 posts 表
class Post
{
public:
    int64_t getId() const { return _id; }
    void setId(int64_t id) { _id = id; }

    int64_t getUserId() const { return _userId; }
    void setUserId(int64_t userId) { _userId = userId; }

    const std::string& getTitle() const { return _title; }
    void setTitle(const std::string& title) { _title = title; }

    const std::string& getContent() const { return _content; }
    void setContent(const std::string& content) { _content = content; }

    // image_urls 在数据库中以 JSON 字符串存储（如 '["url1","url2"]'）
    // 使用 optional 表示该字段可能为 NULL
    const std::optional<std::string>& getImageUrls() const { return _imageUrls; }
    void setImageUrls(const std::optional<std::string>& imageUrls) { _imageUrls = imageUrls; }

    int getLikeCount() const { return _likeCount; }
    void setLikeCount(int likeCount) { _likeCount = likeCount; }

    int getCommentCount() const { return _commentCount; }
    void setCommentCount(int commentCount) { _commentCount = commentCount; }

    const std::string& getCreatedAt() const { return _createdAt; }
    void setCreatedAt(const std::string& createdAt) { _createdAt = createdAt; }

    const std::string& getUpdatedAt() const { return _updatedAt; }
    void setUpdatedAt(const std::string& updatedAt) { _updatedAt = updatedAt; }

    Json::Value toJson() const;

    static Post fromResult(const drogon::orm::Row& row);

private:
    int64_t _id = 0;
    int64_t _userId = 0;
    std::string _title;
    std::string _content;
    std::optional<std::string> _imageUrls;  // 可能为 NULL，用 optional 表示
    int _likeCount = 0;
    int _commentCount = 0;
    std::string _createdAt;
    std::string _updatedAt;
};
