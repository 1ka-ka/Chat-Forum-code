#include "Message.h"

Json::Value Message::toJson() const
{
    Json::Value val;
    val["id"] = static_cast<Json::Int64>(_id);
    val["senderId"] = static_cast<Json::Int64>(_senderId);
    val["receiverId"] = static_cast<Json::Int64>(_receiverId);
    val["content"] = _content;
    val["isRead"] = _isRead;  // 0=未读，1=已读
    val["createdAt"] = _createdAt;
    return val;
}

Message Message::fromResult(const drogon::orm::Row& row)
{
    Message msg;
    msg._id = row["id"].as<int64_t>();
    msg._senderId = row["sender_id"].as<int64_t>();
    msg._receiverId = row["receiver_id"].as<int64_t>();
    msg._content = row["content"].as<std::string>();
    msg._isRead = row["is_read"].as<int>();
    msg._createdAt = row["created_at"].as<std::string>();
    return msg;
}
