#include "CommentController.h"
#include "../services/CommentService.h"
#include "../utils/ResponseUtil.h"

void CommentController::createComment(const drogon::HttpRequestPtr &req,
                                       std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                       int64_t postId)
{
    int64_t userId = req->getAttribute<int64_t>("userId");

    auto json = req->getJsonObject();
    if (!json)
    {
        callback(ResponseUtil::badRequest("请求体格式错误"));
        return;
    }

    std::string content = (*json).get("content", "").asString();
    // parent_comment_id > 0 表示回复某条评论（楼中楼），= 0 表示直接评论帖子
    int64_t parentCommentId = (*json).get("parent_comment_id", 0).asInt64();

    CommentService service;
    service.createComment(postId, userId, content, parentCommentId, std::move(callback));
}

void CommentController::getComments(const drogon::HttpRequestPtr &req,
                                     std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                     int64_t postId)
{
    int page = std::stoi(req->getParameter("page", "1"));
    int pageSize = std::stoi(req->getParameter("page_size", "10"));

    CommentService service;
    service.getComments(postId, page, pageSize, std::move(callback));
}
