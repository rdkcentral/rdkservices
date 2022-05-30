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

#include "FireboltMediaPlayer.h"
#include "UtilsJsonRpc.h"
#include "Module.h"

namespace WPEFramework {

    namespace Plugin {

        /**
         * @brief Destructor ensures all references are released.
         *
         */
        FireboltMediaPlayer::MediaStreamProxy::~MediaStreamProxy()
        {
        }

        /**
         * @brief Release this instance, destroying if necessary.
         *
         * @return Whether the underlying instance could be released.
         *
         */
        uint32_t FireboltMediaPlayer::MediaStreamProxy::Release()
        {
            auto result = _implementation->Unregister(&_mediaPlayerSink);
            if (result != Core::ERROR_DESTRUCTION_SUCCEEDED)
                LOGERR("Unregister failed: %d", result);
            result = _implementation->Release();
            if (result != Core::ERROR_DESTRUCTION_SUCCEEDED)
                LOGERR("Release failed: %d", result);
            _implementation = nullptr;
            return result;
        }

        SERVICE_REGISTRATION(FireboltMediaPlayer, 1, 0);

        FireboltMediaPlayer::FireboltMediaPlayer()
        : _notification(this)
        , _aampMediaPlayerConnectionId(0)
        , _aampMediaPlayer(nullptr)
        , _mediaStreams()
        {
            RegisterAll();
        }

        FireboltMediaPlayer::~FireboltMediaPlayer()
        {
            UnregisterAll();
        }

        const string FireboltMediaPlayer::Initialize(PluginHost::IShell* service)
        {
            LOGINFO();
            string message;
            _aampMediaPlayerConnectionId = 0;
            _service = service;

            // Register the Process::Notification stuff. The Remote process might die before we get a
            // change to "register" the sink for these events !!! So do it ahead of instantiation.
            _service->Register(&_notification);

            _aampMediaPlayer = _service->Root<Exchange::IMediaPlayer>(_aampMediaPlayerConnectionId, 2000, "AampMediaPlayer");

            if (_aampMediaPlayer) {
                LOGINFO("Successfully instantiated AAMP Player in Firebolt Media Player");

            } else {
                LOGERR("AAMP Player in Firebolt Media Player could not be initialized.");
                message = "AAMP Player Firebolt Media Player could not be initialized.";
                _service->Unregister(&_notification);
            }

            return message;
        }

        void FireboltMediaPlayer::Deinitialize(PluginHost::IShell* service)
        {
            LOGINFO();
            ASSERT(_service == service);
            ASSERT(_aampMediaPlayer != nullptr);

            //release all Streams
            for(auto stream : _mediaStreams)
            {
                stream.second->Release();
                delete stream.second;
            }
            _mediaStreams.clear();

            service->Unregister(&_notification);

            RPC::IRemoteConnection* connection(_service->RemoteConnection(_aampMediaPlayerConnectionId));
            auto const result = _aampMediaPlayer->Release();
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);
            if (connection != nullptr) {
                    connection->Terminate();
                    connection->Release();
            }
            _aampMediaPlayer = nullptr;
            _service = nullptr;
        }

        string FireboltMediaPlayer::Information() const
        {
            LOGINFO();
            // No additional info to report.
            return (string());
        }

        void FireboltMediaPlayer::Deactivated(RPC::IRemoteConnection* connection)
        {
            LOGINFO();
            // This can potentially be called on a socket thread, so the deactivation (wich in turn kills this object) must be done
            // on a seperate thread. Also make sure this call-stack can be unwound before we are totally destructed.
            if (_aampMediaPlayerConnectionId == connection->Id()) {
                LOGINFO("Deactivating");
                ASSERT(_service != nullptr);
                Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
            }
        }

        void FireboltMediaPlayer::RegisterAll()
        {
            Register(_T("create"), &FireboltMediaPlayer::create, this);
            Register(_T("release"), &FireboltMediaPlayer::release, this);
            Register(_T("load"), &FireboltMediaPlayer::load, this);
            Register(_T("play"), &FireboltMediaPlayer::play, this);
            Register(_T("pause"), &FireboltMediaPlayer::pause, this);
            Register(_T("seek"), &FireboltMediaPlayer::seek, this);
            Register(_T("stop"), &FireboltMediaPlayer::stop, this);
            Register(_T("initConfig"), &FireboltMediaPlayer::initConfig, this);
            Register(_T("setDRMConfig"), &FireboltMediaPlayer::setDRMConfig, this);
        }

        void FireboltMediaPlayer::UnregisterAll()
        {
            Unregister(_T("create"));
            Unregister(_T("release"));
            Unregister(_T("load"));
            Unregister(_T("play"));
            Unregister(_T("pause"));
            Unregister(_T("seek"));
            Unregister(_T("stop"));
            Unregister(_T("initConfig"));
            Unregister(_T("setDRMConfig"));
        }

        uint32_t FireboltMediaPlayer::create(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            const char *keyId = "id";
            returnIfStringParamNotFound(parameters, keyId);

            if(parameters[keyId].String().empty())
            {
                LOGERR("Argument \'%s\' is empty", keyId);
                returnResponse(false);
            }

            string id = parameters[keyId].String();

            if(_mediaStreams.find(id) == _mediaStreams.end())
            {
                //create new stream
                Exchange::IMediaPlayer::IMediaStream *stream = _aampMediaPlayer->CreateStream(id);
                if(!stream)
                {
                    LOGERR("Error creating stream with id '%s'", id.c_str());
                    returnResponse(false);
                }

                _mediaStreams[id] = new MediaStreamProxy(*this, id, stream);
            }
            else
            {
                // we have already such stream created (no need to AddRef one more time)
                returnResponse(true);
            }

            returnResponse(true);
        }

        uint32_t FireboltMediaPlayer::release(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            const char *keyId = "id";
            returnIfStringParamNotFound(parameters, keyId);

            string id = parameters[keyId].String();

            if(_mediaStreams.find(id) != _mediaStreams.end())
            {
                if(_mediaStreams[id]->Release() == Core::ERROR_DESTRUCTION_SUCCEEDED)
                {
                    delete _mediaStreams[id];
                    _mediaStreams.erase(id);
                }
                returnResponse(true);
            }

            LOGERR("Instance '%s' does not exis", id.c_str());
            returnResponse(false);
        }

        uint32_t FireboltMediaPlayer::load(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            const char *keyId = "id";
            const char *keyUrl = "url";
            const char *keyAutoPlay = "autoplay";
            returnIfStringParamNotFound(parameters, keyId);
            returnIfParamNotFound(parameters, keyUrl);

            string id = parameters[keyId].String();
            if(_mediaStreams.find(id) == _mediaStreams.end())
            {
                LOGERR("Instace \'%s\' does not exist", id.c_str());
                returnResponse(false);
            }

            string url = parameters[keyUrl].Value();

            bool autoPlay = true;
            if(parameters.HasLabel(keyAutoPlay))
            {
                autoPlay = parameters[keyAutoPlay].Boolean();
            }

            returnResponse(_mediaStreams[id]->Stream()->Load(url, autoPlay) == Core::ERROR_NONE);
        }

        uint32_t FireboltMediaPlayer::play(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            const char *keyId = "id";
            returnIfStringParamNotFound(parameters, keyId);

            string id = parameters[keyId].String();
            if(_mediaStreams.find(id) == _mediaStreams.end())
            {
                LOGERR("Instace \'%s\' does not exist", id.c_str());
                returnResponse(false);
            }

            returnResponse(_mediaStreams[id]->Stream()->SetRate(100) == Core::ERROR_NONE);
        }

        uint32_t FireboltMediaPlayer::pause(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            const char *keyId = "id";
            returnIfStringParamNotFound(parameters, keyId);
            string id = parameters[keyId].String();
            if(_mediaStreams.find(id) == _mediaStreams.end())
            {
                LOGERR("Instace \'%s\' does not exist", id.c_str());
                returnResponse(false);
            }

            returnResponse(_mediaStreams[id]->Stream()->SetRate(0) == Core::ERROR_NONE);
        }

        uint32_t FireboltMediaPlayer::seek(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            const char *keyId = "id";
            const char *keyPositionSec = "positionSec";
            returnIfStringParamNotFound(parameters, keyId);
            returnIfNumberParamNotFound(parameters, keyPositionSec);
            string id = parameters[keyId].String();
            if(_mediaStreams.find(id) == _mediaStreams.end())
            {
                LOGERR("Instace \'%s\' does not exist", id.c_str());
                returnResponse(false);
            }

            int positionSec = parameters[keyPositionSec].Number();
            returnResponse(_mediaStreams[id]->Stream()->SeekTo(positionSec) == Core::ERROR_NONE);
        }

        uint32_t FireboltMediaPlayer::stop(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            const char *keyId = "id";
            returnIfStringParamNotFound(parameters, keyId);
            string id = parameters[keyId].String();
            if(_mediaStreams.find(id) == _mediaStreams.end())
            {
                LOGERR("Instace \'%s\' does not exist", id.c_str());
                returnResponse(false);
            }

            returnResponse(_mediaStreams[id]->Stream()->Stop() == Core::ERROR_NONE);
        }

        uint32_t FireboltMediaPlayer::initConfig(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            const char *keyId = "id";
            returnIfStringParamNotFound(parameters, keyId);
            string id = parameters[keyId].String();
            MediaStreams::const_iterator it = _mediaStreams.find(id);
            if (it == _mediaStreams.end())
            {
                LOGERR("Instance '%s' does not exist", id.c_str());
                returnResponse(false);
            }

            // Ideally I'd like to remove 'id' with Core::JSON::Conainer::Remove but its broken in current release
            string parametersStr;
            if (!parameters.ToString(parametersStr)) 
            {
                LOGERR("Failed to serialize parameters into a string");
                returnResponse(false);
            }
            returnResponse((*it).second->Stream()->InitConfig(parametersStr) == Core::ERROR_NONE);
        }

        uint32_t FireboltMediaPlayer::setDRMConfig(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            const char *keyId = "id";
            returnIfStringParamNotFound(parameters, keyId);
            string id = parameters[keyId].String();
            MediaStreams::const_iterator it = _mediaStreams.find(id);
            if (it == _mediaStreams.end()) {
                LOGERR("Instance '%s' does not exist", id.c_str());
                returnResponse(false);
            }

            // Duplicate parameters and remove id
            JsonObject parametersWithoutId(parameters);
            parametersWithoutId.Remove(keyId);
            string parametersWithoutIdStr;
            if (!parametersWithoutId.ToString(parametersWithoutIdStr)) 
            {
                LOGERR("Failed to serialize parameters into a string");
                returnResponse(false);
            }

            returnResponse((*it).second->Stream()->InitDRMConfig(parametersWithoutIdStr) == Core::ERROR_NONE);
        }

        void FireboltMediaPlayer::onMediaStreamEvent(const string& id, const string &eventName, const string &parametersJson)
        {
            JsonObject parametersJsonObjWithId;
            parametersJsonObjWithId[id.c_str()] = JsonObject(parametersJson);

            // Notify to all with:
            // params : { "<id>" : { <parametersJson> } }
            sendNotify(eventName.c_str(), parametersJsonObjWithId);
        }

    }
}
