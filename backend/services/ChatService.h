#pragma once

#include <string>
#include <functional>
#include <drogon/drogon.h>

// 聊天服务层 - 封装聊天消息的保存和查询逻辑
class ChatService
{
public:
    using Callback = std::function<void(const drogon::HttpResponsePtr &)>;

    // saveMessage 使用双回调模式（而非统一的 Callback）：
    // 成功回调接收 messageId，失败回调无参数
    // 这样设计是因为 WebSocket 控制器需要 messageId 来构建推送消息
    void saveMessage(int64_t senderId,
                     int64_t receiverId,
                     const std::string &content,
                     std::function<void(int64_t)> successCallback,
                     std::function<void()> errorCallback);

    void getConversations(int64_t userId, Callback &&callback);

    void getMessages(int64_t userId,
                     int64_t otherUserId,
                     int page,
                     int pageSize,
                     Callback &&callback);

    void markAsRead(int64_t userId, int64_t otherUserId, Callback &&callback);
};
