#ifndef RDKSHELL_LOGGER_H
#define RDKSHELL_LOGGER_H
#include <string>

#include "rdkshell.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

namespace RdkShell
{
    enum LogLevel { 
        Debug,
        Information,
        Warn,
        Error, 
        Fatal
    };

    class Logger
    {
        public:
            static void log(LogLevel level, const char* format, ...){return;}
            static void setLogLevel(const char* loglevel){return;}
            static void logLevel(std::string& level){return;}
            static void enableFlushing(bool enable){return;}
            static bool isFlushingEnabled(){return true;}

        private:
            static LogLevel sLogLevel;
            static bool sFlushingEnabled;
    };

    static const char* logLevelStrings[] =
    {
      "DEBUG",
      "INFO",
      "WARN",
      "ERROR",
      "FATAL"
    };

    static const int numLogLevels = sizeof(logLevelStrings)/sizeof(logLevelStrings[0]);
    
    const char* logLevelToString(LogLevel l)
    {
      const char* s = "LOG";
      int level = (int)l;
      if (level < numLogLevels)
        s = logLevelStrings[level];
      return s;
    }

    LogLevel Logger::sLogLevel = Information;
    bool Logger::sFlushingEnabled = false;

}

#endif //RDKSHELL_LOGGER_H

