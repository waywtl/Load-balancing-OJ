#pragma once

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sys/time.h>
#include <atomic>
#include <string>
#include <boost/algorithm/string.hpp>

namespace ns_util
{      
    const std::string tmp_path = "./tmp/";
    
    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }

        static std::string GetTimeMs()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };

    class PathUtil
    {
    public:
        static std::string AddSuffix(const std::string& file_name, const std::string& suffix)
        {
            std::string path_name = tmp_path;
            path_name += file_name;
            path_name += suffix;
            return path_name;
        }

        //编译时需要的临时文件
        // 构建源文件的完整路径+后缀名
        static std::string Src(const std::string& file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }
        // 构建可执行程序的完整路径+后缀名
        static std::string Exe(const std::string& file_name)
        {
            return AddSuffix(file_name, ".exe");
        }
        // 构建该程序对应的标准错误的完整路径+后缀名(编译时错误)
        static std::string CompileErr(const std::string& file_name)
        {
            return AddSuffix(file_name, ".compile_err");
        }

        //运行时需要的临时文件
        static std::string Stdin(const std::string& file_name)
        {
            return AddSuffix(file_name, ".stdin");
        }
        // 构建该程序对应的标准输出的完整路径+后缀名
        static std::string Stdout(const std::string& file_name)
        {
            return AddSuffix(file_name, ".stdout");
        }
        // 构建该程序对应的标准错误的完整路径+后缀名(运行时错误)
        static std::string Stderr(const std::string& file_name)
        {
            return AddSuffix(file_name, ".stderr");
        }
    };

    class FileUtil
    {
    public:
        static bool IsFileExist(const std::string& path_name)
        {
            struct stat st;
            if(stat(path_name.c_str(), &st) == 0)
            {
                return true;
            }
            return false;
        }

        //保证唯一性: 毫秒级时间戳+原子性递增唯一值
        static std::string GetUniqueFileName()
        {
            static std::atomic_uint id(0);
            ++id;
            std::string ms = TimeUtil::GetTimeMs();
            std::string unique_id = ms + "_" + std::to_string(id);
            return unique_id;
        }

        static bool WriteFile(const std::string& target, const std::string& content)
        {
            std::ofstream out(target);
            if(!out.is_open())
            {
                return false;
            }
            out.write(content.c_str(), content.size());
            out.close();
            return true;
        }

        static bool ReadFile(const std::string& target, std::string* content, bool keep = false)
        {
            (*content).clear();
            std::ifstream in(target);
            if(!in.is_open())
            {
                return false;
            }
            std::string line;
            //getline不保留'\n'，有时我们需要保留
            while(std::getline(in, line))
            {
                (*content) += line;
                //通过增加keep选项来决定是否保留"\n"
                (*content) += (keep ? "\n" : "");
            }
            in.close();
            return true;
        }
    };

    class StringUtil
    {
    public:
        /*
         * target: 需要切分的字符串
         * out; 保存切分后的结果
         * sep: 以sep为分隔符切分str
         */
        static void SplitString(const std::string& target, std::vector<std::string>* out, const std::string& sep)
        {
            // boost split
            boost::split((*out), target, boost::is_any_of(sep), boost::algorithm::token_compress_on);
        }
    };
};