// 聊天论坛后端 - 入口文件
// 基于 Drogon 框架的 HTTP/WebSocket 服务器

#include <drogon/drogon.h>
#include <signal.h>
#include "controllers/UserController.h"
#include "controllers/PostController.h"
#include "controllers/CommentController.h"
#include "controllers/LikeController.h"
#include "controllers/ChatController.h"
#include "controllers/NotificationController.h"
#include "websocket/ChatWebSocketController.h"
#include "utils/AuthMiddleware.h"
#include "utils/RateLimitMiddleware.h"    // 限流中间件
#include "utils/RequestTracer.h"          // 请求追踪

int main(int argc, char *argv[]) {
    // 支持通过命令行参数指定配置文件路径，默认使用 ../config/config.json
    std::string configPath = "../config/config.json";
    if (argc > 1) {
        configPath = argv[1];
    }

    // drogon::app() 返回全局单例应用对象，loadConfigFile 加载数据库、端口等配置
    auto &app = drogon::app();
    app.loadConfigFile(configPath);

    // ========== 注册中间件 ==========
    // 中间件在请求到达控制器之前执行，可用于鉴权、限流等
    // "auth" 是中间件名称，路由中通过此名称引用
    app.registerMiddleware<AuthMiddleware>("auth");
    app.registerMiddleware<RateLimitMiddleware>("rate_limit");

    // ========== 注册控制器 ==========
    // Drogon 的 HttpController 通过模板参数注册，框架会自动扫描 METHOD_LIST 中的路由
    app.registerController<UserController>();
    app.registerController<PostController>();
    app.registerController<CommentController>();
    app.registerController<LikeController>();
    app.registerController<ChatController>();
    app.registerController<ChatWebSocketController>();
    app.registerController<NotificationController>();

    // ========== 健康检查端点 ==========
    // 用于部署时探活检测，返回简单的 ok 状态
    app.registerHandler(
        "/api/health",
        [](const drogon::HttpRequestPtr &req,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
            Json::Value data;
            data["status"] = "ok";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(data);
            callback(resp);
        },
        {drogon::Get});

    // ========== PreRoutingAdvice：OPTIONS 预检 + 请求追踪注入 ==========
    // PreRoutingAdvice 在路由匹配之前执行，适合做 CORS 预检等全局拦截
    app.registerPreRoutingAdvice(
        [](const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&acb,
           drogon::AdviceChainCallback &&accb) {
            // 请求追踪：为每个请求生成唯一 ID 并注入上下文
            RequestTracer::injectRequestId(req);

            // 浏览器跨域请求会先发 OPTIONS 预检，直接返回 204 避免进入业务逻辑
            if (req->method() == drogon::Options) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k204NoContent);
                resp->addHeader("Access-Control-Allow-Origin", "*");
                resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
                resp->addHeader("Access-Control-Max-Age", "86400");  // 预检结果缓存24小时
                acb(resp);  // acb：直接返回响应，不继续路由链
                return;
            }
            accb();  // accb：继续执行后续路由链
        });

    // ========== PostHandlingAdvice：CORS 响应头 + 请求追踪响应头 ==========
    // PostHandlingAdvice 在响应返回之前执行，用于统一添加响应头
    app.registerPostHandlingAdvice(
        [](const drogon::HttpRequestPtr &req, const drogon::HttpResponsePtr &resp) {
            // 请求追踪：将 Request-ID 写入响应头
            RequestTracer::attachRequestId(req, resp);

            // 为所有响应添加 CORS 头，允许前端跨域访问
            resp->addHeader("Access-Control-Allow-Origin", "*");
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
            resp->addHeader("Access-Control-Allow-Credentials", "true");
        });

    // 监听所有网卡的 8080 端口
    app.addListener("0.0.0.0", 8080);

    LOG_INFO << "ChatForum server starting...";

    // ========== 优雅关闭：注册信号处理 ==========
    // 捕获 SIGTERM（kill 命令）和 SIGINT（Ctrl+C），调用 app().quit() 优雅退出
    // 优雅退出会等待正在处理的请求完成后再关闭
    signal(SIGTERM, [](int) {
        LOG_INFO << "Received SIGTERM, graceful shutdown...";
        drogon::app().quit();
    });
    signal(SIGINT, [](int) {
        LOG_INFO << "Received SIGINT, graceful shutdown...";
        drogon::app().quit();
    });

    LOG_INFO << "ChatForum server running on http://0.0.0.0:8080";
    app.run();  // 启动事件循环，阻塞直到调用 quit()
}
