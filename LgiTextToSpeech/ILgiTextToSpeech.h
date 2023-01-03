
#ifndef __ILGITEXTTOSPEECH_H
#define __ILGITEXTTOSPEECH_H

#include "Module.h"
#include <interfaces/Ids.h>

namespace WPEFramework {
namespace Exchange {

    struct EXTERNAL ILgiTextToSpeech : virtual public Core::IUnknown {
        enum { ID = ID_BROWSER + 0x10000 };

        struct INotification : virtual public Core::IUnknown {
            enum { ID = ILgiTextToSpeech::ID + 1};

            virtual ~INotification() {}

            virtual void StateChanged(const string &data) = 0;
            virtual void VoiceChanged(const string &data) = 0;
            virtual void WillSpeak(const string &data) = 0;
            virtual void SpeechStart(const string &data) = 0;
            virtual void SpeechPause(const string &data) = 0;
            virtual void SpeechResume(const string &data) = 0;
            virtual void SpeechCancelled(const string &data) = 0;
            virtual void SpeechInterrupted(const string &data) = 0;
            virtual void NetworkError(const string &data) = 0;
            virtual void PlaybackError(const string &data) = 0;
            virtual void SpeechComplete(const string &data) = 0;
        };

        virtual ~ILgiTextToSpeech() {}

        virtual uint32_t Configure(PluginHost::IShell* service) = 0;
        virtual void Register(INotification* sink) = 0;
        virtual void Unregister(INotification* sink) = 0;

        virtual uint32_t Enable(const string &input, string &output /* @out */) = 0;
        virtual uint32_t ListVoices(const string &input, string &output /* @out */) = 0;
        virtual uint32_t SetConfiguration(const string &input, string &output /* @out */) = 0;
        virtual uint32_t GetConfiguration(const string &input, string &output /* @out */) = 0;
        virtual uint32_t IsEnabled(const string &input, string &output /* @out */) = 0;
        virtual uint32_t Speak(const string &input, string &output /* @out */) = 0;
        virtual uint32_t Cancel(const string &input, string &output /* @out */) = 0;
        virtual uint32_t Pause(const string &input, string &output /* @out */) = 0;
        virtual uint32_t Resume(const string &input, string &output /* @out */) = 0;
        virtual uint32_t IsSpeaking(const string &input, string &output /* @out */) = 0;
        virtual uint32_t GetSpeechState(const string &input, string &output /* @out */) = 0;

    };

} // Exchange
} // WPEFramework

#endif //__ILGITEXTTOSPEECH_H

