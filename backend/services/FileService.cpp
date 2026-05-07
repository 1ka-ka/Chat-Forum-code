#include "FileService.h"
#include "../utils/ResponseUtil.h"
#include "../utils/ValidationUtil.h"
#include <fstream>
#include <chrono>
#include <random>
#include <filesystem>
#include <sstream>

void FileService::saveUploadFile(int64_t userId,
                                  const std::string &filename,
                                  const std::string &fileContent,
                                  const std::string &uploadDir,
                                  Callback &&callback)
{
    if (!isValidImageExtension(filename))
    {
        callback(ResponseUtil::badRequest("不支持的图片格式，仅支持 jpg/jpeg/png/gif"));
        return;
    }

    if (fileContent.empty())
    {
        callback(ResponseUtil::badRequest("文件内容不能为空"));
        return;
    }

    // 文件大小限制：5MB
    const size_t MAX_FILE_SIZE = 5 * 1024 * 1024;
    if (fileContent.size() > MAX_FILE_SIZE)
    {
        callback(ResponseUtil::badRequest("文件大小不能超过5MB"));
        return;
    }

    // 生成唯一文件名：用户ID_时间戳_随机数.扩展名
    // 避免文件名冲突和恶意文件名注入
    size_t dotPos = filename.find_last_of('.');
    std::string ext = (dotPos != std::string::npos) ? filename.substr(dotPos) : "";

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    int randomNum = dis(gen);

    std::ostringstream oss;
    oss << userId << "_" << timestamp << "_" << randomNum << ext;
    std::string uniqueFilename = oss.str();

    // 如果上传目录不存在则创建
    std::filesystem::path dirPath(uploadDir);
    if (!std::filesystem::exists(dirPath))
    {
        std::filesystem::create_directories(dirPath);
    }

    std::filesystem::path filePath = dirPath / uniqueFilename;

    // 将文件写入操作提交到业务线程池，避免阻塞 IO 线程
    // Drogon 的 IO 线程负责处理网络请求，磁盘 IO 是慢操作不应阻塞 IO 线程
    ThreadPool::instance().submit([filePath, fileContent, uniqueFilename, callback = std::move(callback)] {
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile)
        {
            callback(ResponseUtil::serverError("文件保存失败"));
            return;
        }

        outFile.write(fileContent.data(), fileContent.size());
        outFile.close();

        Json::Value data;
        data["filename"] = uniqueFilename;
        data["url"] = "/uploads/images/" + uniqueFilename;
        callback(ResponseUtil::success(data, "上传成功"));
    });
}

bool FileService::isValidImageExtension(const std::string &filename)
{
    return ValidationUtil::isValidImageExt(filename);
}
