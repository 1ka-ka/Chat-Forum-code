#include "ChatService.h"
#include "../utils/ResponseUtil.h"
#include "../models/Message.h"

void ChatService::saveMessage(int64_t senderId,
                               int64_t receiverId,
                               const std::string &content,
                               std::function<void(int64_t)> successCallback,
                               std::function<void()> errorCallback)
{
    auto dbClient = drogon::app().getDbClient();
    // 双回调模式：成功时返回消息 ID，失败时通知调用方
    dbClient->execSqlAsync(
        "INSERT INTO messages (sender_id, receiver_id, content) VALUES (?, ?, ?)",
        [successCallback = std::move(successCallback)]
        (const drogon::orm::Result &result) mutable {
            int64_t msgId = result.insertId();
            successCallback(msgId);  // 成功：将消息 ID 传给调用方
        },
        [errorCallback = std::move(errorCallback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            errorCallback();  // 失败：通知调用方
        },
        senderId, receiverId, content);
}

void ChatService::getConversations(int64_t userId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    // 复杂 SQL 解读：
    // 1. INNER JOIN messages m：找到每个会话的最新一条消息
    //    子查询 m2 按 created_at DESC LIMIT 1 取最新消息的 id
    // 2. WHERE 条件：(sender_id=? AND receiver_id=u.id) OR 反之，即两人之间的消息
    // 3. EXISTS 子查询：只返回与当前用户有过消息往来的用户
    // 4. unread_count 子查询：统计每个会话的未读消息数
    dbClient->execSqlAsync(
        "SELECT u.id AS user_id, u.nickname, u.avatar_url, "
        "m.content AS last_message, m.created_at AS last_message_time, "
        "(SELECT COUNT(*) FROM messages WHERE sender_id = u.id AND receiver_id = ? AND is_read = 0) AS unread_count "
        "FROM users u "
        "INNER JOIN messages m ON m.id = ("
        "  SELECT m2.id FROM messages m2 "
        "  WHERE (m2.sender_id = ? AND m2.receiver_id = u.id) "
        "     OR (m2.sender_id = u.id AND m2.receiver_id = ?) "
        "  ORDER BY m2.created_at DESC LIMIT 1"
        ") "
        "WHERE u.id != ? AND EXISTS ("
        "  SELECT 1 FROM messages m3 "
        "  WHERE (m3.sender_id = ? AND m3.receiver_id = u.id) "
        "     OR (m3.sender_id = u.id AND m3.receiver_id = ?)"
        ") "
        "ORDER BY m.created_at DESC",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            Json::Value items(Json::arrayValue);
            for (size_t i = 0; i < result.size(); ++i)
            {
                Json::Value row;
                row["user_id"] = result[i]["user_id"].as<int64_t>();
                row["nickname"] = std::string(result[i]["nickname"].c_str());
                row["avatar_url"] = std::string(result[i]["avatar_url"].c_str());
                row["last_message"] = std::string(result[i]["last_message"].c_str());
                row["last_message_time"] = std::string(result[i]["last_message_time"].c_str());
                row["unread_count"] = result[i]["unread_count"].as<int>();
                items.append(row);
            }
            callback(ResponseUtil::success(items));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("查询会话列表失败"));
        },
        userId, userId, userId, userId, userId, userId);  // 6 个 ? 分别对应 SQL 中的 6 个 ?
}

void ChatService::getMessages(int64_t userId, int64_t otherUserId,
                               int page, int pageSize, Callback &&callback)
{
    int offset = (page - 1) * pageSize;
    auto dbClient = drogon::app().getDbClient();
    // 查询两人之间的所有消息：(A→B) OR (B→A)
    dbClient->execSqlAsync(
        "SELECT m.id, m.sender_id, m.receiver_id, m.content, m.is_read, m.created_at "
        "FROM messages m "
        "WHERE (m.sender_id = ? AND m.receiver_id = ?) "
        "   OR (m.sender_id = ? AND m.receiver_id = ?) "
        "ORDER BY m.created_at DESC LIMIT ? OFFSET ?",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            Json::Value items(Json::arrayValue);
            for (size_t i = 0; i < result.size(); ++i)
            {
                auto msg = Message::fromResult(result[i]);
                items.append(msg.toJson());
            }
            callback(ResponseUtil::success(items));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("查询消息列表失败"));
        },
        userId, otherUserId, otherUserId, userId, pageSize, offset);
}

void ChatService::markAsRead(int64_t userId, int64_t otherUserId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    // 将对方发给自己的未读消息标记为已读
    dbClient->execSqlAsync(
        "UPDATE messages SET is_read = 1 WHERE sender_id = ? AND receiver_id = ? AND is_read = 0",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            callback(ResponseUtil::success(Json::Value(), "标记已读成功"));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("标记已读失败"));
        },
        otherUserId, userId);
}
