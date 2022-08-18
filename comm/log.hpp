#pragma once

#include <iostream>
#include <string>

#include "util.hpp"

namespace ns_log
{
    using namespace ns_util;
    // 日志等级
    enum
    {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL
    };

    // LOG(INFO) << "message" << "\n";
    inline std::ostream& Log(const std::string& level, const std::string& file_name, int line)
    {
        std::string message;

        // 添加日志等级
        message += "[";
        message += level;
        message += "]";

        // 添加报错文件名
        message += "[";
        message += file_name;
        message += "]";

        // 添加报错行
        message += "[";
        message += std::to_string(line);
        message += "]";

        // 添加日志时间戳
        message += "[";
        message += TimeUtil::GetTimeStamp();
        message += "]";

        // cout内部包含缓冲区
        std::cout << message; // 不要endl进行刷新

        return std::cout;
    }

    // LOG(INFO) << "message" << "\n";
    // 开放式日志
    #define LOG(LEVEL) Log(#LEVEL, __FILE__, __LINE__)
}