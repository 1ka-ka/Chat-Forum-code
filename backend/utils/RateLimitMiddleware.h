#pragma once

#include <drogon/drogon.h>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <string>

/**
 * 限流中间件：令牌桶算法
 *
 * 原理：每个客户端 IP 有一个"桶"，桶里最多放 maxTokens 个令牌。
 * 每秒往桶里补充 refillRate 个令牌。每次请求消耗 1 个令牌。
 * 桶空了就拒绝请求，返回 429 Too Many Requests。
 *
 * 为什么用令牌桶而不是简单计数：
 * - 简单计数：1秒内允许10次，第0.9秒来了10次，第1.1秒又来10次 → 0.2秒内20次
 * - 令牌桶：桶里有存量，允许短时间突发，但长期平均速率受控
 *
 * 使用方式：在需要限流的路由上添加 "rate_limit" 中间件
 *   ADD_METHOD_TO(UserController::login, "/api/auth/login", drogon::Post, "rate_limit")
 *
 * 当前状态：代码原型，未注册到 main.cpp
 */
class RateLimitMiddleware : public drogon::Middleware<RateLimitMiddleware>
{
public:
    RateLimitMiddleware() = default;

    void invoke(const drogon::HttpRequestPtr &req,
                drogon::MiddlewareNextCallback &&nextCb,
                drogon::MiddlewareCallback &&mcb) override
    {
        std::string clientIp = std::string(req->getPeerAddr().toIp());

        // 在锁内完成令牌消耗判断，避免竞态条件
        if (!tryConsumeToken(clientIp))
        {
            // 令牌不足，拒绝请求
            Json::Value data;
            data["error"] = "Too many requests";
            data["retry_after"] = 1;  // 建议等待秒数
            auto resp = drogon::HttpResponse::newHttpJsonResponse(data);
            resp->setStatusCode(drogon::k429TooManyRequests);
            resp->addHeader("Retry-After", "1");
            mcb(resp);
            return;
        }

        // 令牌充足，放行
        nextCb([mcb = std::move(mcb)](const drogon::HttpResponsePtr &resp) {
            mcb(resp);
        });
    }

private:
    /**
     * 令牌桶结构体
     *
     * 每个客户端 IP 对应一个令牌桶，包含：
     * - tokens: 当前令牌数量（可以是小数，因为补充是按时间比例的）
     * - lastRefill: 上次补充令牌的时间点
     * - maxTokens: 桶容量，即最大令牌数（也决定了最大突发量）
     * - refillRate: 每秒补充的令牌数（决定了长期平均速率）
     *
     * 例：maxTokens=10, refillRate=1.0 表示：
     *   - 最多允许连续 10 个请求（突发）
     *   - 长期平均每秒 1 个请求
     *   - 桶空后，每秒恢复 1 个令牌，10 秒后满
     */
    struct TokenBucket
    {
        double tokens = 0;                                   // 当前令牌数
        std::chrono::steady_clock::time_point lastRefill;    // 上次补充时间
        int maxTokens = 10;                                  // 桶容量
        double refillRate = 1.0;                             // 每秒补充令牌数

        /**
         * 尝试消耗 1 个令牌
         *
         * 步骤：
         * 1. 计算距离上次补充经过的时间
         * 2. 按比例补充令牌：补充量 = 经过秒数 × 每秒补充速率
         * 3. 令牌数不超过桶容量
         * 4. 如果令牌数 >= 1，消耗 1 个并返回 true；否则返回 false
         */
        bool tryConsume()
        {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - lastRefill).count();

            // 补充令牌：经过的秒数 * 每秒补充速率
            tokens = std::min(static_cast<double>(maxTokens), tokens + elapsed * refillRate);
            lastRefill = now;

            if (tokens >= 1.0)
            {
                tokens -= 1.0;
                return true;
            }
            return false;
        }
    };

    /**
     * 尝试为指定 IP 消耗一个令牌
     *
     * 整个操作在锁内完成，确保线程安全：
     * 1. 查找或创建该 IP 的令牌桶
     * 2. 在桶上执行 tryConsume
     * 3. 返回结果
     *
     * 注意：不能先 getOrCreateBucket 返回引用再 tryConsume，
     * 因为那样锁已经释放，其他线程可能同时修改同一个桶。
     */
    bool tryConsumeToken(const std::string &ip)
    {
        std::lock_guard<std::mutex> lock(bucketsMutex_);

        auto it = buckets_.find(ip);
        if (it == buckets_.end())
        {
            // 首次访问的 IP，创建满令牌的桶
            TokenBucket newBucket;
            newBucket.tokens = newBucket.maxTokens;
            newBucket.lastRefill = std::chrono::steady_clock::now();
            buckets_[ip] = newBucket;
            it = buckets_.find(ip);
        }

        return it->second.tryConsume();
    }

    // 注意：生产环境应该定期清理长时间无请求的 IP，避免内存泄漏
    // 简单做法：设一个定时器，每隔 60 秒清理 lastRefill 超过 300 秒的条目
    // Drogon 提供了 app().getLoop()->runEvery(60, [](){ ... }) 来实现定时任务

    // C++17 inline 静态成员：无需在 .cpp 中单独定义
    static inline std::unordered_map<std::string, TokenBucket> buckets_;
    static inline std::mutex bucketsMutex_;
};
