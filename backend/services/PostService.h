#pragma once

#include <string>
#include <functional>
#include <drogon/drogon.h>

// 帖子服务层 - 封装帖子相关的业务逻辑
class PostService
{
public:
    using Callback = std::function<void(const drogon::HttpResponsePtr &)>;

    void createPost(int64_t userId,
                    const std::string &title,
                    const std::string &content,
                    const std::string &imageUrls,
                    Callback &&callback);

    void getPostList(int page, int pageSize, int64_t currentUserId,
                     const std::string &search, Callback &&callback);

    void getPostById(int64_t postId, int64_t currentUserId, Callback &&callback);
};
