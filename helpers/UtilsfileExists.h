#pragma once

#include <sys/stat.h>

namespace Utils {
inline bool fileExists(const char* pFileName)
{
    struct stat fileStat;
    return 0 == stat(pFileName, &fileStat);
}
}
