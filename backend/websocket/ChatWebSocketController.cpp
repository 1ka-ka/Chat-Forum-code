#include "ChatWebSocketController.h"
#include "../services/ChatService.h"
#include "../utils/JwtUtil.h"
#include "../utils/ConfigUtil.h"

#include <json/json.h>
#include <drogon/drogon.h>
#include <chrono>
#include <iomanip>
#include <sstream>

// 静态成员初始化
std::unordered_map<int64_t, drogon::WebSocketConnectionPtr> ChatWebSocketController::_connections;
std::shared_mutex ChatWebSocketController::_mutex;

void ChatWebSocketController::handleNewConnection(const drogon::HttpRequestPtr &req,
                                                    const drogon::WebSocketConnectionPtr &conn) {
    // 验证 WebSocket 连接的身份（通过 URL 参数或请求头中的 JWT）
    int64_t userId = authenticateConnection(req);
    if (userId <= 0) {
        LOG_WARN << "WebSocket: connection rejected, authentication failed from " << req->getPeerAddr().toIp();
        conn->forceClose();  // 鉴权失败，强制关闭连接
        return;
    }

    // 将 userId 存入连接上下文，后续收发消息时可以快速获取
    conn->setContext(std::make_shared<int64_t>(userId));

    // 将连接加入在线用户表（写锁保护）
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        _connections[userId] = conn;
    }

    LOG_INFO << "WebSocket: user " << userId << " connected, total connections: " << _connections.size();

    // 向所有在线用户广播该用户上线
    broadcastOnlineStatus(userId, true);

    // 查询该用户的未读消息（当前仅查询，未做推送处理）
    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "SELECT DISTINCT sender_id FROM messages WHERE receiver_id = ? AND is_read = 0 GROUP BY sender_id",
        [this](const drogon::orm::Result &result) {},
        [](const drogon::orm::DrogonDbException &e) {
            LOG_ERROR << "WebSocket: failed to query unread messages - " << e.base().what();
        },
        userId);
}

void ChatWebSocketController::handleNewMessage(const drogon::WebSocketConnectionPtr &conn,
                                                 std::string &&message) {
    // 从连接上下文获取用户 ID
    auto ctx = conn->getContext<int64_t>();
    if (!ctx) return;
    int64_t senderId = *ctx;

    // 解析客户端发来的 JSON 消息
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::istringstream stream(message);
    std::string errs;
    if (!Json::parseFromStream(builder, stream, &root, &errs)) return;

    std::string type = root.get("type", "").asString();

    // 心跳响应：客户端定期发 ping，服务端回 pong 保持连接
    if (type == "ping") {
        Json::Value pong;
        pong["type"] = "pong";
        Json::StreamWriterBuilder wb;
        conn->send(Json::writeString(wb, pong));
        return;
    }

    // 聊天消息处理
    if (type == "message") {
        int64_t receiverId = root.get("receiver_id", 0).asInt64();
        std::string content = root.get("content", "").asString();

        LOG_DEBUG << "WebSocket: user " << senderId << " -> user " << receiverId << ": " << content;

        // 1. 先将消息持久化到数据库
        ChatService chatService;
        chatService.saveMessage(
            senderId, receiverId, content,
            // 成功回调：消息保存成功后，查询发送者信息并转发
            [this, senderId, receiverId, content](int64_t msgId) {
                auto dbClient = drogon::app().getDbClient();
                dbClient->execSqlAsync(
                    "SELECT nickname, avatar_url FROM users WHERE id = ?",
                    [this, senderId, receiverId, content, msgId]
                    (const drogon::orm::Result &result) {
                        std::string nickname = "未知用户";
                        if (!result.empty()) {
                            nickname = std::string(result[0]["nickname"].c_str());
                        }

                        // 构建推送消息 JSON
                        auto now = std::chrono::system_clock::now();
                        auto time_t_now = std::chrono::system_clock::to_time_t(now);
                        std::stringstream ss;
                        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");

                        Json::Value msg;
                        msg["type"] = "message";
                        msg["sender_id"] = senderId;
                        msg["sender_nickname"] = nickname;
                        msg["content"] = content;
                        msg["message_id"] = msgId;
                        msg["created_at"] = ss.str();

                        Json::StreamWriterBuilder wb;
                        std::string msgStr = Json::writeString(wb, msg);

                        // 2. 将消息转发给接收者和发送者（发送者也需要看到自己发的消息）
                        sendMessageToUser(receiverId, msgStr);
                        sendMessageToUser(senderId, msgStr);
                    },
                    [](const drogon::orm::DrogonDbException &e) {
                        LOG_ERROR << "WebSocket: failed to query sender info - " << e.base().what();
                    },
                    senderId);
            },
            []() {
                LOG_ERROR << "WebSocket: failed to save message";
            });
    }
}

void ChatWebSocketController::handleConnectionClosed(const drogon::WebSocketConnectionPtr &conn) {
    auto ctx = conn->getContext<int64_t>();
    if (!ctx) return;
    int64_t userId = *ctx;

    // 从在线用户表中移除（写锁保护）
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        _connections.erase(userId);
    }

    LOG_INFO << "WebSocket: user " << userId << " disconnected, total connections: " << _connections.size();

    // 广播该用户下线
    broadcastOnlineStatus(userId, false);
}

void ChatWebSocketController::broadcastOnlineStatus(int64_t userId, bool online) {
    // 构建在线状态变更消息
    Json::Value msg;
    msg["type"] = "online_status";
    msg["user_id"] = userId;
    msg["online"] = online;
    Json::StreamWriterBuilder wb;
    std::string msgStr = Json::writeString(wb, msg);

    // 读锁：遍历连接表只需要读权限，允许并发广播
    std::shared_lock<std::shared_mutex> lock(_mutex);
    for (auto &[id, conn] : _connections) {
        if (id != userId) conn->send(msgStr);  // 不发给自己
    }
}

void ChatWebSocketController::sendMessageToUser(int64_t userId, const std::string &message) {
    // 读锁：查找连接只需要读权限
    std::shared_lock<std::shared_mutex> lock(_mutex);
    auto it = _connections.find(userId);
    if (it != _connections.end()) {
        it->second->send(message);  // 用户在线则推送，不在线则跳过（消息已持久化）
    }
}

int64_t ChatWebSocketController::authenticateConnection(const drogon::HttpRequestPtr &req) {
    // WebSocket 不支持自定义请求头，因此优先从 URL 参数获取 token
    // 前端连接示例：ws://host/ws/chat?token=xxx
    std::string token = std::string(req->getParameter("token"));
    if (token.empty()) {
        // 备选方案：从请求头获取（某些客户端支持）
        std::string authHeader = std::string(req->getHeader("Authorization"));
        if (!authHeader.empty() && authHeader.substr(0, 7) == "Bearer ") {
            token = authHeader.substr(7);
        }
    }
    if (token.empty()) return -1;

    std::string secret = ConfigUtil::getJwtSecret();
    if (!JwtUtil::verifyToken(token, secret)) return -1;
    return JwtUtil::getUserIdFromToken(token, secret);
}
