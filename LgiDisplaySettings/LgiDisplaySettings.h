#include "DisplaySettings.h"
#include "utils.h"

namespace WPEFramework {
    namespace Plugin {
        // all LGI extensions are implemented here
        class LgiDisplaySettings : public DisplaySettings
        {
            public:
                LgiDisplaySettings();
                virtual ~LgiDisplaySettings()
                {
                }

                uint32_t setOutputFrameRatePreference(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay, followContent)
                uint32_t setAudioProcessingHint(const JsonObject& parameters, JsonObject& response); // args:  (audioPort, audioMode, audioDelayMs)
                uint32_t getAudioOutputEncoding(const JsonObject& parameters, JsonObject& response); //args: (audioPort)
                uint32_t getFollowColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay)
                uint32_t setFollowColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay,followColorSpace)
                uint32_t getPreferredOutputColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay)
                uint32_t setPreferredOutputColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay,colorSpaces: array of colorspace string)
                uint32_t getHDRGfxColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay)
                uint32_t setHDRGfxColorSpace(const JsonObject& parameters, JsonObject& response); // args: (videoDisplay,y,cr,cb)

            protected:
                virtual bool getDefaultActiveInput() const
                {
                    return false;
                }

                virtual bool isFollowSoundModeSupported() const
                {
                    return true;
                }

                virtual uint32_t getBassEnhancer(const JsonObject& parameters, JsonObject& response);
        };
    } //Plugin
} //WPEFramework
