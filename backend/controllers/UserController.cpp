#include "UserController.h"
#include "../services/UserService.h"
#include "../services/FileService.h"
#include "../utils/ResponseUtil.h"

void UserController::registerUser(const drogon::HttpRequestPtr &req,
                                   std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    // 从请求体解析 JSON
    auto json = req->getJsonObject();
    if (!json)
    {
        callback(ResponseUtil::badRequest("请求体格式错误"));
        return;
    }

    // 从 JSON 中提取注册参数
    std::string username = (*json).get("username", "").asString();
    std::string password = (*json).get("password", "").asString();
    std::string nickname = (*json).get("nickname", "").asString();

    // 将业务逻辑委托给 Service 层，callback 通过 std::move 传递保证唯一所有权
    UserService service;
    service.registerUser(username, password, nickname, std::move(callback));
}

void UserController::login(const drogon::HttpRequestPtr &req,
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto json = req->getJsonObject();
    if (!json)
    {
        callback(ResponseUtil::badRequest("请求体格式错误"));
        return;
    }

    std::string username = (*json).get("username", "").asString();
    std::string password = (*json).get("password", "").asString();

    UserService service;
    service.login(username, password, std::move(callback));
}

void UserController::getProfile(const drogon::HttpRequestPtr &req,
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    // getAttribute 从请求上下文中获取中间件注入的数据
    // AuthMiddleware 验证 JWT 后会将 userId 存入请求属性
    int64_t userId = req->getAttribute<int64_t>("userId");

    UserService service;
    service.getProfile(userId, std::move(callback));
}

void UserController::updateProfile(const drogon::HttpRequestPtr &req,
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    int64_t userId = req->getAttribute<int64_t>("userId");

    // getParameter 从表单数据中获取参数（multipart/form-data 或 URL 参数）
    std::string nickname = std::string(req->getParameter("nickname"));
    std::string avatarUrl;

    // 检查是否有上传的头像文件
    auto files = req->getFiles();
    if (!files.empty())
    {
        auto &file = files[0];
        std::string filename = file.getFileName();
        std::string fileContent = file.fileContent();

        // 先保存文件，再更新用户信息——使用回调链串联两个异步操作
        FileService fileService;
        fileService.saveUploadFile(
            userId, filename, fileContent, "avatars",
            [this, userId, nickname, callback = std::move(callback)](
                const drogon::HttpResponsePtr &fileResp) mutable {
                // 从文件上传响应中提取保存后的 URL
                auto json = fileResp->getJsonObject();
                std::string savedPath;
                if (json && (*json).isMember("data") && (*json)["data"].isMember("url"))
                {
                    savedPath = (*json)["data"]["url"].asString();
                }

                // 文件保存成功后，再调用 UserService 更新用户信息
                UserService service;
                service.updateProfile(userId, nickname, savedPath, std::move(callback));
            });
        return;  // 文件上传走异步回调，直接返回
    }

    // 没有上传文件，直接更新昵称
    UserService service;
    service.updateProfile(userId, nickname, avatarUrl, std::move(callback));
}

void UserController::getUserById(const drogon::HttpRequestPtr &req,
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                  int64_t userId)
{
    // userId 来自 URL 路径参数 {1}
    UserService service;
    service.getUserById(userId, std::move(callback));
}
