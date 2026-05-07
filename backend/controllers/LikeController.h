#pragma once

#include <drogon/drogon.h>

// 点赞控制器 - 处理帖子的点赞/取消点赞和点赞用户列表
class LikeController : public drogon::HttpController<LikeController>
{
public:
    METHOD_LIST_BEGIN
    // toggleLike：点赞和取消点赞共用一个接口，由 Service 层判断当前状态
    ADD_METHOD_TO(LikeController::toggleLike, "/api/posts/{1}/like", drogon::Post, drogon::Options, "auth");
    ADD_METHOD_TO(LikeController::getLikeUsers, "/api/posts/{1}/like/users", drogon::Get, drogon::Options);
    METHOD_LIST_END

    void toggleLike(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                    int64_t postId);

    void getLikeUsers(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                      int64_t postId);
};
