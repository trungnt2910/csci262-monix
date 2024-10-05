#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <cerrno>
#include <cstring>

#include "Logger.h"

#define MX_RETURN_IF_FAIL(expr)                                     \
    do                                                              \
    {                                                               \
        int result__ = (expr);                                      \
        if (result__ < 0)                                           \
        {                                                           \
            Logger.LogWarning(#expr " failed: ", strerror(errno));  \
            return -1;                                              \
        }                                                           \
    } while (0)


#endif // UTIL_H_INCLUDED
