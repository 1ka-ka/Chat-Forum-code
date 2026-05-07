#pragma once

#include <drogon/drogon.h>
#include <unordered_map>
#include <shared_mutex>
#include <cstdint>

// WebSocket 聊天控制器 - 处理实时消息推送
// Drogon 的 WebSocketController 与 HttpController 类似，但处理的是 WebSocket 连接
// 需要实现三个生命周期方法：连接建立、收到消息、连接关闭
class ChatWebSocketController : public drogon::WebSocketController<ChatWebSocketController> {
public:
    // WS_PATH_LIST_BEGIN / END 类似 HttpController 的 METHOD_LIST
    // WS_PATH_ADD 声明 WebSocket 路由路径和允许的 HTTP 方法（仅用于握手阶段）
    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/ws/chat", drogon::Get);
    WS_PATH_LIST_END

    // 新连接建立时调用（WebSocket 握手成功后）
    void handleNewConnection(const drogon::HttpRequestPtr &req,
                             const drogon::WebSocketConnectionPtr &conn) override;
    // 收到客户端消息时调用
    void handleNewMessage(const drogon::WebSocketConnectionPtr &conn,
                          std::string &&message) override;
    // 连接关闭时调用
    void handleConnectionClosed(const drogon::WebSocketConnectionPtr &conn) override;

private:
    // 在线用户连接表：userId → WebSocket 连接
    // static 成员因为控制器实例是临时创建的，需要共享连接状态
    static std::unordered_map<int64_t, drogon::WebSocketConnectionPtr> _connections;
    // 读写锁：允许多个线程同时读，但写时独占
    // 使用 shared_mutex 而非 mutex 是因为读多写少（频繁查连接，偶尔增删）
    static std::shared_mutex _mutex;

    void broadcastOnlineStatus(int64_t userId, bool online);  // 广播在线状态
    void sendMessageToUser(int64_t userId, const std::string &message);  // 向指定用户发送消息
    int64_t authenticateConnection(const drogon::HttpRequestPtr &req);   // 验证 WebSocket 连接身份
};
