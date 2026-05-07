#pragma once

#include <drogon/drogon.h>

// 评论控制器 - 处理帖子的评论创建和查询
class CommentController : public drogon::HttpController<CommentController>
{
public:
    METHOD_LIST_BEGIN
    // {1} 对应 postId，来自 URL 路径参数
    ADD_METHOD_TO(CommentController::createComment, "/api/posts/{1}/comments", drogon::Post, drogon::Options, "auth");
    ADD_METHOD_TO(CommentController::getComments, "/api/posts/{1}/comments", drogon::Get, drogon::Options);
    METHOD_LIST_END

    void createComment(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                       int64_t postId);

    void getComments(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                     int64_t postId);
};
