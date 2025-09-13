//
// Created by 21628 on 2025/9/12.
//

#include "httplib.h"
#include <condition_variable>
#include <functional>
#include <ll/api/mod/NativeMod.h>
#include <mutex>
#include <queue>
#include <thread>

#ifndef VOXESIS_LEVILAMINA_APP_H
#define VOXESIS_LEVILAMINA_APP_H

namespace voxesis_levilamina {
class VoxesisLevilamina;
}

// 定义日志级别
enum class LogLevel { LOG_INFO, LOG_ERROR };

// 定义日志消息结构
struct LogMessage {
    LogLevel    level;
    std::string message;
};

class app {
public:
    void init(int64_t port);

    void run(voxesis_levilamina::VoxesisLevilamina* mod);

    void stop();

    // 处理日志消息的方法
    void processLogMessages(voxesis_levilamina::VoxesisLevilamina* mod);

private:
    httplib::Server server;
    int64_t         serverPort;
    std::thread     serverThread;
    bool            isRunning = false;

    // 日志消息队列
    std::queue<LogMessage>  logQueue;
    std::mutex              logMutex;
    std::condition_variable logCondition;
};


#endif // VOXESIS_LEVILAMINA_APP_H
