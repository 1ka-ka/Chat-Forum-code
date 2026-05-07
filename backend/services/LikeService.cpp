#include "LikeService.h"
#include "../utils/ResponseUtil.h"
#include <drogon/drogon.h>

// ===== 点赞切换逻辑：先查再增删 =====
// 1. 查询 likes 表中是否已有该用户的点赞记录
// 2. 如果有 → 删除记录（取消点赞），like_count - 1
// 3. 如果没有 → 插入记录（点赞），like_count + 1
void LikeService::toggleLike(int64_t postId, int64_t userId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();

    // 第1步：查询是否已点赞
    dbClient->execSqlAsync(
        "SELECT id FROM likes WHERE post_id = ? AND user_id = ?",
        [postId, userId, callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            auto db = drogon::app().getDbClient();
            if (!result.empty())
            {
                // 已点赞 → 取消点赞
                int64_t likeId = result[0]["id"].as<int64_t>();
                db->execSqlAsync(
                    "DELETE FROM likes WHERE id = ?",
                    [postId, callback = std::move(callback)]
                    (const drogon::orm::Result &) mutable {
                        auto db2 = drogon::app().getDbClient();
                        // GREATEST(like_count - 1, 0)：防止 like_count 变为负数
                        // 并发场景下可能出现多次减1的情况，GREATEST 保证最小为0
                        db2->execSqlAsync(
                            "UPDATE posts SET like_count = GREATEST(like_count - 1, 0) WHERE id = ?",
                            [postId, callback = std::move(callback)]
                            (const drogon::orm::Result &) mutable {
                                auto db3 = drogon::app().getDbClient();
                                db3->execSqlAsync(
                                    "SELECT like_count FROM posts WHERE id = ?",
                                    [callback = std::move(callback)]
                                    (const drogon::orm::Result &r) mutable {
                                        Json::Value data;
                                        data["is_liked"] = false;
                                        data["like_count"] = r.empty() ? 0 : r[0]["like_count"].as<int>();
                                        callback(ResponseUtil::success(data, "取消点赞成功"));
                                    },
                                    [callback = std::move(callback)]
                                    (const drogon::orm::DrogonDbException &) mutable {
                                        callback(ResponseUtil::serverError("操作失败"));
                                    },
                                    postId);
                            },
                            [](const drogon::orm::DrogonDbException &e) {
                                LOG_ERROR << "Failed to update like count (unlike) - " << e.base().what();
                            },
                            postId);
                    },
                    [callback = std::move(callback)]
                    (const drogon::orm::DrogonDbException &) mutable {
                        callback(ResponseUtil::serverError("取消点赞失败"));
                    },
                    likeId);
            }
            else
            {
                // 未点赞 → 添加点赞
                db->execSqlAsync(
                    "INSERT INTO likes (post_id, user_id) VALUES (?, ?)",
                    [postId, callback = std::move(callback)]
                    (const drogon::orm::Result &) mutable {
                        auto db2 = drogon::app().getDbClient();
                        db2->execSqlAsync(
                            "UPDATE posts SET like_count = like_count + 1 WHERE id = ?",
                            [postId, callback = std::move(callback)]
                            (const drogon::orm::Result &) mutable {
                                auto db3 = drogon::app().getDbClient();
                                db3->execSqlAsync(
                                    "SELECT like_count FROM posts WHERE id = ?",
                                    [callback = std::move(callback)]
                                    (const drogon::orm::Result &r) mutable {
                                        Json::Value data;
                                        data["is_liked"] = true;
                                        data["like_count"] = r.empty() ? 0 : r[0]["like_count"].as<int>();
                                        callback(ResponseUtil::success(data, "点赞成功"));
                                    },
                                    [callback = std::move(callback)]
                                    (const drogon::orm::DrogonDbException &) mutable {
                                        callback(ResponseUtil::serverError("操作失败"));
                                    },
                                    postId);
                            },
                            [](const drogon::orm::DrogonDbException &e) {
                                LOG_ERROR << "Failed to update like count (like) - " << e.base().what();
                            },
                            postId);
                    },
                    [callback = std::move(callback)]
                    (const drogon::orm::DrogonDbException &) mutable {
                        callback(ResponseUtil::serverError("点赞失败"));
                    },
                    postId, userId);
            }
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &) mutable {
            callback(ResponseUtil::serverError("查询点赞状态失败"));
        },
        postId, userId);
}

void LikeService::getLikeUsers(int64_t postId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "SELECT u.id, u.nickname, u.avatar_url "
        "FROM likes l LEFT JOIN users u ON l.user_id = u.id "
        "WHERE l.post_id = ? ORDER BY l.created_at DESC",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            Json::Value items(Json::arrayValue);
            for (size_t i = 0; i < result.size(); ++i)
            {
                Json::Value row;
                row["id"] = result[i]["id"].as<int64_t>();
                row["nickname"] = std::string(result[i]["nickname"].c_str());
                row["avatar_url"] = std::string(result[i]["avatar_url"].c_str());
                items.append(row);
            }
            callback(ResponseUtil::success(items));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &) mutable {
            callback(ResponseUtil::serverError("查询点赞用户失败"));
        },
        postId);
}
