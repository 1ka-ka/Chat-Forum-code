#include "NotificationController.h"
#include "../services/NotificationService.h"

void NotificationController::getNotifications(const drogon::HttpRequestPtr &req,
                                                std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    int64_t userId = req->getAttribute<int64_t>("userId");

    int page = std::stoi(req->getParameter("page", "1"));
    int pageSize = std::stoi(req->getParameter("page_size", "20"));

    NotificationService service;
    service.getNotifications(userId, page, pageSize, std::move(callback));
}

void NotificationController::getUnreadCount(const drogon::HttpRequestPtr &req,
                                              std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    int64_t userId = req->getAttribute<int64_t>("userId");

    NotificationService service;
    service.getUnreadCount(userId, std::move(callback));
}

void NotificationController::markAsRead(const drogon::HttpRequestPtr &req,
                                          std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                          int64_t notificationId)
{
    int64_t userId = req->getAttribute<int64_t>("userId");

    // 同时传入 userId 确保只能标记自己的通知，防止越权
    NotificationService service;
    service.markAsRead(notificationId, userId, std::move(callback));
}

void NotificationController::markAllAsRead(const drogon::HttpRequestPtr &req,
                                             std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    int64_t userId = req->getAttribute<int64_t>("userId");

    NotificationService service;
    service.markAllAsRead(userId, std::move(callback));
}
