#pragma once

#include <drogon/drogon.h>

// 通知控制器 - 处理通知的查询、已读标记等
class NotificationController : public drogon::HttpController<NotificationController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(NotificationController::getNotifications, "/api/notifications", drogon::Get, drogon::Options, "auth");
    ADD_METHOD_TO(NotificationController::getUnreadCount, "/api/notifications/unread_count", drogon::Get, drogon::Options, "auth");
    ADD_METHOD_TO(NotificationController::markAsRead, "/api/notifications/read/{1}", drogon::Put, drogon::Options, "auth");
    ADD_METHOD_TO(NotificationController::markAllAsRead, "/api/notifications/read_all", drogon::Put, drogon::Options, "auth");
    METHOD_LIST_END

    void getNotifications(const drogon::HttpRequestPtr &req,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getUnreadCount(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void markAsRead(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                    int64_t notificationId);

    void markAllAsRead(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
