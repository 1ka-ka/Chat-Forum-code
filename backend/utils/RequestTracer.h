#pragma once

#include <drogon/drogon.h>
#include <atomic>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>

/**
 * 请求追踪工具：为每个请求生成唯一 ID，贯穿日志链路
 *
 * 原理：
 * 1. PreRoutingAdvice 中生成唯一 Request-ID
 * 2. 存入 request->setAttribute()，后续所有日志都可以读取
 * 3. PostHandlingAdvice 中将 Request-ID 写入响应头，方便客户端排查
 *
 * Request-ID 格式：时间戳-随机数-计数器
 *   例：20250520143000-a3f2-00000142
 *
 * 使用方式：在 main.cpp 的 PreRoutingAdvice 和 PostHandlingAdvice 中调用
 *
 * 当前状态：代码原型，未集成到 main.cpp
 */
class RequestTracer
{
public:
    /**
     * 生成唯一 Request-ID
     * 格式：YYYYMMDDHHmmss-XXXXXX-NNNNNNNN
     *       时间戳      随机数   原子计数器
     */
    static std::string generateRequestId()
    {
        // 时间戳部分
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y%m%d%H%M%S");

        // 随机数部分（6位十六进制）
        std::uniform_int_distribution<int> dist(0, 0xFFFFFF);
        char randomPart[8];
        std::snprintf(randomPart, sizeof(randomPart), "%06x", dist(rng_));

        // 原子计数器部分（8位十六进制）
        uint64_t counter = counter_.fetch_add(1, std::memory_order_relaxed);
        char counterPart[10];
        std::snprintf(counterPart, sizeof(counterPart), "%08llx", (unsigned long long)counter);

        return ss.str() + "-" + randomPart + "-" + counterPart;
    }

    /**
     * 从请求中获取 Request-ID（如果有的话）
     */
    static std::string getRequestId(const drogon::HttpRequestPtr &req)
    {
        auto id = req->getAttribute<std::string>("requestId");
        return id ? *id : "";
    }

    /**
     * 在 PreRoutingAdvice 中调用：生成 ID 并注入请求
     *
     * 用法：
     *   app.registerPreRoutingAdvice([](const drogon::HttpRequestPtr &req, ...) {
     *       RequestTracer::injectRequestId(req);
     *       // ... 其他逻辑
     *       accb();
     *   });
     */
    static void injectRequestId(const drogon::HttpRequestPtr &req)
    {
        std::string requestId = generateRequestId();
        req->setAttribute("requestId", std::make_shared<std::string>(requestId));

        // 注入后，后续所有日志都可以带上这个 ID
        LOG_DEBUG << "Request-ID: " << requestId
                  << " | " << std::string(req->methodString())
                  << " " << req->getPath();
    }

    /**
     * 在 PostHandlingAdvice 中调用：将 ID 写入响应头
     *
     * 用法：
     *   app.registerPostHandlingAdvice([](const drogon::HttpRequestPtr &req,
     *                                      const drogon::HttpResponsePtr &resp) {
     *       RequestTracer::attachRequestId(req, resp);
     *   });
     */
    static void attachRequestId(const drogon::HttpRequestPtr &req,
                                 const drogon::HttpResponsePtr &resp)
    {
        std::string requestId = getRequestId(req);
        if (!requestId.empty())
        {
            resp->addHeader("X-Request-ID", requestId);
        }
    }

private:
    static inline std::atomic<uint64_t> counter_{0};
    static inline std::mt19937 rng_{std::random_device{}()};
};
