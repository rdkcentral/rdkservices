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

/* Get the last line from the given input file - tail -n 1  */
bool GetLastLine(const std::string& filename, std::string& last_line)
{
    string last_line = "", read_line = "";
    ifstream readfile(filename);
    bool ret_value = false;

    if (readfile.is_open())
    {
        while (getline (readfile, read_line))
        {
            last_line = read_line;
        }
        readfile.close();
        ret_value = true;
    }
    return ret_value;
}

/* remove empty lines from the given input file -  tr -s '\r' '\n'  */
bool RemoveEmptyLines(const std::string& fname, std::string& res_str)
{
    string read_line = "";
    int str_len = 0;
    bool ret_value = false;

    ifstream readfile(fname);
    if (readfile.is_open())
    {
        while (getline (readfile, read_line))
        {
            str_len = read_line.size();
            if (str_len != 0)
            {
                res_str.append((read_line.substr(0, str_len - 0)));
                res_str.append("\n");
            }
        }
        readfile.close();
        str_len = res_str.length();
        res_str = res_str.substr(0, str_len - 1);
        ret_value = true;
    }
    return ret_value;
}

// Remove Leading spaces in each line in the file - sed 's/^  *//g' <InputFile>
bool TrimLeadingSpaces(const std::string& filename, std::string& res_str)
{
    string read_line = "";
    int i = 0, str_len = 0;
    bool ret_value = false;

    ifstream readfile(filename);
    if (readfile.is_open())
    {
        while (getline (readfile, read_line))
        {
            i=0;
            str_len = read_line.size();
            while ((read_line[i] == ' ') && (read_line[i] != '\0'))
            {
                i++;
            }
            res_str.append((read_line.substr(i, str_len - i)));
            res_str.append("\n");
        }
        readfile.close();
        str_len = res_str.length();
        res_str = res_str.substr(0, str_len - 1);
        ret_value = true;
    }
    return ret_value;
}

}
