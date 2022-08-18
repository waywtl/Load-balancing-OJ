#pragma once

#include <iostream>
#include <string>
#include <ctemplate/template.h>

#include "oj_model.hpp"

namespace ns_view
{
    using namespace ns_model;

    const std::string template_path = "./template_html/";

    class View
    {
    public:
        View()
        {}
        ~View()
        {}
    public:
        void AllExpandHtml(const std::vector<Question>& all, std::string* html)
        {
            // 题目的编号 题目的标题 题目的难度
            // 推荐使用表格显示
            // 1. 形成路径
            std::string src_html_path = template_path + "all_questions.html";
            // 2. 形成数字典
            ctemplate::TemplateDictionary root("all_questions");
            for(const auto& q : all)
            {
                ctemplate::TemplateDictionary* sub = root.AddSectionDictionary("question_list");
                sub->SetValue("number", q.number);
                sub->SetValue("title", q.title);
                sub->SetValue("level", q.level);
            }
            // 3. 获取被渲染的html
            ctemplate::Template* tpl = ctemplate::Template::GetTemplate(src_html_path, ctemplate::DO_NOT_STRIP);
            // 4. 开始完成渲染功能
            tpl->Expand(html, &root);
        }

        void OneExpandHtml(const Question& q, std::string* html)
        {
            // 1. 形成路径
            std::string src_html_path = template_path + "question.html";
            // 2. 形成数字典
            ctemplate::TemplateDictionary root("question");
            root.SetValue("number", q.number);
            root.SetValue("title", q.title);
            root.SetValue("level", q.level);
            root.SetValue("desc", q.desc);
            root.SetValue("pre_code", q.head);
            // 3. 获取被渲染的html
            ctemplate::Template* tpl = ctemplate::Template::GetTemplate(src_html_path, ctemplate::DO_NOT_STRIP);
            // 4. 开始完成渲染功能
            tpl->Expand(html, &root);
        }
    };
};