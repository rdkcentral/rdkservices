#pragma once

#include <syscall.h>

enum LogLevel {FATAL_LEVEL = 0, ERROR_LEVEL, WARNING_LEVEL, INFO_LEVEL, DEBUG_LEVEL};

static int gDefaultLogLevel = ERROR_LEVEL;

namespace {
    template <typename T>
    const T* UNUSED_GLOBAL_VARIABLE_HELPER(const T& dummy) {
        return &dummy;
    }

    static const auto UNUSED_GLOBAL_VARIABLE_gDefaultLogLevel =
        UNUSED_GLOBAL_VARIABLE_HELPER(gDefaultLogLevel);
}

#define LOGDBG(fmt, ...) do { if(gDefaultLogLevel >= DEBUG_LEVEL) { fprintf(stderr, "[%d] DEBUG [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); }} while (0)
#define LOGINFO(fmt, ...) do { if(gDefaultLogLevel >= INFO_LEVEL) { fprintf(stderr, "[%d] INFO [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); }} while (0)
#define LOGWARN(fmt, ...) do { if(gDefaultLogLevel >= WARNING_LEVEL) { fprintf(stderr, "[%d] WARN [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); }} while (0)
#define LOGERR(fmt, ...) do { if(gDefaultLogLevel >= ERROR_LEVEL) { fprintf(stderr, "[%d] ERROR [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); }} while (0)

#define LOG_DEVICE_EXCEPTION0() LOGWARN("Exception caught: code=%d message=%s", err.getCode(), err.what());
#define LOG_DEVICE_EXCEPTION1(param1) LOGWARN("Exception caught" #param1 "=%s code=%d message=%s", param1.c_str(), err.getCode(), err.what());
#define LOG_DEVICE_EXCEPTION2(param1, param2) LOGWARN("Exception caught " #param1 "=%s " #param2 "=%s code=%d message=%s", param1.c_str(), param2.c_str(), err.getCode(), err.what());
