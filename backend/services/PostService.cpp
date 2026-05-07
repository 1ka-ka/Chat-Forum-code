#include "PostService.h"
#include "../utils/ResponseUtil.h"
#include "../utils/ValidationUtil.h"
#include "../models/Post.h"
#include <drogon/drogon.h>

void PostService::createPost(int64_t userId,
                              const std::string &title,
                              const std::string &content,
                              const std::string &imageUrls,
                              Callback &&callback)
{
    if (!ValidationUtil::isValidTitle(title))
    {
        callback(ResponseUtil::badRequest("标题格式不正确"));
        return;
    }

    if (content.empty())
    {
        callback(ResponseUtil::badRequest("内容不能为空"));
        return;
    }

    auto dbClient = drogon::app().getDbClient();
    // 插入帖子后，再查询完整信息（含用户昵称和头像）返回给前端
    dbClient->execSqlAsync(
        "INSERT INTO posts (user_id, title, content, image_urls) VALUES (?, ?, ?, ?)",
        [this, userId, callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            int64_t postId = result.insertId();
            auto db = drogon::app().getDbClient();
            // 插入后再 JOIN 查询，获取发帖人的昵称和头像
            dbClient->execSqlAsync(
                "SELECT p.id, p.user_id, p.title, p.content, p.image_urls, "
                "p.like_count, p.comment_count, p.created_at, p.updated_at, "
                "u.nickname, u.avatar_url "
                "FROM posts p LEFT JOIN users u ON p.user_id = u.id WHERE p.id = ?",
                [callback = std::move(callback)]
                (const drogon::orm::Result &result) mutable {
                    if (result.empty())
                    {
                        callback(ResponseUtil::notFound("帖子不存在"));
                        return;
                    }

                    auto &row = result[0];
                    Json::Value data;
                    data["id"] = row["id"].as<int64_t>();
                    data["user_id"] = row["user_id"].as<int64_t>();
                    data["title"] = std::string(row["title"].c_str());
                    data["content"] = std::string(row["content"].c_str());
                    data["image_urls"] = row["image_urls"].isNull()
                                            ? Json::Value(Json::arrayValue)
                                            : row["image_urls"];
                    data["like_count"] = row["like_count"].as<int>();
                    data["comment_count"] = row["comment_count"].as<int>();
                    data["created_at"] = std::string(row["created_at"].c_str());
                    data["nickname"] = std::string(row["nickname"].c_str());
                    data["avatar_url"] = std::string(row["avatar_url"].c_str());
                    callback(ResponseUtil::success(data, "发布成功"));
                },
                [callback = std::move(callback)]
                (const drogon::orm::DrogonDbException &e) mutable {
                    callback(ResponseUtil::serverError("查询帖子失败"));
                },
                postId);
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("发布帖子失败"));
        },
        userId, title, content,
        imageUrls.empty() ? std::string("NULL") : imageUrls);  // 空图片列表传 NULL
}

void PostService::getPostList(int page, int pageSize, int64_t currentUserId,
                               const std::string &search, Callback &&callback)
{
    // 分页偏移量计算：第1页 offset=0，第2页 offset=pageSize，以此类推
    int offset = (page - 1) * pageSize;
    auto dbClient = drogon::app().getDbClient();

    bool hasSearch = !search.empty();
    // 根据是否有搜索关键词选择不同的 SQL
    std::string countSql = hasSearch
        ? "SELECT COUNT(*) AS total FROM posts WHERE title LIKE ?"
        : "SELECT COUNT(*) AS total FROM posts";

    if (hasSearch)
    {
        // LIKE 模糊搜索：前后加 % 实现包含匹配
        std::string keyword = "%" + search + "%";
        dbClient->execSqlAsync(
            countSql,
            [this, currentUserId, page, pageSize, keyword, callback = std::move(callback)]
            (const drogon::orm::Result &countResult) mutable {
                int total = countResult[0]["total"].as<int>();
                // 向上取整计算总页数
                int totalPages = (total + pageSize - 1) / pageSize;

                auto db = drogon::app().getDbClient();
                // EXISTS 子查询判断当前用户是否已点赞该帖子
                dbClient->execSqlAsync(
                    "SELECT p.id, p.user_id, p.title, p.content, p.image_urls, "
                    "p.like_count, p.comment_count, p.created_at, "
                    "u.nickname, u.avatar_url, "
                    "EXISTS(SELECT 1 FROM likes WHERE post_id = p.id AND user_id = ?) AS is_liked "
                    "FROM posts p LEFT JOIN users u ON p.user_id = u.id "
                    "WHERE p.title LIKE ? ORDER BY p.created_at DESC LIMIT ? OFFSET ?",
                    [currentUserId, page, pageSize, total, totalPages, callback = std::move(callback)]
                    (const drogon::orm::Result &result) mutable {
                        Json::Value items(Json::arrayValue);
                        for (size_t i = 0; i < result.size(); ++i)
                        {
                            Json::Value row;
                            row["id"] = result[i]["id"].as<int64_t>();
                            row["user_id"] = result[i]["user_id"].as<int64_t>();
                            row["title"] = std::string(result[i]["title"].c_str());

                            // 列表页只显示内容前100字，避免传输过多数据
                            std::string contentStr = std::string(result[i]["content"].c_str());
                            if (contentStr.length() > 100)
                            {
                                contentStr = contentStr.substr(0, 100) + "...";
                            }
                            row["content"] = contentStr;

                            row["image_urls"] = result[i]["image_urls"].isNull()
                                                    ? Json::Value(Json::arrayValue)
                                                    : result[i]["image_urls"];
                            row["like_count"] = result[i]["like_count"].as<int>();
                            row["comment_count"] = result[i]["comment_count"].as<int>();
                            row["created_at"] = std::string(result[i]["created_at"].c_str());
                            row["nickname"] = std::string(result[i]["nickname"].c_str());
                            row["avatar_url"] = std::string(result[i]["avatar_url"].c_str());
                            row["is_liked"] = result[i]["is_liked"].as<int>() > 0;  // EXISTS 返回 0/1
                            items.append(row);
                        }
                        Json::Value data;
                        data["items"] = items;
                        data["total"] = total;
                        data["page"] = page;
                        data["page_size"] = pageSize;
                        data["total_pages"] = totalPages;
                        callback(ResponseUtil::success(data));
                    },
                    [callback = std::move(callback)]
                    (const drogon::orm::DrogonDbException &e) mutable {
                        callback(ResponseUtil::serverError("查询帖子列表失败"));
                    },
                    currentUserId, keyword, pageSize, offset);
            },
            [callback = std::move(callback)]
            (const drogon::orm::DrogonDbException &e) mutable {
                callback(ResponseUtil::serverError("查询帖子数量失败"));
            },
            keyword);
    }
    else
    {
        dbClient->execSqlAsync(
            countSql,
            [this, currentUserId, page, pageSize, callback = std::move(callback)]
            (const drogon::orm::Result &countResult) mutable {
                int total = countResult[0]["total"].as<int>();
                int totalPages = (total + pageSize - 1) / pageSize;

                auto db = drogon::app().getDbClient();
                db->execSqlAsync(
                    "SELECT p.id, p.user_id, p.title, p.content, p.image_urls, "
                    "p.like_count, p.comment_count, p.created_at, "
                    "u.nickname, u.avatar_url, "
                    "EXISTS(SELECT 1 FROM likes WHERE post_id = p.id AND user_id = ?) AS is_liked "
                    "FROM posts p LEFT JOIN users u ON p.user_id = u.id "
                    "ORDER BY p.created_at DESC LIMIT ? OFFSET ?",
                    [currentUserId, page, pageSize, total, totalPages, callback = std::move(callback)]
                    (const drogon::orm::Result &result) mutable {
                        Json::Value items(Json::arrayValue);
                        for (size_t i = 0; i < result.size(); ++i)
                        {
                            Json::Value row;
                            row["id"] = result[i]["id"].as<int64_t>();
                            row["user_id"] = result[i]["user_id"].as<int64_t>();
                            row["title"] = std::string(result[i]["title"].c_str());

                            // 列表页只显示内容前100字，避免传输过多数据
                            std::string contentStr = std::string(result[i]["content"].c_str());
                            if (contentStr.length() > 100)
                            {
                                contentStr = contentStr.substr(0, 100) + "...";
                            }
                            row["content"] = contentStr;

                            row["image_urls"] = result[i]["image_urls"].isNull()
                                                    ? Json::Value(Json::arrayValue)
                                                    : result[i]["image_urls"];
                            row["like_count"] = result[i]["like_count"].as<int>();
                            row["comment_count"] = result[i]["comment_count"].as<int>();
                            row["created_at"] = std::string(result[i]["created_at"].c_str());
                            row["nickname"] = std::string(result[i]["nickname"].c_str());
                            row["avatar_url"] = std::string(result[i]["avatar_url"].c_str());
                            row["is_liked"] = result[i]["is_liked"].as<int>() > 0;  // EXISTS 返回 0/1
                            items.append(row);
                        }
                        Json::Value data;
                        data["items"] = items;
                        data["total"] = total;
                        data["page"] = page;
                        data["page_size"] = pageSize;
                        data["total_pages"] = totalPages;
                        callback(ResponseUtil::success(data));
                    },
                    [callback = std::move(callback)]
                    (const drogon::orm::DrogonDbException &e) mutable {
                        callback(ResponseUtil::serverError("查询帖子列表失败"));
                    },
                    currentUserId, pageSize, offset);
            },
            [callback = std::move(callback)]
            (const drogon::orm::DrogonDbException &e) mutable {
                callback(ResponseUtil::serverError("查询帖子数量失败"));
            });
    }
}

void PostService::getPostById(int64_t postId, int64_t currentUserId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    // EXISTS 子查询：判断当前用户是否已点赞，避免额外查询
    dbClient->execSqlAsync(
        "SELECT p.id, p.user_id, p.title, p.content, p.image_urls, "
        "p.like_count, p.comment_count, p.created_at, p.updated_at, "
        "u.nickname, u.avatar_url, "
        "EXISTS(SELECT 1 FROM likes WHERE post_id = p.id AND user_id = ?) AS is_liked "
        "FROM posts p LEFT JOIN users u ON p.user_id = u.id WHERE p.id = ?",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            if (result.empty())
            {
                callback(ResponseUtil::notFound("帖子不存在"));
                return;
            }

            auto post = Post::fromResult(result[0]);
            Json::Value data = post.toJson();
            // 补充 JOIN 查询的关联字段
            data["nickname"] = std::string(result[0]["nickname"].c_str());
            data["avatar_url"] = std::string(result[0]["avatar_url"].c_str());
            data["is_liked"] = result[0]["is_liked"].as<int>() > 0;
            callback(ResponseUtil::success(data));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            LOG_ERROR << "Failed to query post detail - " << e.base().what();
            callback(ResponseUtil::serverError("查询帖子详情失败"));
        },
        currentUserId, postId);
}
