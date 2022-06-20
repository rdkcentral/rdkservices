#pragma once

#include "UtilsLogging.h"

namespace Utils {
//class for std::thread RAII
class ThreadRAII {
public:
    ThreadRAII() {}
    ThreadRAII(std::thread&& t)
        : t(std::move(t))
    {
    }
    ~ThreadRAII()
    {
        try {
            if (t.joinable()) {
                t.join();
            }
        } catch (const std::system_error& e) {
            LOGERR("system_error exception in thread join %s", e.what());
        } catch (const std::exception& e) {
            LOGERR("exception in thread join %s", e.what());
        }
    }

    //support moving
    ThreadRAII(ThreadRAII&&) = default;
    ThreadRAII& operator=(ThreadRAII&&) = default;

    std::thread& get() { return t; }

private:
    std::thread t;
};
}
