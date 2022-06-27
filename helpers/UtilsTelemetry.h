#pragma once

// telemetry
#ifdef ENABLE_TELEMETRY_LOGGING
#include <telemetry_busmessage_sender.h>
#endif

namespace Utils
{
    struct Telemetry
    {
        static void init()
        {
#ifdef ENABLE_TELEMETRY_LOGGING
            t2_init((char *) "Thunder_Plugins");
#endif
        };

        static void sendMessage(char* message)
        {
#ifdef ENABLE_TELEMETRY_LOGGING
            t2_event_s((char *)"THUNDER_MESSAGE", message);
#endif
        };

        static void sendMessage(char *marker, char* message)
        {
#ifdef ENABLE_TELEMETRY_LOGGING
            t2_event_s(marker, message);
#endif
        };

        static void sendError(const char* format, ...)
        {
#ifdef ENABLE_TELEMETRY_LOGGING
            va_list parameters;
            va_start(parameters, format);
            std::string message;
            WPEFramework::Trace::Format(message, format, parameters);
            va_end(parameters);

            // get rid of const for t2_event_s
            char* error = strdup(message.c_str());
            t2_event_s((char *)"THUNDER_ERROR", error);
            if (error)
            {
                free(error);
            }
#endif
        };
    };
}
