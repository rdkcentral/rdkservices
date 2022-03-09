#pragma once

#include <core/core.h>

namespace Utils
{
auto MoveFile(
    const string &to,
    const string &from) -> bool
{
    bool result = true;

    using namespace WPEFramework::Core;

    File fileTo(to);
    File fileFrom(from);

    if (fileFrom.Exists() &&
        !fileTo.Exists() &&
        fileFrom.Open(true) &&
        fileTo.Create()) {

        const uint32_t bufLen = 1024;

        uint32_t len;
        uint8_t buffer[bufLen];

        do {
            len = fileFrom.Read(buffer, bufLen);

            if (len != 0) {
                auto ptr = buffer;

                do {
                    auto count = fileTo.Write(ptr, len);

                    if (count == 0) {
                        result = false;

                        break;
                    }

                    len -= count;
                    ptr += count;
                }
                while (len != 0);
            }
            else {
                break;
            }
        }
        while (result);

        if (result) {
            fileFrom.Destroy();
        }
        else {
            fileTo.Destroy();
        }
    }
    else {
        result = false;
    }

    return result;
}
}
