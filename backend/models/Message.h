#pragma once

#include <string>
#include <cstdint>
#include <json/json.h>
#include <drogon/orm/Row.h>

// 消息模型 - 对应数据库 messages 表
// 用于私聊消息的存储，is_read 字段标记是否已读
class Message
{
public:
    int64_t getId() const { return _id; }
    void setId(int64_t id) { _id = id; }

    int64_t getSenderId() const { return _senderId; }
    void setSenderId(int64_t senderId) { _senderId = senderId; }

    int64_t getReceiverId() const { return _receiverId; }
    void setReceiverId(int64_t receiverId) { _receiverId = receiverId; }

    const std::string& getContent() const { return _content; }
    void setContent(const std::string& content) { _content = content; }

    int getIsRead() const { return _isRead; }
    void setIsRead(int isRead) { _isRead = isRead; }

    const std::string& getCreatedAt() const { return _createdAt; }
    void setCreatedAt(const std::string& createdAt) { _createdAt = createdAt; }

    Json::Value toJson() const;

    static Message fromResult(const drogon::orm::Row& row);

private:
    int64_t _id = 0;
    int64_t _senderId = 0;
    int64_t _receiverId = 0;
    std::string _content;
    int _isRead = 0;
    std::string _createdAt;
};
