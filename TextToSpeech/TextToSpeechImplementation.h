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
#include <interfaces/ITextToSpeech.h>
#include "tracing/Logging.h"

#include "impl/TTSManager.h"
#include "impl/TTSConfiguration.h"
#include <vector>

namespace WPEFramework {
namespace Plugin {

    class TextToSpeechImplementation : public Exchange::ITextToSpeech, public PluginHost::IStateControl, public TTS::TTSEventCallback {
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
             Job(TextToSpeechImplementation *tts, Event event, JsonValue &params)
                : _tts(tts)
                , _event(event)
                , _params(params) {
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
            static Core::ProxyType<Core::IDispatch> Create(TextToSpeechImplementation *tts, Event event, JsonValue params) {
                return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(tts, event, params)));
            }

            virtual void Dispatch() {
                _tts->Dispatch(_event, _params);
            }

        private:
            TextToSpeechImplementation *_tts;
            const Event _event;
            const JsonValue _params;
        };

    public:
        // We do not allow this plugin to be copied !!
        TextToSpeechImplementation(const TextToSpeechImplementation&) = delete;
        TextToSpeechImplementation& operator=(const TextToSpeechImplementation&) = delete;

        virtual uint32_t Configure(PluginHost::IShell* service) override;
        virtual void Register(Exchange::ITextToSpeech::INotification* sink) override ;
        virtual void Unregister(Exchange::ITextToSpeech::INotification* sink) override ;

        virtual PluginHost::IStateControl::state State() const override { return PluginHost::IStateControl::RESUMED; }
        virtual uint32_t Request(const command state) override;
        virtual void Register(IStateControl::INotification* notification) override {}
        virtual void Unregister(IStateControl::INotification* notification) override {}

        virtual uint32_t Enable(const bool enable) override;
        virtual uint32_t Enable(bool &enable /* @out */) const override;
        virtual uint32_t ListVoices(const string language,RPC::IStringIterator*& voices/* @out */) const override;
        virtual uint32_t SetConfiguration(const Exchange::ITextToSpeech::Configuration &object,Exchange::ITextToSpeech::TTSErrorDetail &status/* @out */) override ;
        virtual uint32_t SetFallbackText(const string scenario,const string value) override ;
        virtual uint32_t SetAPIKey(const string apikey) override ;
        virtual uint32_t SetPrimaryVolDuck(const uint8_t prim) override ;
        virtual uint32_t GetConfiguration(Exchange::ITextToSpeech::Configuration &object/* @out */) const override;
        virtual uint32_t Speak(const string text,uint32_t &speechid/* @out */,Exchange::ITextToSpeech::TTSErrorDetail &status/* @out */) override;
        virtual uint32_t Cancel(const uint32_t speechid) override;
        virtual uint32_t Pause(const uint32_t speechid,Exchange::ITextToSpeech::TTSErrorDetail &status /* @out */) override;
        virtual uint32_t Resume(const uint32_t speechid,Exchange::ITextToSpeech::TTSErrorDetail &status /* @out */) override;
        virtual uint32_t GetSpeechState(const  uint32_t speechid,Exchange::ITextToSpeech::SpeechState &state/* @out */) override;

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
        INTERFACE_ENTRY(PluginHost::IStateControl)
        END_INTERFACE_MAP

    private:
        static TTS::TTSManager* _ttsManager;
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::ITextToSpeech::INotification*> _notificationClients;

        void dispatchEvent(Event, const JsonValue &params);
        void Dispatch(Event event, const JsonValue params);

    public:
        TextToSpeechImplementation();
        virtual ~TextToSpeechImplementation();

        friend class Job;
    };

} // namespace Plugin
} // namespace WPEFramework
