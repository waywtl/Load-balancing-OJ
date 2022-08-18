#pragma once

#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>
#include <unistd.h>
#include <signal.h>

#include "compiler.hpp"
#include "runner.hpp"
#include "../comm/util.hpp"

namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;

    class CompileAndRun
    {
    public:
        //code > 0 : 用户程序收到信号，导致异常退出
        //code < 0 : 用户程序非运行报错
        //code = 0 : 用户程序成功运行
        static std::string CodeToDesc(int code, const std::string& file_name)
        {
            std::string desc;
            switch (code)
            {
            case 0:
                desc = "编译运行成功";
                break;
            case -1:
                desc = "用户提交的代码为空";
                break;
            case -2:
                desc = "发生了未知错误";
                break;
            case -3:
                FileUtil::ReadFile(PathUtil::CompileErr(file_name), &desc, true);
                break;
            case SIGABRT: // 6
                desc = "内存超过范围";
                break;
            case SIGXCPU: // 24
                desc = "超出时间限制";
                break;
            case SIGFPE: // 8
                desc = "发生除零错误,浮点数溢出";
                break;
            default:
                desc = "未知: " + std::to_string(code);
                break;
            }
            return desc;
        }

        // 删除临时文件
        static void RemoveTmpFile(const std::string& file_name)
        {
            std::string _src = PathUtil::Src(file_name);
            if(FileUtil::IsFileExist(_src))
                unlink(_src.c_str());

            std::string _exe = PathUtil::Exe(file_name);
            if(FileUtil::IsFileExist(_exe))
                unlink(_exe.c_str());

            std::string _compile_err = PathUtil::CompileErr(file_name);
            if(FileUtil::IsFileExist(_compile_err))
                unlink(_compile_err.c_str());
        
            std::string _stdin = PathUtil::Stdin(file_name);
            if(FileUtil::IsFileExist(_stdin))
                unlink(_stdin.c_str());

            std::string _stdout = PathUtil::Stdout(file_name);
            if(FileUtil::IsFileExist(_stdout))
                unlink(_stdout.c_str());

            std::string _stderr = PathUtil::Stderr(file_name);
            if(FileUtil::IsFileExist(_stderr))
                unlink(_stderr.c_str());
        }

        /*
         * 输入:
         * code:  用户提交的代码
         * input: 用户给自己提交的代码对应的输入，不做处理
         * cpu_limit: 时间要求
         * mem_limit: 内存要求
         *
         * 输出:
         * 必填
         * status: 状态码
         * reason: 请求结果
         * 选填
         * stdout: 用户程序的运行结果
         * stderr: 用户程序的错误运行结果
         */
        static void Start(const std::string &in_json, std::string *out_json)
        {
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json, in_value);

            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            int status_code = 0;
            Json::Value out_value;
            std::string file_name;
            int run_result = 0;

            //处理空代码情况
            if (code.size() == 0)
            {
                status_code = -1; //代码为空
                goto END;
            }

            //形成的文件名具有唯一性，不带目录和后缀
            //保证唯一性: 毫秒级时间戳+原子性递增唯一值
            file_name = FileUtil::GetUniqueFileName();

            //形成src临时文件，并写入用户的代码
            if (!FileUtil::WriteFile(PathUtil::Src(file_name), code)) //形成临时src文件
            {
                status_code = -2; //未知错误
                goto END;
            }

            if (!Compiler::Compile(file_name))
            {
                //编译失败
                status_code = -3; //代码编译时发生错误
                goto END;
            }

            run_result = Runner::Run(file_name, cpu_limit, mem_limit);
            if (run_result < 0)
            {
                //内部错误
                status_code = -2; //未知错误
            }
            else if (run_result > 0)
            {
                //收到信号退出
                status_code = run_result; //运行用户代码时错误
            }
            else
            {
                //运行成功
                status_code = 0;
            }

            // end
        END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeToDesc(status_code, file_name);

            if (status_code == 0)
            {
                //所有过程全部成功时
                std::string stdout_content;
                FileUtil::ReadFile(PathUtil::Stdout(file_name), &stdout_content, true);
                out_value["stdout"] = stdout_content;

                std::string stderr_content;
                FileUtil::ReadFile(PathUtil::Stderr(file_name), &stderr_content, true);
                out_value["stderr"] = stderr_content;
            }

            //序列化
            Json::StyledWriter writer;
            *out_json = writer.write(out_value);

            RemoveTmpFile(file_name);
        }
    };
};