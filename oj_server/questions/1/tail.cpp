#ifndef COMPILER_ONLINE
#include "head.cpp"
#endif

void Test1()
{
    vector<vector<char>> vv = {{'1','0','1','0','0'},{'1','0','1','1','1'},{'1','1','1','1','1'},{'1','0','0','1','0'}};
    int ret = Solution().maximalSquare(vv);
    if(ret == 4)
    {
        std::cout << "通过用例1!" << std::endl;
    }
    else
    {
        std::cout << "用例1错误...!" << std::endl;
    }
}

void Test2()
{
    vector<vector<char>> vv = {{'1','0'},{'0','1'}};
    int ret = Solution().maximalSquare(vv);
    if(ret == 1)
    {
        std::cout << "通过用例2!" << std::endl;
    }
    else
    {
        std::cout << "用例2错误...!" << std::endl;
    }
}

int main()
{
    Test1();
    Test2();
    return 0;
}