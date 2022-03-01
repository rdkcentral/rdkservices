/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include "Module.h"
#include "libIARM.h"
#include "libIBus.h"

class socket_adaptor;

namespace WPEFramework {
    namespace Plugin {
        class DataCapture : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        public:
            DataCapture();
            virtual ~DataCapture();
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;
            void eventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            BEGIN_INTERFACE_MAP(MODULE_NAME)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public/*members*/:
            static DataCapture* _instance;

        public /*constants*/:
            static const string SERVICE_NAME;
            static const string METHOD_ENABLE_AUDIO_CAPTURE;
            static const string METHOD_GET_AUDIO_CLIP;
            static const string EVT_ON_AUDIO_CLIP_READY;

        private/*registered methods*/:
            //methods
            uint32_t enableAudioCaptureWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getAudioClipWrapper(const JsonObject& parameters, JsonObject& response);

        private/*internal methods*/:
            DataCapture(const DataCapture&) = delete;
            DataCapture& operator=(const DataCapture&) = delete;

            static void InitializeIARM();
            static void DeinitializeIARM();
            static void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *eventData, size_t len);

            int enableAudioCapture(unsigned int bufferMaxDuration);
            int getAudioClip(const JsonObject& clipRequest);
            void constructFormatString();
            bool uploadDataToUrl(std::vector<unsigned char> &data, const char *url, std::string &error_str);
        private/*members*/:
            audiocapturemgr::session_id_t _session_id;
            unsigned int _max_supported_duration;
            socket_adaptor* _sock_adaptor;
            audiocapturemgr::audio_properties_ifce_t _audio_properties;
            string _audio_format_string;
            string _destination_url;
            bool _is_precapture;
            unsigned int _duration;
            static pthread_mutex_t _mutex;
        };
    } // namespace Plugin
} // namespace WPEFramework
