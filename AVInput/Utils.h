#pragma once

#include <cstddef>
#include <string>
#include <syscall.h>

namespace Utils
{
    struct SecurityToken
    {
        static void getSecurityToken(std::string& token);
        static bool isThunderSecurityConfigured();

    private:
        static inline std::string m_sToken = "SecurityToken";
        static inline bool m_sThunderSecurityChecked = true;
    };
} // namespace Utils

