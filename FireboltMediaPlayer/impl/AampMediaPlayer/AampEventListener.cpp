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

#include "AampEventListener.h"
#include "AampMediaStream.h"
#include "UtilsLogging.h"

namespace WPEFramework {

    namespace Plugin {


        AampEventListener::AampEventListener(AampMediaStream& parent)
        : _parent(parent)
        {
        }

        AampEventListener::~AampEventListener()
        {
        }

        void AampEventListener::Event(const AAMPEvent& event)
        {
            LOGINFO("Event: handling event: %d", event.type);
            switch(event.type)
            {
            case AAMP_EVENT_TUNED:
                HandlePlaybackStartedEvent();
                break;
            case AAMP_EVENT_TUNE_FAILED:
                HandlePlaybackFailed(event);
                break;
            case AAMP_EVENT_SPEED_CHANGED:
                HandlePlaybackSpeedChanged(event);
                break;
            case AAMP_EVENT_PROGRESS:
                HandlePlaybackProgressUpdateEvent(event);
                break;
            case AAMP_EVENT_STATE_CHANGED:
                HandlePlaybackStateChangedEvent(event);
                break;
            case AAMP_EVENT_BUFFERING_CHANGED:
                HandleBufferingChangedEvent(event);
                break;
            default:
                LOGWARN("Event: AAMP event is not supported: %d", event.type);
            }
        }

        void AampEventListener::HandlePlaybackStartedEvent()
        {
            _parent.SendEvent(_T("playbackStarted"), string());
        }

        void AampEventListener::HandlePlaybackStateChangedEvent(const AAMPEvent& event)
        {
            JsonObject parameters;
            parameters[_T("state")] = static_cast<int>(event.data.stateChanged.state);

            string s;
            parameters.ToString(s);
            _parent.SendEvent(_T("playbackStateChanged"), s);
        }

        void AampEventListener::HandlePlaybackProgressUpdateEvent(const AAMPEvent& event)
        {
            JsonObject parameters;
            parameters[_T("durationMiliseconds")] = static_cast<int>(event.data.progress.durationMiliseconds);
            parameters[_T("positionMiliseconds")] = static_cast<int>(event.data.progress.positionMiliseconds);
            parameters[_T("playbackSpeed")] = static_cast<int>(event.data.progress.playbackSpeed);
            parameters[_T("startMiliseconds")] = static_cast<int>(event.data.progress.startMiliseconds);
            parameters[_T("endMiliseconds")] = static_cast<int>(event.data.progress.endMiliseconds);

            string s;
            parameters.ToString(s);
            _parent.SendEvent(_T("playbackProgressUpdate"), s);
        }

        void AampEventListener::HandleBufferingChangedEvent(const AAMPEvent& event)
        {
            JsonObject parameters;
            parameters[_T("buffering")] = event.data.bufferingChanged.buffering;

            string s;
            parameters.ToString(s);
            _parent.SendEvent(_T("bufferingChanged"), s);
        }

        void AampEventListener::HandlePlaybackSpeedChanged(const AAMPEvent& event)
        {
            JsonObject parameters;
            parameters[_T("speed")] = event.data.speedChanged.rate;

            string s;
            parameters.ToString(s);
            _parent.SendEvent(_T("playbackSpeedChanged"), s);
        }

        void AampEventListener::HandlePlaybackFailed(const AAMPEvent& event)
        {
            JsonObject parameters;
            parameters[_T("shouldRetry")] = event.data.mediaError.shouldRetry;
            parameters[_T("code")] = event.data.mediaError.code;
            parameters[_T("description")] = string(event.data.mediaError.description);

            string s;
            parameters.ToString(s);
            _parent.SendEvent(_T("playbackFailed"), s);
        }

    }
}
