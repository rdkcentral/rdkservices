/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright Synamedia, All rights reserved
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

#include "UnifiedPlayer.h"


namespace WPEFramework
{

    ENUM_CONVERSION_BEGIN(JsonData::UnifiedPlayer::ModeType)
    { JsonData::UnifiedPlayer::ModeType::MODE_NONE, _TXT("MODE_NONE") },
    { JsonData::UnifiedPlayer::ModeType::MODE_LIVE, _TXT("MODE_LIVE") },
    { JsonData::UnifiedPlayer::ModeType::MODE_RECORD, _TXT("MODE_RECORD") },
    { JsonData::UnifiedPlayer::ModeType::MODE_PLAYBACK, _TXT("MODE_PLAYBACK") },
    ENUM_CONVERSION_END(WPEFramework::JsonData::UnifiedPlayer::ModeType)

    ENUM_CONVERSION_BEGIN(JsonData::UnifiedPlayer::ManageType)
    { JsonData::UnifiedPlayer::ManageType::MANAGE_NONE, _TXT("MANAGE_NONE") },
    { JsonData::UnifiedPlayer::ManageType::MANAGE_FULL, _TXT("MANAGE_FULL") },
    { JsonData::UnifiedPlayer::ManageType::MANAGE_NO_PSI, _TXT("MANAGE_NO_PSI") },
    { JsonData::UnifiedPlayer::ManageType::MANAGE_NO_TUNER, _TXT("MANAGE_NO_TUNER") },
    ENUM_CONVERSION_END(JsonData::UnifiedPlayer::ManageType)

    namespace Plugin
    {
        void UnifiedPlayer::convertPlainStringToFloatVector(const std::string& plainChar, std::vector<float>& vec)
        {
            std::string plainStr(plainChar);
            std::stringstream ss(plainStr);
            std::string item;
            float value;
            while(std::getline(ss, item, ','))
            {
                try
                {
                    value = std::stof(item);
                    vec.push_back(value);
                }
                catch(const std::exception& e)
                {
                    LOGERR("Unable to convert %s to float error: %s ", item.c_str(), e.what());
                }
            }
            return;
        }

        void UnifiedPlayer::convertPlainStringToStrVector(const std::string& plainStr, std::vector<std::string>& vec)
        {
            std::stringstream ss(plainStr);
            std::string item;
            while(std::getline(ss, item, ','))
            {
                vec.push_back(item);
            }
            return;
        }

        template<typename ArrayType, typename VectorType>
        void UnifiedPlayer::addToArrayParamter(Core::JSON::ArrayType<ArrayType>& param, std::vector<VectorType>& vec)
        {
            ArrayType item;
            for(auto iter: vec)
            {
                item = iter;
                param.Add(item);
            }
            vec.clear();
            return;
        }

                // Event: onplayerinitialized - Sent when the player has accepted a media URL
        void UnifiedPlayer::event_onplayerinitialized(const uint32& sessionid)
        {
            PlayerinitializedeventInfo params;
            params.Sessionid  = sessionid;
            Notify(_T("onplayerinitialized"));
        }

        // Event: onmediaopened - Sent when media is successfully opened
        void UnifiedPlayer::event_onmediaopened(const uint32_t& sessionid, const string& mediatype, const uint32_t& duration, const uint32_t& width, const uint32_t& height, const std::string& availablespeeds, const std::string& availableaudiolanguages, const std::string& availablesubtitleslanguages)
        {
            LOGDBG("UnifiedPlayer event_onmediaopened called with params");
            std::vector<std::string> strVec;
            std::vector<float> floatVec;
            MediaopenedeventData params;
            params.Mediatype = mediatype;
            params.Duration = duration;
            params.Width = width;
            params.Height = height;
            params.Sessionid  = sessionid;
            // params.Availablespeeds = availablespeeds;
            convertPlainStringToFloatVector(availablespeeds, floatVec);
            addToArrayParamter<Core::JSON::Float, float>(params.Availablespeeds, floatVec);
            convertPlainStringToStrVector(availableaudiolanguages, strVec);
            addToArrayParamter<Core::JSON::String, std::string>(params.Availableaudiolanguages, strVec);
            convertPlainStringToStrVector(availablesubtitleslanguages, strVec);
            addToArrayParamter<Core::JSON::String, std::string>(params.Availablesubtitleslanguages, strVec);

            Notify(_T("onmediaopened"), params);
        }

        // Event: onplaying - Sent when the player transitions to x1 play speed
        void UnifiedPlayer::event_onplaying(const uint32_t& sessionid)
        {
           WarningerroreventInfo params;
           params.Sessionid  = sessionid;
            Notify(_T("onplaying"),params);
        }

        // Event: onpaused - Sent when the player enters a paused state
        void UnifiedPlayer::event_onpaused(const uint32_t& sessionid)
        {
            WarningerroreventInfo params;
            params.Sessionid  = sessionid;
            Notify(_T("onpaused"),params);
        }

        // Event: onclosed - Sent when media is closed
        void UnifiedPlayer::event_onclosed(const uint32_t& sessionid)
        {
            WarningerroreventInfo params;
            params.Sessionid  = sessionid;
            Notify(_T("onclosed"),params);
        }

        // Event: oncomplete - Sent when the stream complete
        void UnifiedPlayer::event_oncomplete(const uint32_t& sessionid)
        {
            WarningerroreventInfo params;
            params.Sessionid  = sessionid;
            Notify(_T("oncomplete"),params);
        }

        // Event: onstatus - Sent when status changes
        void UnifiedPlayer::event_onstatus(const uint32_t&, const float& position, const float& duration)
        {
            StatuseventData params;
            params.Position = position;
            params.Duration = duration;

            Notify(_T("onstatus"), params);
        }

        // Event: onprogress - Sent when progress or duration changes
        void UnifiedPlayer::event_onprogress(const uint32_t& sessionid, const float& position, const float& duration, const float& speed, const float& start, const float& end)
        {
            ProgresseventData params;
            params.Position = position;
            params.Duration = duration;
            params.Speed = speed;
            params.Start = start;
            params.End = end;
            params.Sessionid  = sessionid;

            Notify(_T("onprogress"), params);
        }

        // Event: onwarning - Sent when a warning is issued
        void UnifiedPlayer::event_onwarning(const uint32_t& sessionid, const uint32_t& code, const string& description)
        {
            WarningerroreventInfo params;
            params.Code = code;
            params.Description = description;
            params.Sessionid  = sessionid;

            Notify(_T("onwarning"), params);
        }

        // Event: onerror - Sent when an error is issued
        void UnifiedPlayer::event_onerror(const uint32_t& sessionid, const uint32_t& code, const string& description)
        {
            WarningerroreventInfo params;
            params.Code = code;
            params.Description = description;
            params.Sessionid  = sessionid;
            Notify(_T("onerror"), params);
        }

        // Event: onbitratechange - Sent when current ABR bitrate changes
        void UnifiedPlayer::event_onbitratechange(const uint32_t& sessionid, const uint32_t& bitrate, const string& reason)
        {
            BitratechangeeventData params;
            params.Bitrate = bitrate;
            params.Reason = reason;
            params.Sessionid  = sessionid;

            Notify(_T("onbitratechange"), params);
        }

        // Event: oncasdata - CAS public data is sent asynchronously from CAS System
        void UnifiedPlayer::event_oncasdata(const uint32_t&, const string& casData)
        {
            LOGDBG("UnifiedPlayer::event_oncasdata - Enter\n");
//            XferinfoInfo params;
            LOGDBG("UnifiedPlayer::event_oncasdata - payload = %s\n", casData.c_str());
//            params.Payload = casData;
//            params.Source = SourceType::PUBLIC;
            BitratechangeeventData params;

            Notify(_T("oncasdata"), params);
        }

        // Events begin
        // Events end

    }
}
