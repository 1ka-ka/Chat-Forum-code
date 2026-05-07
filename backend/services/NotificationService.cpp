#include "NotificationService.h"
#include "../utils/ResponseUtil.h"

void NotificationService::getNotifications(int64_t userId, int page, int pageSize, Callback &&callback)
{
    int offset = (page - 1) * pageSize;
    auto dbClient = drogon::app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT COUNT(*) AS total FROM notifications WHERE user_id = ?",
        [this, userId, page, pageSize, callback = std::move(callback)]
        (const drogon::orm::Result &countResult) mutable {
            int total = countResult[0]["total"].as<int>();
            int totalPages = (total + pageSize - 1) / pageSize;

            auto db = drogon::app().getDbClient();
            db->execSqlAsync(
                "SELECT n.id, n.type, n.post_id, n.comment_id, n.message, n.is_read, n.created_at, "
                "u.id AS actor_id, u.nickname AS actor_nickname, u.avatar_url AS actor_avatar_url "
                "FROM notifications n LEFT JOIN users u ON n.actor_id = u.id "
                "WHERE n.user_id = ? ORDER BY n.created_at DESC LIMIT ? OFFSET ?",
                [total, totalPages, page, pageSize, callback = std::move(callback)]
                (const drogon::orm::Result &result) mutable {
                    Json::Value items(Json::arrayValue);
                    for (size_t i = 0; i < result.size(); ++i)
                    {
                        Json::Value row;
                        row["id"] = result[i]["id"].as<int64_t>();
                        row["type"] = std::string(result[i]["type"].c_str());
                        row["post_id"] = result[i]["post_id"].isNull()
                                             ? Json::Value()
                                             : result[i]["post_id"].as<int64_t>();
                        row["comment_id"] = result[i]["comment_id"].isNull()
                                                ? Json::Value()
                                                : result[i]["comment_id"].as<int64_t>();
                        row["message"] = std::string(result[i]["message"].c_str());
                        row["is_read"] = result[i]["is_read"].as<int>() > 0;
                        row["created_at"] = std::string(result[i]["created_at"].c_str());
                        row["actor_id"] = result[i]["actor_id"].as<int64_t>();
                        row["actor_nickname"] = std::string(result[i]["actor_nickname"].c_str());
                        row["actor_avatar_url"] = std::string(result[i]["actor_avatar_url"].c_str());
                        items.append(row);
                    }
                    Json::Value data;
                    data["items"] = items;
                    data["total"] = total;
                    data["page"] = page;
                    data["page_size"] = pageSize;
                    data["total_pages"] = totalPages;
                    callback(ResponseUtil::success(data));
                },
                [callback = std::move(callback)]
                (const drogon::orm::DrogonDbException &e) mutable {
                    callback(ResponseUtil::serverError("查询通知列表失败"));
                },
                userId, pageSize, offset);
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("查询通知数量失败"));
        },
        userId);
}

void NotificationService::getUnreadCount(int64_t userId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "SELECT COUNT(*) AS count FROM notifications WHERE user_id = ? AND is_read = 0",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            int count = result[0]["count"].as<int>();
            Json::Value data;
            data["count"] = count;
            callback(ResponseUtil::success(data));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("查询未读数失败"));
        },
        userId);
}

void NotificationService::markAsRead(int64_t notificationId, int64_t userId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "UPDATE notifications SET is_read = 1 WHERE id = ? AND user_id = ?",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            callback(ResponseUtil::success(Json::Value(), "已标记为已读"));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("标记已读失败"));
        },
        notificationId, userId);
}

void NotificationService::markAllAsRead(int64_t userId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "UPDATE notifications SET is_read = 1 WHERE user_id = ? AND is_read = 0",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            callback(ResponseUtil::success(Json::Value(), "全部已标记为已读"));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("标记全部已读失败"));
        },
        userId);
}

void NotificationService::createNotification(int64_t userId, int64_t actorId,
                                               const std::string &type,
                                               int64_t postId,
                                               int64_t commentId,
                                               const std::string &message)
{
    // 不给自己发通知（如自己点赞自己的帖子）
    if (userId == actorId) return;

    // fire-and-forget 模式：成功和失败回调都为空，不等待结果
    // 通知创建失败不应影响主业务（如点赞、评论）的响应
    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "INSERT INTO notifications (user_id, actor_id, type, post_id, comment_id, message) "
        "VALUES (?, ?, ?, ?, ?, ?)",
        [](const drogon::orm::Result &) {},
        [](const drogon::orm::DrogonDbException &) {},
        userId, actorId, type,
        postId > 0 ? std::to_string(postId) : "NULL",      // postId 为 0 表示与帖子无关
        commentId > 0 ? std::to_string(commentId) : "NULL", // commentId 为 0 表示与评论无关
        message);
}
