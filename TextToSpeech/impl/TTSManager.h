/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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
#ifndef _TTS_ENGINE_H_
#define _TTS_ENGINE_H_

#include "TTSCommon.h"
#include <iostream>
#include "TTSSession.h" 

#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>

namespace TTS {

struct Configuration {
    Configuration() : volume(0), rate(0) {};
    ~Configuration() {}

    std::string ttsEndPoint;
    std::string ttsEndPointSecured;
    std::string language;
    std::string voice;
    double volume;
    uint8_t rate;
};

class TTSConnectionCallback {
public:
    TTSConnectionCallback() {}
    virtual ~TTSConnectionCallback() {}

    virtual void onTTSServerConnected() = 0;
    virtual void onTTSServerClosed() = 0;
    virtual void onTTSStateChanged(bool enabled) = 0;
    virtual void onVoiceChanged(std::string voice) { (void)voice; }
};

class TTSManager {
public:
    static TTSManager *create(TTSConnectionCallback *connCallback);
    TTSManager(TTSConnectionCallback *connCallback);
    virtual ~TTSManager();

    // TTS Global APIs
    TTS_Error enableTTS(bool enable);
    bool isTTSEnabled();
    TTS_Error listVoices(std::string language, std::vector<std::string> &voices);
    TTS_Error setConfiguration(Configuration &configuration);
    TTS_Error getConfiguration(Configuration &configuration);
    bool isSessionActiveForApp(uint32_t appid);

    // Resource management APIs
    TTS_Error getResourceAllocationPolicy(ResourceAllocationPolicy &policy);
    TTS_Error reservePlayerResource(uint32_t appId, bool internalReq=false);
    TTS_Error releasePlayerResource(uint32_t appId, bool internalReq=false);

    /***************** For Override Control *****************/
    TTS_Error claimPlayerResource(uint32_t appId);
    /***************** For Override Control *****************/

    // Session control functions
    TTSSession* createSession(uint32_t appId, std::string appName, uint32_t &sessionID, bool &ttsEnabled, TTS_Error &status, TTSSessionCallback *eventCallbacks);
    TTS_Error destroySession(uint32_t sessionId);

private:
    using ID_Session_Map=std::map<uint32_t, TTSSession*>;
    ID_Session_Map m_appMap;
    ID_Session_Map m_sessionMap;

    TTSConfiguration m_defaultConfiguration;
    ResourceAllocationPolicy m_policy;
    uint32_t m_reservationForApp;
    uint32_t m_reservedApp;
    uint32_t m_claimedApp;
    TTSSession *m_activeSession;
    TTSSpeaker *m_speaker;
    std::thread *m_thread;
    bool m_monitorClients;
    bool m_claimedSession;
    bool m_ttsEnabled;
    std::mutex m_mutex;
    TTSConnectionCallback *m_callback;

    void loadConfigurationsFromFile(std::string configFile);
    void setResourceAllocationPolicy(ResourceAllocationPolicy policy);
    void makeSessionActive(TTSSession *session);
    void makeSessionInActive(TTSSession *session);
    void makeReservedOrClaimedSessionActive();

    static void MonitorClients(void *ctx);
    static void MonitorClientsSourceIOCB(void *source, void *ctx);
    static void MonitorClientsSourceDestroyedCB(void *source, void *ctx);
};

} // namespace TTS

#endif
