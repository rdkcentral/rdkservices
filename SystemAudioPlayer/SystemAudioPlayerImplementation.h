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

#include "ISystemAudioPlayer.h"
#include "impl/AudioPlayer.h"
#include "impl/logger.h"
#include "impl/SecurityParameters.h"
#include <vector>

#define CHECK_SAP_PARAMETER_RETURN_ON_FAIL(param) do {\
    if(!parameters.HasLabel(param)) { \
        LOGERR("Parameter \"%s\" is not found", param); \
        returnResponse(false); \
    } } while(0)

#define CHECK_SAP_CONFIG_RETURN_ON_FAIL(param) do {\
    if(!config.HasLabel(param)) { \
        LOGERR("Parameter \"%s\" is not found", param); \
        returnResponse(false); \
    } } while(0)

#define getNumberConfigParameter(paramName, param) { \
    if (Core::JSON::Variant::type::NUMBER == config[paramName].Content()) \
        param = config[paramName].Number(); \
    else \
        try { param = std::stoi( config[paramName].String()); } \
        catch (...) { param = 0; } \
}


namespace WPEFramework {
namespace Plugin {

    class SystemAudioPlayerImplementation : public Exchange::ISystemAudioPlayer, public SAPEventCallback {
    public:
        enum Event {
                ONSAPEVENT
            };

        class EXTERNAL Job : public Core::IDispatch {
        protected:
             Job(SystemAudioPlayerImplementation *sap, Event event, string &data)
                : _sap(sap)
                , _event(event)
                , _data(data) {
                if (_sap != nullptr) {
                    _sap->AddRef();
                }
            }

       public:
            Job() = delete;
            Job(const Job&) = delete;
            Job& operator=(const Job&) = delete;
            ~Job() {
                if (_sap != nullptr) {
                    _sap->Release();
                }
            }

       public:
            static Core::ProxyType<Core::IDispatch> Create(SystemAudioPlayerImplementation *sap, Event event, string data) {
#ifndef USE_THUNDER_R4
                return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(sap, event, data)));
#else
                return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create(sap, event, data)));
#endif /* USE_THUNDER_R4 */
            }

            virtual void Dispatch() {
                _sap->Dispatch(_event, _data);
            }

        private:
            SystemAudioPlayerImplementation *_sap;
            const Event _event;
            const string _data;
        };

    public:
        // We do not allow this plugin to be copied !!
        SystemAudioPlayerImplementation(const SystemAudioPlayerImplementation&) = delete;
        SystemAudioPlayerImplementation& operator=(const SystemAudioPlayerImplementation&) = delete;

        virtual uint32_t Configure(PluginHost::IShell* service);
        virtual void Register(INotification* sink) override ;
        virtual void Unregister(INotification* sink) override ;

        virtual uint32_t Open(const string &input, string &output /* @out */) override ;
        virtual uint32_t Play(const string &input, string &output /* @out */) override ;
        virtual uint32_t PlayBuffer(const string &input, string &output /* @out */) override ;
        virtual uint32_t Pause(const string &input, string &output /* @out */) override ;
        virtual uint32_t Resume(const string &input, string &output /* @out */) override ;
        virtual uint32_t Stop(const string &input, string &output /* @out */) override ;
        virtual uint32_t Close(const string &input, string &output /* @out */) override ;
        virtual uint32_t SetMixerLevels(const string &input, string &output /* @out */) override ;
        virtual uint32_t SetSmartVolControl(const string &input, string &output /* @out */) override ;
        virtual uint32_t IsPlaying(const string &input, string &output /* @out */) override ;
	virtual uint32_t Config(const string &input, string &output /* @out */) override ;
        virtual uint32_t GetPlayerSessionId(const string &input, string &output /* @out */) override ;

        virtual void onSAPEvent(uint32_t id,std::string message) override; 
      
        BEGIN_INTERFACE_MAP(SystemAudioPlayerImplementation)
        INTERFACE_ENTRY(Exchange::ISystemAudioPlayer)
        END_INTERFACE_MAP

    private:       
        std::map<int, AudioPlayer*> objectMap;
        AudioPlayer* getObjectFromMap(int key);
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::ISystemAudioPlayer::INotification*> _notificationClients;

        void dispatchEvent(Event, JsonObject &params);
        void Dispatch(Event event, string data);
        void OpenMapping(AudioType audioType,SourceType sourceType,PlayMode mode,int &playerid);
        bool GetSessionFromUrl(string url,int &playerid);
        bool SameModeNotPlaying(AudioPlayer*,int &playerid);
        bool CloseMapping(int key);
        impl::SecurityParameters extractSecurityParams(const JsonObject& params) const;

    public:
        SystemAudioPlayerImplementation();
        virtual ~SystemAudioPlayerImplementation();        
        friend class Job;
    };

} // namespace Plugin
} // namespace WPEFramework
