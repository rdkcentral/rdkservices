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

#include "AampMediaStream.h"
#include "ocdm/open_cdm.h"
#include "utils.h"
namespace WPEFramework {
    namespace Plugin {


        AampMediaStream::AampMediaStream()
        : _adminLock()
        , _notification(nullptr)
        , _aampPlayer(nullptr)
        , _aampEventListener(nullptr)
        , _aampGstPlayerMainLoop(nullptr)
        {
            gst_init(0, nullptr);
            _aampPlayer = new PlayerInstanceAAMP();
            if(_aampPlayer == nullptr)
            {
                return;
            }

            _aampEventListener = new AampEventListener(*this);
            if(_aampEventListener == nullptr)
            {
                return;
            }
            _aampPlayer->RegisterEvents(_aampEventListener);
            _aampPlayer->SetReportInterval(1000 /* ms */);

            _aampGstPlayerMainLoop = g_main_loop_new(nullptr, false);

            // Run thread with _aampGstPlayerMainLoop
            Run();
        }

        AampMediaStream::~AampMediaStream()
        {
            _adminLock.Lock();
            if(!_aampPlayer)
            {
                return;
            }

            if (_notification != nullptr) {
                _notification->Release();
                _notification = nullptr;
            }

            _adminLock.Unlock();
            _aampPlayer->Stop();
            Block();
            _adminLock.Lock();

            if(_aampGstPlayerMainLoop)
            {
                g_main_loop_quit(_aampGstPlayerMainLoop);
            }

            _adminLock.Unlock();
            Wait(Thread::BLOCKED | Thread::STOPPED, Core::infinite);

            _adminLock.Lock();
            _aampPlayer->Stop();
            _aampPlayer->RegisterEvents(nullptr);
            delete _aampEventListener;
            _aampEventListener = nullptr;
            delete _aampPlayer;
            _aampPlayer = nullptr;
            _adminLock.Unlock();
            opencdm_deinit();
        }

        uint32_t AampMediaStream::Load(const string& url, bool autoPlay)
        {
            LOGINFO("Load with url=%s, autoPlay=%d", url.c_str(), autoPlay);
            _adminLock.Lock();

            ASSERT(_aampPlayer != nullptr);
            _aampPlayer->Tune(url.c_str(), autoPlay);

            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::SetRate(int32_t rate)
        {
            LOGINFO("SetRate with rate=%d", rate);
            double dRate = rate;
            dRate /= 100;

            _adminLock.Lock();
            ASSERT(_aampPlayer != nullptr);
            _aampPlayer->SetRate(dRate);
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::SeekTo(int32_t positionSec)
        {
            LOGINFO("SetPosition with pos=%d sec", positionSec);
            _adminLock.Lock();
            ASSERT(_aampPlayer != nullptr);
            _aampPlayer->Seek(static_cast<double>(positionSec));
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::Stop()
        {
            LOGINFO("Stop");
            _adminLock.Lock();
            ASSERT(_aampPlayer != nullptr);
            _aampPlayer->Stop();
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::InitConfig(const string& configurationJson)
        {
            LOGINFO("InitConfig with config=%s", configurationJson.c_str());
            _adminLock.Lock();
            //TODO: no parameter supported right now
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::InitDRMConfig(const string& configurationJson)
        {
            LOGINFO("InitDRMConfig with config=%s", configurationJson.c_str());
            _adminLock.Lock();
            //TODO: no parameter supported right now
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::Register(Exchange::IMediaPlayer::IMediaStream::INotification* notification)
        {
            LOGINFO();
            _adminLock.Lock();

            if (_notification != nullptr) {
                _notification->Release();
            }
            if (notification != nullptr) {
                notification->AddRef();
            }
            _notification = notification;
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::Unregister(Exchange::IMediaPlayer::IMediaStream::INotification* notification)
        {
            LOGINFO();
            _adminLock.Lock();

            if (_notification != nullptr
                    && _notification == notification) {
                _notification->Release();
                notification->Release();
                _notification = nullptr;
            }
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        void AampMediaStream::SendEvent(const string& eventName, const string& parameters)
        {
            LOGINFO("eventName=%s, parameters=%s", eventName.c_str(), parameters.c_str());
            _adminLock.Lock();
            if(!_notification)
            {
                LOGERR("SendEvent: notification callback is null");
                _adminLock.Unlock();
                return;
            }
            _notification->Event(eventName, parameters);
            _adminLock.Unlock();
        }

        // Thread overrides
        uint32_t AampMediaStream::Worker()
        {
            if (_aampGstPlayerMainLoop) {
                g_main_loop_run(_aampGstPlayerMainLoop); // blocks
                g_main_loop_unref(_aampGstPlayerMainLoop);
                _aampGstPlayerMainLoop = nullptr;
            }
            return WPEFramework::Core::infinite;
        }

    }
}
