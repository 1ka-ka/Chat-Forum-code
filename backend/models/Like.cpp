#include "Like.h"

Json::Value Like::toJson() const
{
    Json::Value val;
    val["id"] = static_cast<Json::Int64>(_id);
    val["postId"] = static_cast<Json::Int64>(_postId);
    val["userId"] = static_cast<Json::Int64>(_userId);
    val["createdAt"] = _createdAt;
    return val;
}

Like Like::fromResult(const drogon::orm::Row& row)
{
    Like like;
    like._id = row["id"].as<int64_t>();
    like._postId = row["post_id"].as<int64_t>();
    like._userId = row["user_id"].as<int64_t>();
    like._createdAt = row["created_at"].as<std::string>();
    return like;
}
