#ifdef _MSC_VER
#include <malloc.h>
#define alloca _alloca
#else
#include <alloca.h>
#endif

int main(void)
{
    char* vla = alloca(10);
    memset(vla, 0, 10);
    return 0;
}
