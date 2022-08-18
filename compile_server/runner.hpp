#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "../comm/log.hpp"
#include "../comm/util.hpp"

namespace ns_runner
{
    using namespace ns_log;
    using namespace ns_util;

    class Runner
    {
    public:
        Runner(){}
        ~Runner(){}

    public:
        //限制进程运行时所占资源
        static void SetProcLimit(int cpu_limit, int mem_limit)
        {
            //限制cpu时长
            struct rlimit r_cpu;
            r_cpu.rlim_cur = cpu_limit;
            r_cpu.rlim_max = RLIM_INFINITY;
            setrlimit(RLIMIT_CPU, &r_cpu);

            //限制内存大小
            struct rlimit r_mem;
            r_mem.rlim_cur = mem_limit*1024;//转化成KB
            r_mem.rlim_max = RLIM_INFINITY;
            setrlimit(RLIMIT_AS, &r_mem);
        }
        /*返回值
        * 返回值 > 0  :程序收到了退出信号，异常退出了，返回信号的编号
        * 返回值 == 0 :程序正常运行完，运行结果在对于文件中
        * 返回值 < 0  :内部错误
        */
        // cpu_limit : 程序运行时，可以使用cpu的最大时长
        // mem_limit : 程序运行时，可以使用的最大内存(KB)
        static int Run(const std::string& file_name, int cpu_limit, int mem_limit)
        {
            /*
            * 对于程序运行而言，只会发生以下三种情况
            * 1.运行成功，结果正确
            * 2.运行成功，结果不正确
            * 3.运行失败，出现异常，运行时报错
            * 但对于Runner来说，对结果不关心
            * 只关心程序运行成功与否
            * 
            * 程序启动后，我们需要拿到打印到显示器上的运行结果或错误信息
            */
            std::string _execute = PathUtil::Exe(file_name);
            std::string _stdin = PathUtil::Stdin(file_name);
            std::string _stdout = PathUtil::Stdout(file_name);
            std::string _stderr = PathUtil::Stderr(file_name);

            umask(0);
            int _stdin_fd = open(_stdin.c_str(), O_CREAT | O_RDONLY, 0644);
            int _stdout_fd = open(_stdout.c_str(), O_CREAT | O_WRONLY, 0644);
            int _stderr_fd = open(_stderr.c_str(), O_CREAT | O_WRONLY, 0644);

            if(_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0)
            {
                LOG(ERROR) << "运行时打开标准文件失败" << std::endl;
                return -1;//代表文件打开失败
            }

            pid_t pid = fork();
            if(pid < 0)
            {
                LOG(ERROR) << "运行时创建子进程失败" << std::endl;
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                return -2;//代表创建子进程失败
            }
            else if(pid == 0)
            {
                // 子进程
                dup2(_stdin_fd, 0);
                dup2(_stdout_fd, 1);
                dup2(_stderr_fd, 2);
                
                SetProcLimit(cpu_limit, mem_limit);
                execl(_execute.c_str(), _execute.c_str(), nullptr);
                exit(1);
            }
            else
            {
                // 父进程
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                int status = 0;
                waitpid(pid, &status, 0);

                //如果程序异常退出，一定是收到了信号
                int exit_signal = status & 0x7F;
                if(exit_signal == 0)
                {
                    LOG(INFO) << _execute << "运行成功!" << std::endl;
                    return 0;
                }
                LOG(ERROR) << "运行失败，程序收到信号: "<< exit_signal << ",异常退出" << std::endl;
                return exit_signal;
            }
        }
    };
};