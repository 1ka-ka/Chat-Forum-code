#include "PostController.h"
#include "../services/PostService.h"
#include "../services/FileService.h"
#include "../utils/ResponseUtil.h"
#include <json/json.h>
#include <memory>

void PostController::createPost(const drogon::HttpRequestPtr &req,
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    int64_t userId = req->getAttribute<int64_t>("userId");

    std::string title = std::string(req->getParameter("title"));
    std::string content = std::string(req->getParameter("content"));

    auto files = req->getFiles();
    if (files.empty())
    {
        // 没有图片，直接创建帖子，image_urls 为空数组
        PostService service;
        service.createPost(userId, title, content, "[]", std::move(callback));
        return;
    }

    // ===== 多文件上传的异步处理：shared_ptr 计数器模式 =====
    // 问题：多个文件异步上传，何时才能创建帖子？
    // 方案：用 shared_ptr 共享计数器，每个文件上传完成后计数减1，归零时触发帖子创建
    auto savedPaths = std::make_shared<Json::Value>(Json::arrayValue);  // 收集所有上传成功的 URL
    auto remaining = std::make_shared<int>(files.size());               // 剩余未完成的文件数
    auto cb = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));  // 用 shared_ptr 包装 callback 以便在多个 lambda 中共享

    for (size_t i = 0; i < files.size(); ++i)
    {
        auto &file = files[i];
        std::string filename = file.getFileName();
        std::string fileContent = file.fileContent();

        FileService fileService;
        fileService.saveUploadFile(
            userId, filename, fileContent, "images",
            [userId, title, content, savedPaths, remaining, cb](
                const drogon::HttpResponsePtr &fileResp) mutable {
                // 从文件上传响应中提取 URL 并追加到数组
                auto json = fileResp->getJsonObject();
                if (json && (*json).isMember("data") && (*json)["data"].isMember("url"))
                {
                    savedPaths->append((*json)["data"]["url"].asString());
                }

                // 计数器减1，所有文件上传完成后才创建帖子
                (*remaining)--;
                if (*remaining == 0)
                {
                    Json::StreamWriterBuilder builder;
                    std::string imageUrlsStr = Json::writeString(builder, *savedPaths);

                    PostService service;
                    service.createPost(userId, title, content, imageUrlsStr, std::move(*cb));
                }
            });
    }
}

void PostController::getPostList(const drogon::HttpRequestPtr &req,
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    int64_t currentUserId = req->getAttribute<int64_t>("userId");

    // getParameter 的第二个参数是默认值，未传参时使用默认值
    int page = std::stoi(req->getParameter("page", "1"));
    int pageSize = std::stoi(req->getParameter("page_size", "10"));
    std::string search = std::string(req->getParameter("search"));

    PostService service;
    service.getPostList(page, pageSize, currentUserId, search, std::move(callback));
}

void PostController::getPostById(const drogon::HttpRequestPtr &req,
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                  int64_t postId)
{
    int64_t currentUserId = req->getAttribute<int64_t>("userId");

    PostService service;
    service.getPostById(postId, currentUserId, std::move(callback));
}
