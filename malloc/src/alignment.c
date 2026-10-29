#include "alignment.h"

size_t align(size_t size)
{
    size_t rest = size % sizeof(long double);
    if (rest == 0)
        return size;
    else
    {
        size_t res;
        if (__builtin_add_overflow(size, sizeof(long double) - rest, &res))
        {
            return 0;
        }
        return res;
    }
}
