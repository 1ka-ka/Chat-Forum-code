#pragma once

#include <drogon/drogon.h>
#include <string>

// 响应工具类 - 统一 API 响应格式
// 所有接口返回统一的 JSON 结构：{ "code": int, "message": string, "data": object }
// 前端根据 code 判断请求是否成功，message 提供提示信息，data 携带业务数据
class ResponseUtil {
public:
    static drogon::HttpResponsePtr success(const Json::Value &data = Json::Value(), const std::string &message = "操作成功");
    static drogon::HttpResponsePtr badRequest(const std::string &message);     // 400
    static drogon::HttpResponsePtr unauthorized(const std::string &message = "未授权");  // 401
    static drogon::HttpResponsePtr notFound(const std::string &message);       // 404
    static drogon::HttpResponsePtr conflict(const std::string &message);       // 409
    static drogon::HttpResponsePtr serverError(const std::string &message);    // 500
};
