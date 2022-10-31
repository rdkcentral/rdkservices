#pragma once

namespace RdkShell
{
    #define RDKSHELL_APPLICATION_MIME_TYPE_NATIVE "application/native"
    #define RDKSHELL_APPLICATION_MIME_TYPE_DAC_NATIVE "application/dac.native"
    #define RDKSHELL_APPLICATION_MIME_TYPE_HTML "application/html"
    #define RDKSHELL_APPLICATION_MIME_TYPE_LIGHTNING "application/lightning"

    enum class ApplicationState
    {
        Unknown,
        Running,
        Suspended,
        Stopped
    };
}
