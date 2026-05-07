#include "ResponseUtil.h"

// 内部辅助函数：构造统一格式的 JSON 响应
// code: 业务状态码，message: 提示信息，data: 业务数据，statusCode: HTTP 状态码
static drogon::HttpResponsePtr makeResponse(int code, const std::string &message, const Json::Value &data, drogon::HttpStatusCode statusCode) {
    Json::Value root;
    root["code"] = code;
    root["message"] = message;
    root["data"] = data;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    resp->setStatusCode(statusCode);
    return resp;
}

drogon::HttpResponsePtr ResponseUtil::success(const Json::Value &data, const std::string &message) {
    return makeResponse(200, message, data, drogon::k200OK);
}

drogon::HttpResponsePtr ResponseUtil::badRequest(const std::string &message) {
    return makeResponse(400, message, Json::Value(), drogon::k400BadRequest);
}

drogon::HttpResponsePtr ResponseUtil::unauthorized(const std::string &message) {
    return makeResponse(401, message, Json::Value(), drogon::k401Unauthorized);
}

drogon::HttpResponsePtr ResponseUtil::notFound(const std::string &message) {
    return makeResponse(404, message, Json::Value(), drogon::k404NotFound);
}

drogon::HttpResponsePtr ResponseUtil::conflict(const std::string &message) {
    return makeResponse(409, message, Json::Value(), drogon::k409Conflict);
}

drogon::HttpResponsePtr ResponseUtil::serverError(const std::string &message) {
    return makeResponse(500, message, Json::Value(), drogon::k500InternalServerError);
}
