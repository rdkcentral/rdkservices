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
#include <interfaces/IMediaPlayer.h>

namespace WPEFramework {
    namespace Plugin {

        class AampMediaPlayer : public Exchange::IMediaPlayer{

        public:
            AampMediaPlayer();
            ~AampMediaPlayer() override;

            AampMediaPlayer(const AampMediaPlayer&) = delete;
            AampMediaPlayer& operator=(const AampMediaPlayer&) = delete;

            // IMediaPlayer Interfaces
            virtual Exchange::IMediaPlayer::IMediaStream* CreateStream(const string& id) override;

            BEGIN_INTERFACE_MAP(AampMediaPlayer)
            INTERFACE_ENTRY(Exchange::IMediaPlayer)
            END_INTERFACE_MAP
        };
    } //Plugin
} //WPEFramework

