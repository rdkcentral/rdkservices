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
#include "AampEventListener.h"

#include <interfaces/IMediaPlayer.h>
#include <gst/gst.h>
#include <main_aamp.h>

namespace WPEFramework {
    namespace Plugin {

        class AampMediaStream : public Exchange::IMediaPlayer::IMediaStream, Core::Thread {
        public:
            AampMediaStream();
            ~AampMediaStream() override;

            AampMediaStream(const AampMediaStream&) = delete;
            AampMediaStream& operator=(const AampMediaStream&) = delete;

            // IMediaStream Interfaces
            virtual uint32_t Load(const string& url, bool autoPlay) override;
            virtual uint32_t SetRate(int32_t rate) override;
            virtual uint32_t SeekTo(int32_t positionSec) override;
            virtual uint32_t Stop() override;
            virtual uint32_t InitConfig(const string& configurationJson) override;
            virtual uint32_t InitDRMConfig(const string& configurationJson) override;
            virtual uint32_t Register(Exchange::IMediaPlayer::IMediaStream::INotification* callback) override;
            virtual uint32_t Unregister(Exchange::IMediaPlayer::IMediaStream::INotification* callback) override;

            BEGIN_INTERFACE_MAP(AampMediaStream)
            INTERFACE_ENTRY(Exchange::IMediaPlayer::IMediaStream)
            END_INTERFACE_MAP

            void SendEvent(const string& eventName, const string& parameters);

        private:
            typedef struct _GMainLoop GMainLoop;

            // Thread Interface
            uint32_t Worker() override;

            mutable Core::CriticalSection _adminLock;
            Exchange::IMediaPlayer::IMediaStream::INotification *_notification;
            PlayerInstanceAAMP *_aampPlayer;
            AampEventListener *_aampEventListener;
            GMainLoop *_aampGstPlayerMainLoop;
        };

    }
}
