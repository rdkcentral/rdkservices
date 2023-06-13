/* 
 * File:   RTPlayer.h
 * Author: gp
 *
 * Created on Feb 24, 2021
 */
#ifndef RTPLAYER_H
#define RTPLAYER_H

#include <string>
#include <list>
#include "Module.h"
#include "utils.h"
#include "ConnectionManager.h"
#include "UnifiedPlayerNotify.h"
#include "RTPlayerNotify.h"


/*
* Helpers
*/
#define RT_LOG_IF_ERR(rc)                                                                                   \
    do {                                                                                                    \
        if (rc != RT_OK) {                                                                                  \
            SYSLOG(WPEFramework::Logging::Notification,(_T("RTPlayer %s failed error %s"), __func__, rtStrError(rc)));    \
        }                                                                                                   \
    } while(0);

#define UNDEFINED_SESSION_ID 0uL
namespace WPEFramework
{
    class RTPlayer : public RTPlayerNotify {
    public:
        // static RTPlayer * getInstance();
        RTPlayer() = delete;
        RTPlayer(const RTPlayer&&) = delete;
        RTPlayer& operator=(const RTPlayer&&) = delete;
        RTPlayer(const RTPlayer&) = delete;
        RTPlayer& operator=(const RTPlayer&) = delete;
        explicit RTPlayer(UnifiedPlayerNotify* parent);
        ~RTPlayer();
        // rtError setParent(UnifiedPlayerNotify* parent);

        rtError setUrl(const std::string& _url);
        // std::string getUrl();
        rtError getUrl(std::string& _url);

        rtError open(const std::string& openParams, uint32_t* sessionId);
        rtError destroy(uint32_t sessionId);
        rtError sendData(const std::string& casData);
        rtError pretune(const std::string& openParams, uint32_t* sessionId);
        rtError park(uint32_t sessionId);

        rtError setPrimaryAudioLanguage(std::string _audioLanguage, uint32_t sessionId);
        rtError getPrimaryAudioLanguage(std::string& audioLanguage, uint32_t sessionId);
        rtError setSecondaryAudioLanguage(std::string _secondaryAudioLanguage, uint32_t sessionId);
        rtError getSecondaryAudioLanguage(std::string& secondaryAudioLanguage, uint32_t sessionId);
        rtError setDefaultAudioLanguage(std::string _defaultAudioLanguage, uint32_t sessionId);
        rtError getDefaultAudioLanguage(std::string& defaultAudioLanguage, uint32_t sessionId);
        rtError getAvailableAudioLanguages(std::string& availableAudioLanguage, uint32_t sessionId);
        rtError getCurrentAudioLanguage(std::string& currentAudioLanguage, uint32_t sessionId);

        rtError getPrimarySubtitlesLanguage(std::string& subtLanguage, uint32_t sessionId);
        rtError setPrimarySubtitlesLanguage(std::string  subtitle, uint32_t sessionId);
        rtError getDefaultSubtitlesLanguage(std::string& subtLanguage, uint32_t sessionId);
        rtError setDefaultSubtitlesLanguage(std::string  subtitle, uint32_t sessionId);
        rtError getAvailableSubtitleLanguages(std::string& availableSubtitleLanguage, uint32_t sessionId);
        rtError getCurrentSubtitlesLanguage(std::string& currentSubtitlesLanguage, uint32_t sessionId);

        rtError getAvailableSpeeds(std::string& availableSpeeds, uint32_t sessionId);
        rtError seekToLive(uint32_t sessionId);
        rtError requestStatus(uint32_t sessionId);
        rtError setSpeed(float _speed, uint32_t sessionId);
        rtError setsubtitlesDisplayed(bool _subt, uint32_t sessionId);
        rtError getsubtitlesDisplayed(bool* _subt, uint32_t sessionId);
        rtError setVideoMute(bool _mute, uint32_t sessionId);
        rtError getVideoMute(bool* _mute, uint32_t sessionId);
        rtError setAudioMute(bool _mute, uint32_t sessionId);
        rtError getAudioMute(bool* _mute, uint32_t sessionId);
        rtError setPosition(float _pos, uint32_t sessionId);
        rtError setPositionRelative(float _pos, uint32_t sessionId);
        rtError getSpeed(float* speed, uint32_t sessionId);
        rtError getPosition(float* pos, uint32_t sessionId);
        rtError getDuration(float* duration, uint32_t sessionId);

        std::list<std::string> getAvailableSubtitles();

        rtError play(uint32_t sessionId);
        rtError pause(uint32_t sessionId);
        rtError stop(uint32_t sessionId);

        rtError setVideoRectangle(int _x, int _y, int _w, int _h, uint32_t sessionId);

        rtError setTsbEnabled(std::string _enableTsb);
        std::string getTsbEnabled();
        rtError getTsbEnabled(std::string& tsbEnabled);

        rtError setAutoPlay(std::string _autoPlay);
        // std::string getAutoPlay();
        rtError getAutoPlay(std::string& autoPlay);

        rtError setBlocked(std::string _blocked);
        // std::string getBlocked();
        rtError getBlocked(std::string& _blocked);

        rtError setsubtitlesEnabled(bool _subt, uint32_t sessionId);
        rtError getsubtitlesEnabled(bool* _subt, uint32_t sessionId);

        // RTPlayerNotify event
        rtError onEvent(int argc, rtValue const* argv, rtValue* result) override;

    private:

        void Initialize();
        void Deinitialize();
        UnifiedPlayerNotify* _parent;
        std::shared_ptr<ConnectionManager> connectionManager;
    };

}
#endif /* RTPLAYER_H */
