#pragma once

#include <drogon/drogon.h>

// 帖子控制器 - 处理帖子的创建和查询
class PostController : public drogon::HttpController<PostController>
{
public:
    METHOD_LIST_BEGIN
    // 创建帖子需要登录（"auth" 中间件）
    ADD_METHOD_TO(PostController::createPost, "/api/posts", drogon::Post, drogon::Options, "auth");
    ADD_METHOD_TO(PostController::getPostList, "/api/posts", drogon::Get, drogon::Options);
    ADD_METHOD_TO(PostController::getPostById, "/api/posts/{1}", drogon::Get, drogon::Options);
    METHOD_LIST_END

    void createPost(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getPostList(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getPostById(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                     int64_t postId);
};
