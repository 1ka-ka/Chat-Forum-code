#include "UserService.h"
#include "../utils/ResponseUtil.h"
#include "../utils/ValidationUtil.h"
#include "../utils/JwtUtil.h"
#include "../utils/ConfigUtil.h"
#include "../models/User.h"
#include <bcrypt/BCrypt.hpp>

void UserService::registerUser(const std::string &username,
                                const std::string &password,
                                const std::string &nickname,
                                Callback &&callback)
{
    // 输入校验
    if (!ValidationUtil::isValidUsername(username))
    {
        callback(ResponseUtil::badRequest("用户名格式不正确"));
        return;
    }

    if (!ValidationUtil::isValidPassword(password))
    {
        callback(ResponseUtil::badRequest("密码格式不正确"));
        return;
    }

    if (!ValidationUtil::isValidNickname(nickname))
    {
        callback(ResponseUtil::badRequest("昵称格式不正确"));
        return;
    }

    // ===== execSqlAsync 回调链 =====
    // Drogon 的数据库操作是异步的，execSqlAsync 接受：
    //   参数1: SQL 语句（? 为占位符）
    //   参数2: 成功回调（const drogon::orm::Result &）
    //   参数3: 失败回调（const drogon::orm::DrogonDbException &）
    //   参数4+: 绑定到 ? 占位符的参数值
    // 回调链通过嵌套实现：第一步的成功回调中发起第二步的异步操作

    auto dbClient = drogon::app().getDbClient();
    // 第1步：检查用户名是否已存在
    dbClient->execSqlAsync(
        "SELECT id FROM users WHERE username = ?",
        [this, username, password, nickname, callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            if (!result.empty())
            {
                callback(ResponseUtil::conflict("用户名已存在"));
                return;
            }

            // 第2步：检查昵称是否已存在
            auto db = drogon::app().getDbClient();
            db->execSqlAsync(
                "SELECT id FROM users WHERE nickname = ?",
                [this, username, password, nickname, callback = std::move(callback)]
                (const drogon::orm::Result &result) mutable {
                    if (!result.empty())
                    {
                        callback(ResponseUtil::conflict("该昵称已被使用"));
                        return;
                    }

                    // 第3步：插入新用户（密码用 bcrypt 哈希后存储）
                    std::string hashedPwd = hashPassword(password);
                    auto db = drogon::app().getDbClient();
                    db->execSqlAsync(
                        "INSERT INTO users (username, password_hash, nickname) VALUES (?, ?, ?)",
                        [this, username, nickname, callback = std::move(callback)]
                        (const drogon::orm::Result &result) mutable {
                            // 插入成功，result.insertId() 获取自增主键
                            int64_t userId = result.insertId();
                            // 生成 JWT 令牌，注册后自动登录
                            std::string token = JwtUtil::generateToken(userId, getJwtSecret(), getJwtExpiration());
                            Json::Value data;
                            data["id"] = static_cast<Json::Int64>(userId);
                            data["username"] = username;
                            data["nickname"] = nickname;
                            data["token"] = token;
                            callback(ResponseUtil::success(data, "注册成功"));
                        },
                        [callback = std::move(callback)]
                        (const drogon::orm::DrogonDbException &e) mutable {
                            callback(ResponseUtil::serverError("注册失败"));
                        },
                        username, hashedPwd, nickname);
                },
                [callback = std::move(callback)]
                (const drogon::orm::DrogonDbException &e) mutable {
                    callback(ResponseUtil::serverError("查询失败"));
                },
                nickname);
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("查询失败"));
        },
        username);
}

void UserService::login(const std::string &username,
                         const std::string &password,
                         Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "SELECT id, username, password_hash, nickname, avatar_url, created_at FROM users WHERE username = ?",
        [this, username, password, callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            if (result.empty())
            {
                // 用户名不存在时也提示"用户名或密码错误"，避免泄露用户是否存在
                callback(ResponseUtil::badRequest("用户名或密码错误"));
                return;
            }

            auto row = result[0];
            // 用 bcrypt 验证密码：将用户输入的明文与数据库中的哈希比对
            std::string storedHash = row["password_hash"].c_str();
            if (!verifyPassword(password, storedHash))
            {
                callback(ResponseUtil::badRequest("用户名或密码错误"));
                return;
            }

            // 验证通过，生成 JWT 令牌
            int64_t userId = row["id"].as<int64_t>();
            std::string token = JwtUtil::generateToken(userId, getJwtSecret(), getJwtExpiration());
            Json::Value data;
            data["id"] = static_cast<Json::Int64>(userId);
            data["username"] = std::string(row["username"].c_str());
            data["nickname"] = std::string(row["nickname"].c_str());
            data["avatar_url"] = std::string(row["avatar_url"].c_str());
            data["token"] = token;
            callback(ResponseUtil::success(data, "登录成功"));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("登录失败"));
        },
        username);
}

void UserService::getProfile(int64_t userId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "SELECT id, username, nickname, avatar_url, created_at FROM users WHERE id = ?",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            if (result.empty())
            {
                callback(ResponseUtil::notFound("用户不存在"));
                return;
            }

            auto user = User::fromResult(result[0]);
            callback(ResponseUtil::success(user.toJson()));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            LOG_ERROR << "Failed to query user profile - " << e.base().what();
            callback(ResponseUtil::serverError("查询用户信息失败"));
        },
        userId);
}

void UserService::updateProfile(int64_t userId,
                                 const std::string &nickname,
                                 const std::string &avatarUrl,
                                 Callback &&callback)
{
    if (!ValidationUtil::isValidNickname(nickname))
    {
        callback(ResponseUtil::badRequest("昵称格式不正确"));
        return;
    }

    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "UPDATE users SET nickname = ?, avatar_url = ? WHERE id = ?",
        [this, userId, callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            if (result.affectedRows() == 0)
            {
                callback(ResponseUtil::notFound("用户不存在"));
                return;
            }
            // 更新成功后，复用 getProfile 返回最新用户信息
            getProfile(userId, std::move(callback));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("更新用户信息失败"));
        },
        nickname, avatarUrl, userId);
}

void UserService::getUserById(int64_t userId, Callback &&callback)
{
    auto dbClient = drogon::app().getDbClient();
    // LEFT JOIN 统计用户发帖数，没有帖子时 post_count 为 0
    dbClient->execSqlAsync(
        "SELECT u.id, u.username, u.nickname, u.avatar_url, u.created_at, "
        "COUNT(p.id) AS post_count "
        "FROM users u LEFT JOIN posts p ON u.id = p.user_id "
        "WHERE u.id = ? GROUP BY u.id",
        [callback = std::move(callback)]
        (const drogon::orm::Result &result) mutable {
            if (result.empty())
            {
                callback(ResponseUtil::notFound("用户不存在"));
                return;
            }

            auto &row = result[0];
            Json::Value data;
            data["id"] = row["id"].as<int64_t>();
            data["username"] = std::string(row["username"].c_str());
            data["nickname"] = std::string(row["nickname"].c_str());
            data["avatar_url"] = std::string(row["avatar_url"].c_str());
            data["created_at"] = std::string(row["created_at"].c_str());
            data["post_count"] = row["post_count"].as<int>();
            callback(ResponseUtil::success(data));
        },
        [callback = std::move(callback)]
        (const drogon::orm::DrogonDbException &e) mutable {
            callback(ResponseUtil::serverError("查询用户信息失败"));
        },
        userId);
}

// 使用 bcrypt 对密码进行哈希，自动生成盐值
std::string UserService::hashPassword(const std::string &password)
{
    return BCrypt::generateHash(password);
}

// 验证明文密码与 bcrypt 哈希是否匹配
bool UserService::verifyPassword(const std::string &password, const std::string &hash)
{
    return BCrypt::validatePassword(password, hash);
}

std::string UserService::getJwtSecret()
{
    return ConfigUtil::getJwtSecret();
}

int UserService::getJwtExpiration()
{
    return ConfigUtil::getJwtExpiration();
}
