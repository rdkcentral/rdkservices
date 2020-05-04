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

#include "SoundPlayer.h"

#if defined(PLATFORM_BROADCOM)
#include "gstreamersoundplayer_bcm.h"
#elif defined(PLATFORM_INTEL)
#include "gstreamersoundplayer_intel.h"
#endif

#include "utils.h"

#define SOUND_PLAYER_METHOD_SET_PROPERTIES "setProperties"
#define SOUND_PLAYER_METHOD_SET_URL "setUrl"
#define SOUND_PLAYER_METHOD_SET_VOLUME "setVolume"
#define SOUND_PLAYER_METHOD_SET_POSITION "setPosition"
#define SOUND_PLAYER_METHOD_SET_AUTO_PLAY "setAutoPlay"
#define SOUND_PLAYER_METHOD_GET_PROPERTIES "getProperties"
#define SOUND_PLAYER_METHOD_GET_URL "getUrl"
#define SOUND_PLAYER_METHOD_GET_VOLUME "getVolume"
#define SOUND_PLAYER_METHOD_GET_POSITION "getPosition"
#define SOUND_PLAYER_METHOD_GET_AUTO_PLAY "getAutoPlay"
#define SOUND_PLAYER_METHOD_PLAY "play"
#define SOUND_PLAYER_METHOD_PAUSE "pause"
#define SOUND_PLAYER_METHOD_GET_HEADERS "getHeaders"
#define SOUND_PLAYER_METHOD_SET_HEADERS "setHeaders"

#define SOUND_PLAYER_EVT_ON_STREAM_PLAYING "onStreamPlaying"
#define SOUND_PLAYER_EVT_ON_STREAM_PAUSED "onStreamPaused"
#define SOUND_PLAYER_EVT_ON_STREAM_PROGRESS "onStreamProgress"
#define SOUND_PLAYER_EVT_ON_STREAM_CLOSED "onStreamClosed"
#define SOUND_PLAYER_EVT_ON_READY "onReady"
#define SOUND_PLAYER_EVT_ON_ERROR "onError"

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(SoundPlayer, 1, 0);

        SoundPlayer* SoundPlayer::_instance = nullptr;

        SoundPlayer::SoundPlayer()
        : AbstractPlugin()
        , m_volume(100)
        , m_position(0)
        , m_currentPosition(0)
        , m_autoPlay(true)
        , m_item(NULL)
        , m_itemCreated(false)
        , m_itemReady(false)
        {
            LOGINFO();
            SoundPlayer::_instance = this;

            registerMethod(SOUND_PLAYER_METHOD_SET_PROPERTIES, &SoundPlayer::setPropertiesWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_SET_URL, &SoundPlayer::setUrlWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_SET_VOLUME, &SoundPlayer::setVolumeWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_SET_POSITION, &SoundPlayer::setPositionWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_SET_AUTO_PLAY, &SoundPlayer::setAutoPlayWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_GET_PROPERTIES, &SoundPlayer::getPropertiesWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_GET_URL, &SoundPlayer::getUrlWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_GET_VOLUME, &SoundPlayer::getVolumeWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_GET_POSITION, &SoundPlayer::getPositionWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_GET_AUTO_PLAY, &SoundPlayer::getAutoPlayWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_PLAY, &SoundPlayer::playWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_PAUSE, &SoundPlayer::pauseWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_GET_HEADERS, &SoundPlayer::getHeadersWrapper, this);
            registerMethod(SOUND_PLAYER_METHOD_SET_HEADERS, &SoundPlayer::setHeadersWrapper, this);
        }

        SoundPlayer::~SoundPlayer()
        {
            LOGINFO();
            SoundPlayer::_instance = nullptr;
        }

        uint32_t SoundPlayer::setPropertiesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            if (!parameters.HasLabel("url") || !parameters.HasLabel("volume") || !parameters.HasLabel("position") || !parameters.HasLabel("autoPlay"))
            {
                response["error"] = "params missing";
                returnResponse(false);
            }
            if (!m_url.empty())
            {
                response["error"] = "url already set";
                returnResponse(false);
            }
            std::string url = parameters["url"].String();
            if (url.empty())
            {
                response["error"] = "url is empty";
                returnResponse(false);
            }

            m_url = url;

            getNumberParameter("volume", m_volume);

            getNumberParameter("position", m_position);

            getBoolParameter("autoPlay", m_autoPlay);

            LOGINFO("setProperties Url:%s Volume:%d Position:%d AutoPlay:%s",
                       m_url.c_str(),
                       m_volume,
                       m_position,
                       m_autoPlay ? "true" : "false");

            onServiceSetUrl(m_url);

            returnResponse(true);
        }

        uint32_t SoundPlayer::setUrlWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            if (!parameters.HasLabel("url"))
            {
                response["error"] = "params missing";
                returnResponse(false);

            }

            /*if (!m_url.empty())
            {
                response["error"] = "url already set";
                returnResponse(false);
            }*/

            std::string url = parameters["url"].String();
            if (url.empty())
            {
                response["error"] = "url is empty";
                returnResponse(false);
            }

            m_url = url;
            LOGINFO("setUrl Url:%s", m_url.c_str());
            onServiceSetUrl(m_url);

            returnResponse(true);
        }

        uint32_t SoundPlayer::setVolumeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            if (!parameters.HasLabel("volume"))
            {
                response["error"] = "params missing";
                returnResponse(false);

            }

            getNumberParameter("volume", m_volume);
            LOGINFO("setVolume %d", m_volume);
            onServiceSetVolume(m_volume);

            returnResponse(true);
        }

        uint32_t SoundPlayer::setPositionWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            if (!parameters.HasLabel("position"))
            {
                response["error"] = "params missing";
                returnResponse(false);

            }

            getNumberParameter("position", m_position);
            LOGINFO("setPosition %d", m_position);
            onServiceSetPosition(m_position);

            returnResponse(true);
        }

        uint32_t SoundPlayer::setAutoPlayWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            if (!parameters.HasLabel("autoPlay"))
            {
                response["error"] = "params missing";
                returnResponse(false);

            }

            getBoolParameter("autoPlay", m_autoPlay);
            LOGINFO("setAutoPlay %s", m_autoPlay ? "true" : "false");
            onServiceSetAutoPlay(m_autoPlay);

            returnResponse(true);
        }

        uint32_t SoundPlayer::getPropertiesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            response["url"] = m_url;
            response["volume"] = m_volume;
            response["position"] = m_currentPosition;
            response["autoPlay"] = m_autoPlay;

            returnResponse(true);
        }


        uint32_t SoundPlayer::getUrlWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            response["url"] = m_url;

            returnResponse(true);
        }

        uint32_t SoundPlayer::getVolumeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            response["volume"] = m_volume;

            returnResponse(true);
        }

        uint32_t SoundPlayer::getPositionWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            response["position"] = m_currentPosition;

            returnResponse(true);
        }

        uint32_t SoundPlayer::getAutoPlayWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            response["autoPlay"] = m_autoPlay;

            returnResponse(true);
        }

        uint32_t SoundPlayer::playWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();
            onServicePlay();
            returnResponse(true);
        }

        uint32_t SoundPlayer::pauseWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();
            onServicePause();
            returnResponse(true);
        }

        uint32_t SoundPlayer::getHeadersWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            JsonObject headers;

            for (std::map <std::string, std::string>::const_iterator it = m_headers.cbegin(); it != m_headers.cend(); it++)
                headers[it->first.c_str()] = it->second;

            response["headers"] = headers;
            returnResponse(true);
        }

        uint32_t SoundPlayer::setHeadersWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            if (!parameters.HasLabel("headers"))
            {
                response["error"] = "'headers' param missing";
                returnResponse(false);
            }

            JsonObject headers = parameters["headers"].Object();

            JsonObject::Iterator index(headers.Variants());

            while (index.Next() == true)
                m_headers[index.Label()] = index.Current().String();

            returnResponse(true);
        }

        void SoundPlayer::createSoundItem()
        {
            LOGWARN("createSoundItem %d", m_itemCreated);
            if (!m_itemCreated)
            {
                CGStreamerSoundPlayer* item = new CGStreamerSoundPlayer(this);
                m_itemCreated = true;
                m_item = item;
            }
        }


        void SoundPlayer::onServiceSetUrl(const std::string& url)
        {
            LOGWARN("onServiceSetUrl %s", url.c_str());
            // TODO use proxy ?
            createSoundItem();

            if (m_item)
                m_item->onResourceSetSoundFile(url, false, "", 0, m_headers);
        }

        void SoundPlayer::onServiceSetVolume(unsigned volume)
        {
            LOGWARN("onServiceSetVolume %d", volume);
            m_volume = volume;
            if (m_itemReady && m_item)
                m_item->onResourceSetVolume(volume);
        }

        void SoundPlayer::onServiceSetPosition(unsigned position)
        {
            LOGWARN("onServiceSetPosition %d", position);
            m_position = position;
            if (m_itemReady && m_item)
                m_item->onResourceSetPosition(position);
        }

        void SoundPlayer::onServiceSetAutoPlay(bool autoPlay)
        {
            LOGWARN("onServiceSetAutoPlay %s", autoPlay ? "true" : "false");
            m_autoPlay = autoPlay;
        }

        void SoundPlayer::onServicePlay()
        {
            LOGWARN("onServicePlay");
            if (m_itemReady && m_item)
                m_item->onResourcePlay();
        }

        void SoundPlayer::onServicePause()
        {
            LOGWARN("onServicePause");
            if (m_itemReady && m_item)
                m_item->onResourcePause();
        }

        void SoundPlayer::onItemOpen()
        {
            LOGWARN("onItemOpen");

            m_itemReady = true;

            JsonObject params;
            sendNotify(SOUND_PLAYER_EVT_ON_READY, params);

            if (m_volume != 100)
                m_item->onResourceSetVolume(m_volume);
            if (m_position != 0)
                m_item->onResourceSetPosition(m_position);
            if (m_autoPlay)
                m_item->onResourcePlay();
        }

        void SoundPlayer::onItemError(std::string error)
        {
            LOGWARN("onItemError %s", error.c_str());
            m_itemReady = false;

            JsonObject params;
            params["description"] = error;
            sendNotify(SOUND_PLAYER_EVT_ON_ERROR, params);

        }

        void SoundPlayer::onItemProgress(int64_t duration, int64_t time)
        {
            LOGINFO("reportProgress %lld, %lld", duration, time);
            m_currentPosition = time;

            JsonObject params;

            params["duration"] = duration;
            params["time"] = time;
            sendNotify(SOUND_PLAYER_EVT_ON_STREAM_PROGRESS, params);
        }

        void SoundPlayer::onItemComplete(int64_t position)
        {
            LOGWARN("onItemComplete %lld", position);
            JsonObject params;
            sendNotify(SOUND_PLAYER_EVT_ON_STREAM_CLOSED, params);
        }

        void SoundPlayer::onItemPlaying()
        {
            LOGWARN("onItemPlaying");
            JsonObject params;
            sendNotify(SOUND_PLAYER_EVT_ON_STREAM_PLAYING, params);
        }

        void SoundPlayer::onItemPaused(int64_t position)
        {
            LOGWARN("onItemPaused %lld", position);
            JsonObject params;
            sendNotify(SOUND_PLAYER_EVT_ON_STREAM_PAUSED, params);
        }
    } // namespace Plugin
} // namespace WPEFramework
