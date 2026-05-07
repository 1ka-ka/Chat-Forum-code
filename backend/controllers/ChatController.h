#pragma once

#include <drogon/drogon.h>

// 聊天控制器 - 处理会话列表、消息历史、消息发送等 HTTP 接口
// 实时消息推送由 WebSocket 控制器负责，这里只处理 REST 接口
class ChatController : public drogon::HttpController<ChatController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ChatController::getConversations, "/api/chat/conversations", drogon::Get, drogon::Options, "auth");
    ADD_METHOD_TO(ChatController::getMessages, "/api/chat/messages/{1}", drogon::Get, drogon::Options, "auth");
    ADD_METHOD_TO(ChatController::markAsRead, "/api/chat/read/{1}", drogon::Put, drogon::Options, "auth");
    ADD_METHOD_TO(ChatController::sendMessage, "/api/chat/send", drogon::Post, drogon::Options, "auth");
    METHOD_LIST_END

    void getConversations(const drogon::HttpRequestPtr &req,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getMessages(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                     int64_t otherUserId);

    void markAsRead(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                    int64_t otherUserId);

    void sendMessage(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
