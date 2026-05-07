#include "Post.h"

Json::Value Post::toJson() const
{
    Json::Value val;
    val["id"] = static_cast<Json::Int64>(_id);
    val["user_id"] = static_cast<Json::Int64>(_userId);
    val["title"] = _title;
    val["content"] = _content;

    // image_urls 在数据库中存储为 JSON 字符串，需要解析为 JSON 数组输出
    if (_imageUrls.has_value())
    {
        Json::Value images;
        Json::CharReaderBuilder builder;
        std::istringstream stream(_imageUrls.value());
        std::string errors;
        if (Json::parseFromStream(builder, stream, &images, &errors))
        {
            val["image_urls"] = images;  // 解析成功，直接赋值 JSON 数组
        }
        else
        {
            val["image_urls"] = Json::arrayValue;  // 解析失败，返回空数组
        }
    }
    else
    {
        val["image_urls"] = Json::arrayValue;  // 数据库中为 NULL，返回空数组
    }

    val["like_count"] = _likeCount;
    val["comment_count"] = _commentCount;
    val["created_at"] = _createdAt;
    val["updated_at"] = _updatedAt;
    return val;
}

Post Post::fromResult(const drogon::orm::Row& row)
{
    Post post;
    post._id = row["id"].as<int64_t>();
    post._userId = row["user_id"].as<int64_t>();
    post._title = row["title"].as<std::string>();
    post._content = row["content"].as<std::string>();

    // image_urls 可能为 NULL，先检查再赋值
    if (!row["image_urls"].isNull())
    {
        post._imageUrls = row["image_urls"].as<std::string>();
    }

    post._likeCount = row["like_count"].as<int>();
    post._commentCount = row["comment_count"].as<int>();
    post._createdAt = row["created_at"].as<std::string>();
    post._updatedAt = row["updated_at"].as<std::string>();
    return post;
}
