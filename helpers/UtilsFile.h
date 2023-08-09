#pragma once

#include <core/core.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

namespace Utils
{
auto MoveFile(
    const string &from,
    const string &to) -> bool
{
    using namespace WPEFramework::Core;

    File fileFrom(from);
    File fileTo(to);

    Directory(fileTo.PathName().c_str()).CreatePath();

    bool result =
        fileFrom.Exists() &&
            !fileTo.Exists() &&
            fileFrom.Open(true) &&
            fileTo.Create();

    if (result) {
        const uint32_t bufLen = 1024;

        uint8_t buffer[bufLen];

        do {
            auto len = fileFrom.Read(buffer, bufLen);
            if (len <= 0) {
                break;
            }

            auto ptr = buffer;

            do {
                auto count = fileTo.Write(ptr, len);
                if (count <= 0) {
                    result = false;
                    break;
                }

                len -= count;
                ptr += count;
            }
            while (len > 0);
        }
        while (result);

        if (result) {
            fileFrom.Destroy();
        }
        else {
            fileTo.Destroy();
        }
    }

    return result;
}

/**
* @brief Get the last non empty line from the input string, equivalent to "tr -s '\r' '\n' | tail -n 1"
* @param[in] input - The input string
* @param[out] res_str - The last non empty line from the input string
* @return whether or not a non empty line was found
*/
bool getLastLine(const std::string& input, std::string& res_str)
{
    string read_line = "";
    bool ret_value = false;

    if (!input.empty())
    {
        stringstream read_str(input);
        while (getline(read_str, read_line, '\n'))
        {
            if (!read_line.empty())
            {
                res_str = read_line;
                ret_value = true;
            }
        }
    }
    return ret_value;
}

}
