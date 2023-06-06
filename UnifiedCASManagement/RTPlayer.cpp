/*
 * File:   RTPlayer.h
 * Author: gp
 *
 * Created on Feb 25, 2021
 */

#include "RTPlayer.h"
//#define ENABLE_SESSIONID

namespace WPEFramework
{
    // RTPlayer *RTPlayer::inst_ = 0;

    // RTPlayer* RTPlayer::getInstance()
    // {
    //     if (!inst_) {
    //         inst_ = new RTPlayer();
    //     }
    //     return inst_;
    // }


    // RTPlayer::RTPlayer()
    // {
    //     Initialize();
    // }

    RTPlayer::~RTPlayer()
    {
        Deinitialize();
    }

    void stringToBool(rtString const& s, bool& b)
    {
        if(s.compare("true") == 0)
            b = true;
        else if(s.compare("false") == 0)
            b = false;
    }

    void boolToString(bool b, rtString& s)
    {
        s = b ? rtString("true") : rtString("false");
    }



    RTPlayer::RTPlayer(UnifiedPlayerNotify* parent):_parent(parent)
    {
        //TODO:
        /*
         * Initialize connection manager and start connection monitor thread.
         */
        connectionManager = std::make_shared<ConnectionManager>(this);
    }

    void RTPlayer::Deinitialize()
    {
        connectionManager.reset();
    }

    rtError RTPlayer::setUrl(const std::string& _url)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setUrl %s"), _url.c_str()));
        rtError rc = RT_ERROR;
        try
        {
            rc = connectionManager->getPlayerObject().set("url", rtString(_url.c_str()));
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer setUrl failed error: %s"), e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getUrl(std::string& _url)
    {
        rtString rUrl;
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().get("url", rUrl);
            _url = rUrl.cString();
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::open(const std::string& openParams, uint32_t* sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer open")));
        rtError rc = RT_ERROR;

        try
        {
            rc = connectionManager->getPlayerObject().sendReturns("open", rtString(openParams.c_str()), *sessionId);
            SYSLOG(Logging::Notification, (_T("RTPlayer open returning %d"), *sessionId));
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }

        return rc;
    }

    rtError RTPlayer::destroy(uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer destroy with sessionId: %u"), sessionId));
        rtError rc = RT_ERROR;

        try
        {
            rc = connectionManager->getPlayerObject().send("destroy", sessionId);
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }

        return rc;
    }

    rtError RTPlayer::sendData(const std::string& casData)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer sendData")));
        rtError rc = RT_ERROR;

        try
        {
            rc = connectionManager->getPlayerObject().send("sendCASData", rtString(casData.c_str()));
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }

        return rc;
    }

    rtError RTPlayer::pretune(const std::string& openParams, uint32_t* sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer pretune")));
        rtError rc = RT_ERROR;

        try
        {
            rc = connectionManager->getPlayerObject().sendReturns("pretune", rtString(openParams.c_str()), *sessionId);
            SYSLOG(Logging::Notification, (_T(" RTPlayer open returning %d"), *sessionId));
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }

        return rc;
    }

    rtError RTPlayer::park(uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer park with sessionId: %u"), sessionId));
        rtError rc = RT_ERROR;

        try
        {
            rc = connectionManager->getPlayerObject().send("park", sessionId);
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }

        return rc;
    }

    rtError RTPlayer::setSecondaryAudioLanguage(std::string _secondaryAudioLanguage, uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setSecondaryAudioLanguage %s"), _secondaryAudioLanguage.c_str()));
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("secondaryAudioLanguage", rtString(_secondaryAudioLanguage.c_str()), sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("secondaryAudioLanguage", rtString(_secondaryAudioLanguage.c_str()));
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getSecondaryAudioLanguage(std::string& secondaryAudioLanguage, uint32_t sessionId)
    {
        rtString rAudio;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("secondaryAudioLanguage", rAudio, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("secondaryAudioLanguage", rAudio);
#endif
            RT_LOG_IF_ERR(rc);
            secondaryAudioLanguage = rAudio.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
       return rc;
    }

    rtError RTPlayer::setDefaultAudioLanguage(std::string _defaultAudioLanguage, uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setdefaultAudioLanguage %s"), _defaultAudioLanguage.c_str()));
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("defaultAudioLanguage", rtString(_defaultAudioLanguage.c_str()), sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("defaultAudioLanguage", rtString(_defaultAudioLanguage.c_str()));
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getDefaultAudioLanguage(std::string& defaultAudioLanguage, uint32_t sessionId)
    {
        rtString rAudio;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("defaultAudioLanguage", rAudio, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("defaultAudioLanguage", rAudio);
#endif
            RT_LOG_IF_ERR(rc);
            defaultAudioLanguage = rAudio.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
       return rc;
    }

    rtError RTPlayer::getAvailableSubtitleLanguages(std::string& availableSubtitleLanguage, uint32_t sessionId)
    {
        rtString rSubtitleList;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("availableSubtitleLanguages", rSubtitleList, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("availableSubtitleLanguages", rSubtitleList);
#endif
            RT_LOG_IF_ERR(rc);
            availableSubtitleLanguage = rSubtitleList.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getAvailableSpeeds(std::string& availableSpeeds, uint32_t sessionId)
    {
        rtString rSpeedList;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("availableSpeeds", rSpeedList, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("availableSpeeds", rSpeedList);
#endif
            RT_LOG_IF_ERR(rc);
            availableSpeeds = rSpeedList.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setDefaultSubtitlesLanguage(std::string  subtitle, uint32_t sessionId)
    {
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("defaultSubtitlesLanguage", rtString( subtitle.c_str()), sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("defaultSubtitlesLanguage", rtString( subtitle.c_str()));
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getDefaultSubtitlesLanguage(std::string& subtLanguage, uint32_t sessionId)
    {
        rtError rc = RT_ERROR;
        try
        {
            rtString rAudio;
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("defaultSubtitlesLanguage", rAudio, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("defaultSubtitlesLanguage", rAudio);
#endif
            RT_LOG_IF_ERR(rc);
            subtLanguage = rAudio.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getCurrentAudioLanguage(std::string& currentAudioLanguage, uint32_t sessionId)
    {
        rtError rc = RT_ERROR;
        rtString rAudio;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("currentAudioLanguage", rAudio, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("currentAudioLanguage", rAudio);
#endif
            RT_LOG_IF_ERR(rc);
            currentAudioLanguage = rAudio.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getCurrentSubtitlesLanguage(std::string& currentSubtitlesLanguage, uint32_t sessionId)
    {
        rtError rc = RT_ERROR;
        rtString rSubt;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("currentSubtitlesLanguage", rSubt, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("currentSubtitlesLanguage", rSubt);
#endif
            RT_LOG_IF_ERR(rc);
            currentSubtitlesLanguage = rSubt.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::seekToLive(uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer seekToLive sessionId: %u"), sessionId));
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().send("seekToLive", sessionId);
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::requestStatus(uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer requestStatus sessionId: %u"), sessionId));
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().send("requestStatus", sessionId);
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setsubtitlesDisplayed(bool _subt, uint32_t sessionId)
    {
        rtError rc = RT_ERROR;
        rtString rSubt;
        boolToString(_subt,rSubt);

        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("subtitlesDisplayed", rSubt, sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("subtitlesDisplayed", rSubt);
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getsubtitlesDisplayed(bool* _subt, uint32_t sessionId)
    {
        rtString rSubt;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("subtitlesDisplayed", rSubt, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("subtitlesDisplayed", rSubt);
#endif
            RT_LOG_IF_ERR(rc);
            stringToBool(rSubt,*_subt);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
       return rc;
    }

    rtError RTPlayer::setVideoMute(bool _mute, uint32_t sessionId)
    {
        rtString rMute;
        boolToString(_mute, rMute);
        SYSLOG(Logging::Notification, (_T("RTPlayer setVideoMute %s"), rMute));
        rtError rc = RT_ERROR;

        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("videoMute", rtString(rMute), sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("videoMute", rtString(rMute));
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }

        return rc;
    }

    rtError RTPlayer::getVideoMute(bool* _mute, uint32_t sessionId)
    {
        rtString rVideoMute;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("videoMute", rVideoMute, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("videoMute", rVideoMute);
#endif
            RT_LOG_IF_ERR(rc);
            stringToBool(rVideoMute,*_mute);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setAudioMute(bool _mute, uint32_t sessionId)
    {
        rtString rMute;
        boolToString(_mute, rMute);
        SYSLOG(Logging::Notification, (_T("RTPlayer setAudioMute %s"), rMute));
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("audioMute", rtString(rMute), sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("audioMute", rtString(rMute));
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }

        return rc;
    }

    rtError RTPlayer::getAudioMute(bool* _mute, uint32_t sessionId)
    {
        rtString rAudioMute;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("audioMute", rAudioMute, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("audioMute", rAudioMute);
#endif
            RT_LOG_IF_ERR(rc);
            stringToBool(rAudioMute,*_mute);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setSpeed(float _speed, uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setSpeed %f"), _speed));
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("speed", _speed, sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("speed", _speed);
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setPrimaryAudioLanguage(std::string _audioLanguage, uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T(" RTPlayer setPrimaryAudioLanguage %s"), _audioLanguage.c_str()));
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("audioLanguage", rtString(_audioLanguage.c_str()), sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("audioLanguage", rtString(_audioLanguage.c_str()));
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getPrimaryAudioLanguage(std::string& audioLanguage, uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T(" RTPlayer getPrimaryAudioLanguage ")));
        rtString rAudio;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("audioLanguage", rAudio, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("audioLanguage", rAudio);
#endif
            RT_LOG_IF_ERR(rc);
            audioLanguage = rAudio.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getAvailableAudioLanguages(std::string& availableAudioLanguage, uint32_t sessionId)
    {
        rtString rAudioList;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("availableAudioLanguages", rAudioList, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("availableAudioLanguages", rAudioList);
#endif
            RT_LOG_IF_ERR(rc);
            availableAudioLanguage = rAudioList.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setPosition(float _pos, uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setPosition %f"), _pos));
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("position", _pos, sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("position", _pos);
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setPositionRelative(float _pos, uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setPosition %f"), _pos));
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("setPositionRelative", _pos, sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("setPositionRelative", _pos);
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getSpeed(float* _speed, uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setPosition %f"), _speed));
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("speed", *_speed, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("speed", *_speed);
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getPosition(float* pos, uint32_t sessionId)
    {
        *pos = -1;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("position", *pos, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("position", *pos);
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getDuration(float* duration, uint32_t sessionId)
    {
        *duration = -1;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("duration", *duration, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("duration", *duration);
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }

        return rc;
    }

    rtError RTPlayer::setPrimarySubtitlesLanguage(std::string  subtitle, uint32_t sessionId)
    {
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("subtitlesLanguage", rtString( subtitle.c_str()), sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("subtitlesLanguage", rtString( subtitle.c_str()));
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getPrimarySubtitlesLanguage(std::string& subtLanguage, uint32_t sessionId)
    {
        rtError rc = RT_ERROR;
        try
        {
            rtString rAudio;
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("subtitlesLanguage", rAudio, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("subtitlesLanguage", rAudio);
#endif
            RT_LOG_IF_ERR(rc);
            subtLanguage = rAudio.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    std::list<std::string> RTPlayer::getAvailableSubtitles()
    {
        std::list<std::string> _availableSubtitles;
        rtError rc = RT_ERROR_NOT_ALLOWED;
        RT_LOG_IF_ERR(rc);
        return _availableSubtitles;
    }

    rtError RTPlayer::play(uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer play with sessionId: %u"), sessionId));
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().send("play", sessionId);
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::pause(uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer pause sessionId: %u"), sessionId));
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().send("pause", sessionId);
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::stop(uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer session sessionId: %u"), sessionId));
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().send("stop", sessionId);
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setVideoRectangle(int _x, int _y, int _w, int _h, uint32_t sessionId)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setVideoRectangle with params: %d, %d, %d, %d, %u"), _x, _y, _w, _h, sessionId));
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().send("setVideoRectangle", _x, _y, _w, _h, sessionId);
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setTsbEnabled(std::string _enableTsb)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setTsbEnabled %s"), _enableTsb.c_str()));
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().set("tsbEnabled", rtString(_enableTsb.c_str()));
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getTsbEnabled(std::string& tsbEnabled)
    {
        rtString rTsbEnabled;
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().get("tsbEnabled", rTsbEnabled);
            RT_LOG_IF_ERR(rc);
            tsbEnabled = rTsbEnabled.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setAutoPlay(std::string _autoPlay)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setAutoPlay %s"), _autoPlay.c_str()));
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().set("autoPlay", rtString(_autoPlay.c_str()));
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getAutoPlay(std::string& autoPlay)
    {
        rtString rAutoPlay;
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().get("autoPlay", rAutoPlay);
            autoPlay = rAutoPlay.cString();
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setBlocked(std::string _blocked)
    {
        SYSLOG(Logging::Notification, (_T("RTPlayer setBlocked %s"), _blocked.c_str()));
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().set("blocked", rtString(_blocked.c_str()));
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }

        return rc;
    }

    rtError RTPlayer::getBlocked(std::string& blocked)
    {
        rtString rBlocked;
        rtError rc = RT_ERROR;
        try
        {
            rc  = connectionManager->getPlayerObject().get("blocked", rBlocked);
            RT_LOG_IF_ERR(rc);
            blocked = rBlocked.cString();
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::setsubtitlesEnabled(bool _subt, uint32_t sessionId)
    {
        rtError rc = RT_ERROR;
        rtString rSubt;
        boolToString(_subt,rSubt);

        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().set("subtitlesEnabled", rSubt, sessionId);
#else
            rc  = connectionManager->getPlayerObject().set("subtitlesEnabled", rSubt);
#endif
            RT_LOG_IF_ERR(rc);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::getsubtitlesEnabled(bool* _subt, uint32_t sessionId)
    {
        rtString rSubt;
        rtError rc = RT_ERROR;
        try
        {
#if ENABLE_SESSIONID
            rc  = connectionManager->getPlayerObject().get("subtitlesEnabled", rSubt, sessionId);
#else
            rc  = connectionManager->getPlayerObject().get("subtitlesEnabled", rSubt);
#endif
            RT_LOG_IF_ERR(rc);
            stringToBool(rSubt,*_subt);
        }
        catch(const std::exception& e)
        {
            SYSLOG(Logging::Notification, (_T("RTPlayer %s failed error: %s"), __PRETTY_FUNCTION__, e.what()));
        }
        return rc;
    }

    rtError RTPlayer::onEvent(int argc, rtValue const* argv, rtValue* result)
    {
        LOGDBG("RTPlayer Some event received with arg count:: %d", argc);
        WPEFramework::UnifiedPlayerNotify* parent = _parent;
        LOGDBG("Value of parent ptr: %p", parent);
        if (argc >= 1)
        {
            rtObjectRef event = argv[0].toObject();
            rtString eventName = event.get<rtString>("name");
            if(parent != nullptr)
            {

                LOGDBG("RTPlayer Received event: %s", eventName.cString());
                uint32_t sessionId = event.get<int>("session");
                LOGDBG("RTPlayer Received session: %d", sessionId);
                if(strcmp(eventName.cString(), "onMediaOpened") == 0)
                {
                    rtString mediaType = event.get<rtString>("mediaType");
                    rtString availSpeeds = event.get<rtString>("availableSpeeds");
                    int duration = event.get<int>("duration");
                    int width = event.get<int>("width");
                    int height = event.get<int>("height");
                    rtString availLang = event.get<rtString>("availableAudioLanguages");
                    rtString availCCLang = event.get<rtString>("availableClosedCaptionsLanguages");
                    rtString customProp = event.get<rtString>("customProperties");
                    rtString mediaSeg = event.get<rtString>("mediaSegments");
                    LOGDBG("RTPlayer Received MediaType: %s availableSpeeds: %s duration: %d width: %d height: %d availableAudioLanguages: %s availCCLang: %s customProp: %s  mediaSeg: %s",
                        mediaType.cString(), availSpeeds.cString(), duration, width, height, availLang.cString(), availCCLang.cString(),
                        customProp.cString(), mediaSeg.cString());
                    LOGDBG("Calling parent's event handler for event: %s", eventName.cString());
                    parent->event_onmediaopened(sessionId, mediaType.cString(), duration, width, height, availSpeeds.cString(), availLang.cString(), availCCLang.cString());
                }
                else if(strcmp(eventName.cString(), "onPlaying") == 0)
                {
                    LOGDBG("Calling parent's event handler for event: %s", eventName.cString());
                    parent->event_onplaying(sessionId);
                }
                else if(strcmp(eventName.cString(), "onPaused") == 0)
                {
                    LOGDBG("Calling parent's event handler for event: %s", eventName.cString());
                    parent->event_onpaused(sessionId);
                }
                else if(strcmp(eventName.cString(), "onClosed") == 0)
                {
                    LOGDBG("Calling parent's event handler for event: %s", eventName.cString());
                    parent->event_onclosed(sessionId);
                }
                else if(strcmp(eventName.cString(), "onComplete") == 0)
                {
                    LOGDBG("Calling parent's event handler for event: %s", eventName.cString());
                    parent->event_oncomplete(sessionId);
                }
                else if(strcmp(eventName.cString(), "onStatus") == 0)
                {
                    float position = event.get<float>("position");
                    float duration = event.get<float>("duration");
                    LOGDBG("Calling parent's event handler for event: %s", eventName.cString());
                    parent->event_onstatus(sessionId, position, duration);
                }
                else if(strcmp(eventName.cString(), "onProgress") == 0)
                {
                    LOGDBG("Calling parent's event handler for event: %s", eventName.cString());
                    float position = event.get<float>("position");
                    float duration = event.get<float>("duration");
                    float speed = event.get<float>("speed");
                    float start = event.get<float>("start");
                    float end = event.get<float>("end");
                    parent->event_onprogress(sessionId, position, duration, speed, start, end);
                }
                 else if(strcmp(eventName.cString(), "onBitrateChanged") == 0)
                {
                    int bitrate = event.get<int>("bitrate");
                    rtString reason = event.get<rtString>("reason");
                    LOGDBG("onBitrateChanged : Calling parent's event handler\n");
                    parent->event_onbitratechange(sessionId, bitrate, reason.cString());
                }
                else if(strcmp(eventName.cString(), "onWarning") == 0)
                {
                    unsigned int code = event.get<int>("code");
                    rtString description = event.get<rtString>("description");
                    LOGDBG("Calling parent's event handler for event: %s", eventName.cString());
                    parent->event_onwarning(sessionId, code, description.cString());
                }
                else if(strcmp(eventName.cString(), "onError") == 0)
                {
                    unsigned int code = event.get<int>("code");
                    rtString description = event.get<rtString>("description");
                    LOGDBG("Calling parent's event handler for event: %s", eventName.cString());
                    parent->event_onerror(sessionId, code, description.cString());
                }
                else if(strcmp(eventName.cString(), "onCASData") == 0)
                {
                    rtString data = event.get<rtString>("casData");
                    LOGDBG("OnCASData : Calling parent's event handler\n");
                    parent->event_oncasdata(sessionId, data.cString());
                }
            }
            else
            {
                LOGERR("parent is null so not calling corresponding event handler for event:%s", eventName.cString());

            }
        }
        return RT_OK;
    }
}
