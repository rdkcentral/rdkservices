#pragma once

#include <stdio.h>
#include <string>

namespace Utils {
inline std::string cRunScript(const char* cmd)
{
    std::string totalStr = "";
    FILE* pipe = NULL;
    char buff[1024] = { '\0' };

    if ((pipe = popen(cmd, "r"))) {
        memset(buff, 0, sizeof(buff));
        while (fgets(buff, sizeof(buff), pipe)) {
            totalStr += buff;
            memset(buff, 0, sizeof(buff));
        }
        pclose(pipe);
    } else {
        /* popen failed. */
    }
    return totalStr;
}
}
