#include "LikeController.h"
#include "../services/LikeService.h"

void LikeController::toggleLike(const drogon::HttpRequestPtr &req,
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                 int64_t postId)
{
    int64_t userId = req->getAttribute<int64_t>("userId");

    LikeService service;
    service.toggleLike(postId, userId, std::move(callback));
}

void LikeController::getLikeUsers(const drogon::HttpRequestPtr &req,
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                    int64_t postId)
{
    LikeService service;
    service.getLikeUsers(postId, std::move(callback));
}
