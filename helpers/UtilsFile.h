#pragma once

#include <core/core.h>

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
}
