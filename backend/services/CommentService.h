#pragma once

#include <string>
#include <functional>
#include <drogon/drogon.h>

// 评论服务层 - 封装评论相关的业务逻辑
class CommentService
{
public:
    using Callback = std::function<void(const drogon::HttpResponsePtr &)>;

    void createComment(int64_t postId,
                       int64_t userId,
                       const std::string &content,
                       int64_t parentCommentId,
                       Callback &&callback);

    void getComments(int64_t postId, int page, int pageSize, Callback &&callback);
};
