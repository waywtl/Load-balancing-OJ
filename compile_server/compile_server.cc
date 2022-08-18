#include <iostream>

#include "compile_and_run.hpp"
#include "../comm/httplib.h"

using namespace ns_compile_and_run;
using namespace httplib;

void Usage(std::string proc)
{
    std::cerr << "Usage: " << "\n\t" << proc << " port" << std::endl;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        return 1;
    }

    Server svr;

    // svr.Get("/s", [](const Request& req, Response& resp)
    // {
    //     resp.set_content("halo!", "text/plain;charset=utf-8");
    // });

    svr.Post("/compile_and_run", [](const Request& req, Response& resp)
    {
        std::string in_json = req.body;
        std::string out_json;
        if(!in_json.empty())
        {
            CompileAndRun::Start(in_json, &out_json);
            resp.set_content(out_json, "application/json;charset=utf-8");
        }
    });

    svr.listen("0.0.0.0", atoi(argv[1]));

    // std::string in_json;
    
    // Json::Value in_value;
    // in_value["code"] = R"(
    // #include <iostream>
    // int main()
    // {
    //     std::cout << "halo" << std::endl;
    //     return 0;
    // }
    // )";
    // in_value["input"] = "";
    // in_value["cpu_limit"] = 1;
    // in_value["mem_limit"] = 10240 * 5;

    // Json::FastWriter writer;
    // in_json = writer.write(in_value);
    // std::cout << in_json << std::endl;

    // std::string out_json;
    // CompileAndRun::Start(in_json, &out_json);
    // std::cout << out_json << std::endl;

    return 0;
}