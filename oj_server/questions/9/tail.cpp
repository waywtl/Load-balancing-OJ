#ifndef COMPILER_ONLINE
#include "head.cpp"
#endif

void Test1()
{
    vector<int> nums = {1,3,-1,-3,5,3,6,7};
    vector<int> ret = Solution().maxSlidingWindow(nums, 3);
    vector<int> ans = {3,3,5,5,6,7};
    if(ret == ans)
    {
        std::cout << "通过用例1!" << std::endl;
    }
    else
    {
        std::cout << "用例1错误...!!" << std::endl;
    }
}

void Test2()
{
    vector<int> nums = {1};
    vector<int> ret = Solution().maxSlidingWindow(nums, 1);
    vector<int> ans = {1};
    if(ret == ans)
    {
        std::cout << "通过用例2!" << std::endl;
    }
    else
    {
        std::cout << "用例2错误...!!" << std::endl;
    }
}

int main()
{
    Test1();
    Test2();
    return 0;
}