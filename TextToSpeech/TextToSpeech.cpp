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

#include "TextToSpeech.h"

#define TTS_MAJOR_VERSION 1
#define TTS_MINOR_VERSION 0
#define API_VERSION_NUMBER 1

namespace WPEFramework {
namespace Plugin {

    /*
     *Register TextToSpeech module as wpeframework plugin
     **/
    SERVICE_REGISTRATION(TextToSpeech, TTS_MAJOR_VERSION, TTS_MINOR_VERSION);

    const string TextToSpeech::Initialize(PluginHost::IShell* service)
    {
        ASSERT(_service == nullptr);

        _connectionId = 0;
        _service = service;
        _skipURL = static_cast<uint8_t>(_service->WebPrefix().length());

        _service->Register(&_notification);

        _tts = _service->Root<Exchange::ITextToSpeech>(_connectionId, 5000, _T("TextToSpeechImplementation"));

        std::string message;
        if(_tts != nullptr) {
            ASSERT(_connectionId != 0);

            _tts->Configure(_service);
            _tts->Register(&_notification);
            RegisterAll();
        } else {
            message = _T("TextToSpeech could not be instantiated.");
            _service->Unregister(&_notification);
            _service = nullptr;
        }

        return message;
    }

    void TextToSpeech::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);
        ASSERT(_tts != nullptr);

        if (!_tts)
            return;

        _tts->Unregister(&_notification);
        _service->Unregister(&_notification);

        if(_tts->Release() != Core::ERROR_DESTRUCTION_SUCCEEDED) {
            ASSERT(_connectionId != 0);
            TRACE_L1("TextToSpeech Plugin is not properly destructed. %d", _connectionId);

            RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));

            // The process can disappear in the meantime...
            if (connection != nullptr) {
                // But if it did not dissapear in the meantime, forcefully terminate it. Shoot to kill :-)
                connection->Terminate();
                connection->Release();
            }
        }

        // Deinitialize what we initialized..
        _service = nullptr;
        _tts = nullptr;
        m_AclCalled = false;
    }

    TextToSpeech::TextToSpeech()
            : PluginHost::JSONRPC()
            , m_AclCalled(false)
            , _apiVersionNumber(API_VERSION_NUMBER)
            , _notification(this)
    {
    }

    TextToSpeech::~TextToSpeech()
    {
    }

    void TextToSpeech::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(_service != nullptr);
            TTSLOG_WARNING("TextToSpeech::Deactivated - %p", this);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin
} // namespace WPEFramework
