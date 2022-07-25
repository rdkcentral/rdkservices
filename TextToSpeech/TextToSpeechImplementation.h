/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Module.h"
#include <interfaces/Ids.h>
#include "tracing/Logging.h"

#include "ITextToSpeech.h"
#include "impl/TTSManager.h"
#include "impl/TTSConfiguration.h"
#include <vector>

namespace WPEFramework {
namespace Plugin {

    class TextToSpeechImplementation : public Exchange::ITextToSpeech, public TTS::TTSEventCallback {
    public:
        enum Event {
                STATE_CHANGED,
                VOICE_CHANGED,
                WILL_SPEAK,
                SPEECH_START,
                SPEECH_PAUSE,
                SPEECH_RESUME,
                SPEECH_CANCEL,
                SPEECH_INTERRUPT,
                NETWORK_ERROR,
                PLAYBACK_ERROR,
                SPEECH_COMPLETE
            };

        class EXTERNAL Job : public Core::IDispatch {
        protected:
             Job(TextToSpeechImplementation *tts, Event event, string &data)
                : _tts(tts)
                , _event(event)
                , _data(data) {
                if (_tts != nullptr) {
                    _tts->AddRef();
                }
            }

       public:
            Job() = delete;
            Job(const Job&) = delete;
            Job& operator=(const Job&) = delete;
            ~Job() {
                if (_tts != nullptr) {
                    _tts->Release();
                }
            }

       public:
            static Core::ProxyType<Core::IDispatch> Create(TextToSpeechImplementation *tts, Event event, string data) {
                return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(tts, event, data)));
            }

            virtual void Dispatch() {
                _tts->Dispatch(_event, _data);
            }

        private:
            TextToSpeechImplementation *_tts;
            const Event _event;
            const string _data;
        };

    public:
        // We do not allow this plugin to be copied !!
        TextToSpeechImplementation(const TextToSpeechImplementation&) = delete;
        TextToSpeechImplementation& operator=(const TextToSpeechImplementation&) = delete;

        virtual uint32_t Configure(PluginHost::IShell* service);
        virtual void Register(INotification* sink) override ;
        virtual void Unregister(INotification* sink) override ;

        virtual uint32_t Enable(const string &input, string &output /* @out */) override ;
        virtual uint32_t ListVoices(const string &input, string &output /* @out */) override ;
        virtual uint32_t SetConfiguration(const string &input, string &output /* @out */) override ;
        virtual uint32_t GetConfiguration(const string &input, string &output /* @out */) override ;
        virtual uint32_t IsEnabled(const string &input, string &output /* @out */) override ;
        virtual uint32_t Speak(const string &input, string &output /* @out */) override ;
        virtual uint32_t Cancel(const string &input, string &output /* @out */) override ;
        virtual uint32_t Pause(const string &input, string &output /* @out */) override ;
        virtual uint32_t Resume(const string &input, string &output /* @out */) override ;
        virtual uint32_t IsSpeaking(const string &input, string &output /* @out */) override ;
        virtual uint32_t GetSpeechState(const string &input, string &output /* @out */) override ;

        virtual void onTTSStateChanged(bool enabled) override ;
        virtual void onVoiceChanged(std::string voice) override ;
        virtual void onWillSpeak(TTS::SpeechData &data) override ;
        virtual void onSpeechStart(TTS::SpeechData &data) override ;
        virtual void onSpeechPause(uint32_t speechId) override ;
        virtual void onSpeechResume(uint32_t speechId) override ;
        virtual void onSpeechCancelled(std::vector<uint32_t> speechIds) override ;
        virtual void onSpeechInterrupted(uint32_t speechId) override ;
        virtual void onNetworkError(uint32_t speechId) override ;
        virtual void onPlaybackError(uint32_t speechId) override ;
        virtual void onSpeechComplete(TTS::SpeechData &data) override ;

        BEGIN_INTERFACE_MAP(TextToSpeechImplementation)
        INTERFACE_ENTRY(Exchange::ITextToSpeech)
        END_INTERFACE_MAP

    private:
        static TTS::TTSManager* _ttsManager;
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::ITextToSpeech::INotification*> _notificationClients;

        void dispatchEvent(Event, JsonObject &params);
        void Dispatch(Event event, string data);

    public:
        TextToSpeechImplementation();
        virtual ~TextToSpeechImplementation();
        void setResponseArray(JsonObject& response, const char* key, const std::vector<std::string>& items);

        friend class Job;
    };

} // namespace Plugin
} // namespace WPEFramework
