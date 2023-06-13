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

#include "AampMediaPlayer.h"
#include "AampMediaStream.h"

#include "UtilsLogging.h"

namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(AampMediaPlayer, 1, 0);

        AampMediaPlayer::AampMediaPlayer()
        {
            //temporary back door for AAMP configuration
            Core::SystemInfo::SetEnvironment(_T("AAMP_ENABLE_OPT_OVERRIDE"), _T("1"));
            //TODO: should be set accordingly to platform set-up
            Core::SystemInfo::SetEnvironment(_T("AAMP_ENABLE_WESTEROS_SINK"), _T("1"), false);
        }

        AampMediaPlayer::~AampMediaPlayer()
        {
        }

        Exchange::IMediaPlayer::IMediaStream* AampMediaPlayer::CreateStream(const string& id)
        {
            LOGINFO("Create with id: %s", id.c_str());
            return Core::Service<AampMediaStream>::Create<IMediaPlayer::IMediaStream>();
        }

    }//Plugin
}//WPEFramework
