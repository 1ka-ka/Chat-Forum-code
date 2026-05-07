#include "Comment.h"

Json::Value Comment::toJson() const
{
    Json::Value val;
    val["id"] = static_cast<Json::Int64>(_id);
    val["post_id"] = static_cast<Json::Int64>(_postId);
    val["user_id"] = static_cast<Json::Int64>(_userId);

    // optional 字段：有值时输出数字，无值时输出 null
    if (_parentCommentId.has_value())
    {
        val["parent_comment_id"] = static_cast<Json::Int64>(_parentCommentId.value());
    }
    else
    {
        val["parent_comment_id"] = Json::nullValue;
    }

    val["content"] = _content;
    val["created_at"] = _createdAt;
    return val;
}

Comment Comment::fromResult(const drogon::orm::Row& row)
{
    Comment comment;
    comment._id = row["id"].as<int64_t>();
    comment._postId = row["post_id"].as<int64_t>();
    comment._userId = row["user_id"].as<int64_t>();

    // 数据库中 parent_comment_id 可能为 NULL，需要先判断
    if (!row["parent_comment_id"].isNull())
    {
        comment._parentCommentId = row["parent_comment_id"].as<int64_t>();
    }

    comment._content = row["content"].as<std::string>();
    comment._createdAt = row["created_at"].as<std::string>();
    return comment;
}
