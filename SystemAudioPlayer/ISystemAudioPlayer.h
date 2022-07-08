
#ifndef __ISYSTEMAUDIOPLAYER_H
#define __ISYSTEMAUDIOPLAYER_H

#include "Module.h"
#include <interfaces/Ids.h>

namespace WPEFramework {
namespace Exchange {

    struct EXTERNAL ISystemAudioPlayer : virtual public Core::IUnknown {
        enum { ID = ID_BROWSER + 0x7000 };

        struct INotification : virtual public Core::IUnknown {
            enum { ID = ISystemAudioPlayer::ID + 1};

            virtual ~INotification() {}
            virtual void OnSAPEvents(const string &data) = 0;        
        };

        virtual ~ISystemAudioPlayer() {}

        virtual uint32_t Configure(PluginHost::IShell* service) = 0;
        virtual void Register(INotification* sink) = 0;
        virtual void Unregister(INotification* sink) = 0;

        virtual uint32_t Open(const string &input, string &output /* @out */) = 0;
        virtual uint32_t Play(const string &input, string &output /* @out */) = 0;
        virtual uint32_t PlayBuffer(const string &input, string &output /* @out */) = 0;
        virtual uint32_t Pause(const string &input, string &output /* @out */) = 0;
        virtual uint32_t Resume(const string &input, string &output /* @out */) = 0;
        virtual uint32_t Stop(const string &input, string &output /* @out */) = 0;
        virtual uint32_t Close(const string &input, string &output /* @out */) = 0;
        virtual uint32_t SetMixerLevels(const string &input, string &output /* @out */) = 0;
        virtual uint32_t SetSmartVolControl(const string &input, string &output /* @out */) = 0;
        virtual uint32_t IsPlaying(const string &input, string &output /* @out */) = 0;
	virtual uint32_t Config(const string &input, string &output /* @out */) = 0;
        virtual uint32_t GetPlayerSessionId(const string &input, string &output /* @out */) = 0;

    };

} // Exchange
} // WPEFramework

#endif //__ISYSTEMAUDIOPLAYER_H
