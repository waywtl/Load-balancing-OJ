#pragma once

#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../comm/log.hpp"
#include "../comm/util.hpp"

//根据questions.list文件，加载所有的题目信息到内存中
//model: 主要用来和数据进行交互，对外提供访问数据的接口

namespace ns_model
{
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
        std::string number; //题目的编号
        std::string title;  //题目的标题
        std::string level;  //题目的难度
        int cpu_limit;      //题目的时间限制(S)
        int mem_limit;      //题目的空间限制(KB)
        std::string desc;   //题目的描述
        std::string head;   //题目的预设代码
        std::string tail;   //题目的测试用例, 需要和head拼接，形成完整代码
    };

    const std::string question_list_path = "./questions/question.list";
    const std::string question_path = "./questions/";

    class Model
    {
    private:
        //题号 : 题目细节
        std::unordered_map<std::string, Question> questions;
    public:
        Model()
        {
            assert(LoadQuestionList(question_list_path));
        }
        ~Model(){}
    public:
        bool LoadQuestionList(const std::string& question_list_path)
        {
            //加载配置文件: questions/question.list + 对应题号目录中的文件
            std::ifstream in(question_list_path);
            if(!in.is_open())
            {
                LOG(FATAL) << "加载题库失败，请检查是否存在题库文件" << std::endl;
                return false;
            }

            std::string line;
            while(std::getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, " ");
                if(tokens.size() != 5)
                {
                    LOG(WARNING) << "加载部分题目失败，请检查题目文件格式" << std::endl;
                    continue;
                }

                Question q;
                q.number = tokens[0];
                q.title = tokens[1];
                q.level = tokens[2];
                q.cpu_limit = stoi(tokens[3]);
                q.mem_limit = stoi(tokens[4]);

                //拼接当前题目具体细节的目录
                std::string cur_question_path = question_path + q.number + "/";

                FileUtil::ReadFile(cur_question_path + "desc.txt", &(q.desc), true);
                FileUtil::ReadFile(cur_question_path + "head.cpp", &(q.head), true);
                FileUtil::ReadFile(cur_question_path + "tail.cpp", &(q.tail), true);
            
                questions.insert({q.number, q});
            }

            LOG(INFO) << "加载题库成功!" << std::endl;
            in.close();

            return true;
        }

        bool GetAllQuestions(std::vector<Question>* out)
        {
            if(questions.size() == 0)
            {
                LOG(ERROR) << "用户获取题库失败！" << std::endl;
                return false;
            }
            
            for(const auto& q : questions)
            {
                out->push_back(q.second);
            }

            //将所有的题目按编号排序
            sort(out->begin(), out->end(), [](const Question& q1, const Question& q2){return stoi(q1.number) < stoi(q2.number);});
            return true;
        }

        bool GetOneQuestion(const std::string& number, Question* q)
        {
            const auto& iter = questions.find(number);
            if(iter == questions.end())
            {
                LOG(ERROR) << "用户获取: " << q->number << " 号题目失败！" << std::endl;
                return false;
            }

            (*q) = iter->second;

            return true;
        }
    };
};