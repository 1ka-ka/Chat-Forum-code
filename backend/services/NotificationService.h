#pragma once

#include <string>
#include <functional>
#include <drogon/drogon.h>

// 通知服务层 - 封装通知的查询和创建逻辑
class NotificationService
{
public:
    using Callback = std::function<void(const drogon::HttpResponsePtr &)>;

    void getNotifications(int64_t userId, int page, int pageSize, Callback &&callback);

    void getUnreadCount(int64_t userId, Callback &&callback);

    void markAsRead(int64_t notificationId, int64_t userId, Callback &&callback);

    void markAllAsRead(int64_t userId, Callback &&callback);

    // 创建通知（供其他 Service 调用，不走 HTTP 回调）
    // 使用 fire-and-forget 模式：发起数据库操作后不等待结果
    // 适用于"通知创建失败不应影响主业务流程"的场景
    static void createNotification(int64_t userId, int64_t actorId,
                                    const std::string &type,
                                    int64_t postId,
                                    int64_t commentId,
                                    const std::string &message);
};
