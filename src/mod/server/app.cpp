//
// Created by 21628 on 2025/9/12.
//

#include "app.h"
#include "httplib.h"
#include "mod/VoxesisLevilamina.h"
#include <asptlb.h>
#include <filesystem>
#include <iostream>

using namespace std;

void app::init(int64_t port) { serverPort = port; }

void app::run(voxesis_levilamina::VoxesisLevilamina* mod) {
    // 检查并创建www目录
    if (!std::filesystem::exists("./www")) {
        if (!std::filesystem::create_directory("./www")) {
            {
                std::lock_guard<std::mutex> lock(logMutex);
                logQueue.push({LogLevel::LOG_ERROR, "无法创建www目录"});
                logCondition.notify_one();
            }
            return;
        }
        {
            std::lock_guard<std::mutex> lock(logMutex);
            logQueue.push({LogLevel::LOG_INFO, "已创建www目录"});
            logCondition.notify_one();
        }
    }

    auto ret = server.set_mount_point("/", "./www");
    if (!ret) {
        {
            std::lock_guard<std::mutex> lock(logMutex);
            logQueue.push({LogLevel::LOG_ERROR, "指定的目录不存在"});
            logCondition.notify_one();
        }
        return;
    }

    server.Get("/hello", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    // 在新线程中启动服务器，避免阻塞主线程
    isRunning    = true;
    serverThread = std::thread([this, mod]() {
        mod->getSelf().getLogger().info("正在尝试启动服务器，端口: " + std::to_string(serverPort));
        // 强制刷新日志缓冲区，确保消息立即显示
        std::cout << std::flush;

        this->server.listen("0.0.0.0", static_cast<int>(this->serverPort));
    });
}

void app::stop() {
    if (isRunning) {
        server.stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }
        isRunning = false;

        {
            std::lock_guard<std::mutex> lock(logMutex);
            logQueue.push({LogLevel::LOG_INFO, "服务器已停止"});
            logCondition.notify_one();
        }
    }
}

void app::processLogMessages(voxesis_levilamina::VoxesisLevilamina* mod) {
    std::unique_lock<std::mutex> lock(logMutex);
    while (!logQueue.empty()) {
        LogMessage logMsg = logQueue.front();
        logQueue.pop();

        lock.unlock();
        switch (logMsg.level) {
        case LogLevel::LOG_INFO:
            mod->getSelf().getLogger().info(logMsg.message);
            break;
        case LogLevel::LOG_ERROR:
            mod->getSelf().getLogger().error(logMsg.message);
            break;
        }
        lock.lock();
    }
}
