#pragma once

#include "UtilsLogging.h"

namespace Utils {

class ThreadRAII {
public:
    ThreadRAII() {}
    ThreadRAII(std::thread&& t)
        : t(std::move(t))
    {
    }
    ~ThreadRAII()
    {
#ifndef ENABLE_GTEST
        try {
#endif
            if (t.joinable()) {
                t.join();
            }
#ifndef ENABLE_GTEST
        } catch (const std::system_error& e) {
            LOGERR("system_error exception in thread join %s", e.what());
        } catch (const std::exception& e) {
            LOGERR("exception in thread join %s", e.what());
        }
#endif
    }

    //support moving
    ThreadRAII(ThreadRAII&&) = default;
    ThreadRAII& operator=(ThreadRAII&&) = default;

    std::thread& get() { return t; }

private:
    std::thread t;
};
}
