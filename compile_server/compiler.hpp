#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string>

#include "../comm/util.hpp"
#include "../comm/log.hpp"


namespace ns_compiler
{   
    using namespace ns_util;
    using namespace ns_log;
    
    class Compiler
    {
    public:
        Compiler(){}
        ~Compiler(){}

        static bool Compile(const std::string& file_name)
        {
            pid_t pid = fork();
            if(pid < 0)
            {
                LOG(ERROR) << "内部错误，创建子进程失败" << std::endl;
                return false;
            }
            else if(pid == 0)
            {
                umask(0);
                int _stderr = open(PathUtil::CompileErr(file_name).c_str(), O_CREAT | O_WRONLY, 0644);
                if(_stderr < 0)
                {
                    LOG(WARNING) << "没有成功形成compile_err文件" << std::endl;
                    exit(1);
                }
                //重定向标准错误到_stderr
                dup2(_stderr, 2);

                // 子进程: 调用编译器，完成对代码的编译工作
                // g++ -o target src -std=c++11
                // 添加 -D COMPILER_ONLINE 来进行条件编译，去掉测试用例中的include
                execlp("g++", "g++", "-o", PathUtil::Exe(file_name).c_str(), 
                        PathUtil::Src(file_name).c_str(), "-std=c++11", "-D", "COMPILER_ONLINE", nullptr);
                LOG(ERROR) << "启动g++编译器失败" << std::endl;
                exit(2);
            }
            else
            {
                // 父进程
                waitpid(pid, nullptr, 0);
                // 编译是否成功，就看有没有形成对应的可执行程序
                if(FileUtil::IsFileExist(PathUtil::Exe(file_name).c_str()))
                {
                    LOG(INFO) << PathUtil::Src(file_name) << "编译成功!" << std::endl;
                    return true;
                }
            }

            LOG(ERROR) << "编译失败，没有形成可执行程序" << std::endl;
            return false;
        } 
    };
};