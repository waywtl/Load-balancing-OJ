#pragma once

#include <iostream>
#include <mutex>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <jsoncpp/json/json.h>

#include "oj_model.hpp"
#include "oj_view.hpp"
#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include "../comm/httplib.h"

namespace ns_control
{
    using namespace ns_model;
    using namespace ns_view;
    using namespace ns_log;
    using namespace ns_util;
    using namespace httplib;

    class Machine
    {
    public:
        std::string ip;  //编译服务的ip
        int port;        //编译服务的port
        uint64_t load;   //编译服务的负载
        std::mutex* mtx; //mutex禁止拷贝， 使用指针
    public:
        Machine()
            :ip(""), port(0), load(0), mtx(nullptr)
        {}
        ~Machine()
        {}
    public:
        //提升主机负载
        void IncLoad()
        {
            if(mtx)
                mtx->lock();
            ++load;
            if(mtx)
                mtx->unlock();
        }

        //减少主机负载
        void DecLoad()
        {
            if(mtx)
                mtx->lock();
            --load;
            if(mtx)
                mtx->unlock();
        }

        //负载清零
        void ResetLoad()
        {
            mtx->lock();
            load = 0;
            mtx->unlock();
        }

        //获取主机负载
        uint64_t GetLoad()
        {
            uint64_t _load = 0;
            if(mtx)
                mtx->lock();
            _load = load;
            if(mtx)
                mtx->unlock();
            
            return _load;
        }
    };

    const std::string service_machine = "./conf/service_machine.conf";
    // 负载均衡模块
    class LoadBalance
    {
    private:
        //可以给我们提供编译服务的所有主机
        //每一台主机都有自己的下标，充当主机的id
        std::vector<Machine> machines;
        //保存所有在线主机的id
        std::vector<int> online;
        //保存所有离线主机的id
        std::vector<int> offline;
        std::mutex mtx;
    public:
        LoadBalance()
        {
            assert(LoadConf(service_machine));
            LOG(INFO) << " 加载 " << service_machine << " 成功" << std::endl;   
        }
        ~LoadBalance()
        {}
    public:
        bool LoadConf(const std::string& machine_conf)
        {
            std::ifstream in(machine_conf);
            if(!in.is_open())
            {
                LOG(FATAL) << " 加载: " << machine_conf << " 失败" << std::endl;
                return false;
            }

            std::string line;
            while(std::getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, ":");
                if(tokens.size() != 2)
                {
                    LOG(WARNING) << " 切分 " << line << " 失败" << std::endl;
                    continue;
                }
                Machine m;
                m.ip = tokens[0];
                m.port = stoi(tokens[1]);
                m.load = 0;
                m.mtx = new std::mutex();

                online.push_back(machines.size());
                machines.push_back(m);
            }

            in.close();
            return true;
        }

        bool SmartChoice(int* id, Machine** m)
        {
            mtx.lock();

            int online_num = online.size();
            if(online_num == 0)
            {
                mtx.unlock();
                LOG(FATAL) << " 所有后端编译主机均离线..." << std::endl;
                return false;
            }

            //找到负载最小的主机
            uint64_t min_load = machines[online[0]].GetLoad();
            *id = online[0];
            *m = &machines[online[0]];
            for(int i = 0; i < online_num; ++i)
            {
                uint64_t cur_load = machines[online[i]].GetLoad();
                if(min_load > cur_load)
                {
                    min_load = cur_load;
                    *id = online[i];
                    *m = &machines[online[i]];
                }
            }
            mtx.unlock();
            return true;
        }

        void OnlineMachine()
        {
            mtx.lock();
            online.insert(online.end(), offline.begin(), offline.end());
            offline.erase(offline.begin(), offline.end());
            LOG(INFO) << "所有主机均已成功上线!" << std::endl;
            mtx.unlock();
        }

        void OfflineMachine(int id)
        {
            mtx.lock();
            for(auto iter = online.begin(); iter != online.end(); ++iter)
            {
                if(*iter == id)
                {
                    machines[id].ResetLoad();
                    online.erase(iter);
                    offline.push_back(id);
                    break;
                }
            }
            mtx.unlock();
        }    
    };

    // 核心业务逻辑的控制器
    class Control
    {
    private:
        Model _model;//提供后台数据
        View _view;  //提供html渲染功能
        LoadBalance _load_balance;//控制负载均衡
    public:
        Control()
        {}
        ~Control()
        {}
    public:
        void RevoveryMachines()
        {
            _load_balance.OnlineMachine();
        }

        bool AllQuestions(std::string* html)
        {
            bool ret = true;
            std::vector<Question> all;
            if(_model.GetAllQuestions(&all))
            {
                //将所有题目构建成网页
                _view.AllExpandHtml(all, html);
            }
            else
            {
                *html = "获取题目列表失败...";
                ret = false;
            }
            return ret;
        }

        bool OneQuestion(const std::string& number, std::string* html)
        {
            bool ret = true;
            Question q;
            if(_model.GetOneQuestion(number, &q))
            {
                //构建单个题目的网页
                _view.OneExpandHtml(q, html);
            }
            else
            {
                *html = "指定题目: " + number + " 不存在!";
                ret = false;
            }
            return ret;
        }

        void Judge(const std::string& number, const std::string& in_json, std::string* out_json)
        {
            // 0. 根据题目编号，直接拿到对应题目的细节
            struct Question q;
            _model.GetOneQuestion(number, &q);

            // 1. in_json进行反序列化，拿到用户提交题目的id, 源码, input
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);
            std::string code = in_value["code"].asString();

            // 2. 重新拼接用户代码+测试用例代码，形成新的代码
            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code + q.tail;
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;
            
            Json::FastWriter writer;
            std::string compile_string = writer.write(compile_value);

            // 3. 选择负载最低的主机
            // 规则: 一直选择，直到选到主机
            while(true)
            {
                int id = 0;
                Machine* m = nullptr;
                if(!_load_balance.SmartChoice(&id, &m))
                {
                    break;
                }
                LOG(INFO) << "选择主机成功, 主机id: " << id << " 详情: " <<  m->ip << ":" << m->port << " 当前主机的负载是: " << m->GetLoad() << std::endl;
                // 4. 发起http请求，得到结果
                Client cli(m->ip, m->port);
                m->IncLoad();
                if(auto res = cli.Post("/compile_and_run", compile_string, "application/json; charset=utf-8"))
                {
                    // 5. 将结果赋值给out_json
                    if(res->status == 200)
                    {
                        *out_json = res->body;
                        m->DecLoad();
                        break;
                    }
                    m->DecLoad();
                }
                else
                {
                    LOG(ERROR) << "当前请求的主机id: " << id << " 详情: " <<  m->ip << ":" << m->port << " 已离线" << std::endl;
                    _load_balance.OfflineMachine(id);
                }
            }

        }
    };
}