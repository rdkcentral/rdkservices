
#ifndef UNIFIEDPLAYERNOTIFY_H
#define UNIFIEDPLAYERNOTIFY_H

#include <string>
#include <stdint.h>

namespace WPEFramework {

    class UnifiedPlayerNotify {
    public:
        UnifiedPlayerNotify() {};
        virtual ~UnifiedPlayerNotify() {};


        virtual void event_onplayerinitialized(const uint32_t& sessionid) {};

        // Event: onmediaopened - Sent when media is successfully opened
        virtual void event_onmediaopened(const uint32_t& sessionid, const string& mediatype, const uint32_t& duration, const uint32_t& width, const uint32_t& height, const std::string& availablespeeds, const std::string& availableaudiolanguages, const std::string& availablesubtitleslanguages) {};

        // Event: onplaying - Sent when the player transitions to x1 play speed
        virtual void event_onplaying(const uint32_t& sessionid) {};

        // Event: onpaused - Sent when the player enters a paused state
        virtual void event_onpaused(const uint32_t& sessionid) {};

        // Event: onclosed - Sent when media is closed
        virtual void event_onclosed(const uint32_t& sessionid) {};

        // Event: oncomplete - Sent when the stream complete
        virtual void event_oncomplete(const uint32_t& sessionid) {};

        // Event: onstatus - Sent when status changes
        virtual void event_onstatus(const uint32_t& sessionid, const float& position, const float& duration) {};

        // Event: onprogress - Sent when progress or duration changes
        virtual void event_onprogress(const uint32_t& sessionid, const float& position, const float& duration, const float& speed, const float& start, const float& end) {};

        // Event: onwarning - Sent when a warning is issued
        virtual void event_onwarning(const uint32_t& sessionid, const uint32_t& code, const string& description) {};

        // Event: onerror - Sent when an error is issued
        virtual void event_onerror(const uint32_t& sessionid, const uint32_t& code, const string& description) {};

        // Event: onbitratechange - Sent when current ABR bitrate changes
        virtual void event_onbitratechange(const uint32_t& sessionid, const uint32_t& bitrate, const string& reason) {};

        // Event: oncasdata - CAS public data is sent asynchronously from CAS System
        virtual void event_oncasdata(const uint32_t& sessionid, const string& casData) {};
    };

} // namespace

#endif /* UNIFIEDPLAYERNOTIFY_H */

