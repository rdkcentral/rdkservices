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

#include <main_aamp.h>

namespace WPEFramework {
    namespace Plugin {

        class AampMediaStream;

        class AampEventListener : public AAMPEventListener {
        public:
            AampEventListener(AampMediaStream& parent);
            ~AampEventListener() override;

            AampEventListener() = delete;
            AampEventListener(const AampEventListener&) = delete;
            AampEventListener& operator=(const AampEventListener&) = delete;

            void Event(const AAMPEvent& event) override;

        private:
            void HandlePlaybackStartedEvent();
            void HandlePlaybackStateChangedEvent(const AAMPEvent& event);
            void HandlePlaybackProgressUpdateEvent(const AAMPEvent& event);
            void HandleBufferingChangedEvent(const AAMPEvent& event);
            void HandlePlaybackSpeedChanged(const AAMPEvent& event);
            void HandlePlaybackFailed(const AAMPEvent& event);

            AampMediaStream& _parent;
        };

    }
}
