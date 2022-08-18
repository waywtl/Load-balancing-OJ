#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;
int main()
{
    struct rlimit r;
    r.rlim_cur = 1024*1024;
    r.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_AS, &r);

    int* p = new int[1024*1024];
    return 0;
}