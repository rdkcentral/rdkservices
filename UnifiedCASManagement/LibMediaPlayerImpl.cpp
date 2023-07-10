/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 RDK Management
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

#include "UtilsJsonRpc.h"

#include "LibMediaPlayerImpl.h"
#include "UnifiedCASManagement.h"

struct kv_pair
{
    const char * key;
    const char * value;
};

// TODO: Need to update/remove the following SOC specific code as and when change is made in MediEngineRMF.cpp
static kv_pair environment_variables[] = { //Environment variables that need to be set for successful platform initialization.
    {"brcm_directfb_mode", "n"},
    {"brcm_multiprocess_server", "refsw_server"},
    {"brcm_multiprocess_mode", "y"},
    {"GST_ENABLE_SVP", "1"}
};

static void setEnvVariables()
{
    int listSize = sizeof(environment_variables) / sizeof(kv_pair);
    for (int i = 0; i < listSize; i++)
    {
        setenv(environment_variables[i].key, environment_variables[i].value, 1);
    }
}

namespace WPEFramework
{

namespace Plugin
{

LibMediaPlayerImpl::LibMediaPlayerImpl(void* t_unifiedCasMgmt) : MediaPlayer(t_unifiedCasMgmt)
{
    LOGINFO(" LibMediaPlayerImpl Constructor");
}

LibMediaPlayerImpl::~LibMediaPlayerImpl()
{
    LOGINFO(" LibMediaPlayerImpl Destructor");
}

bool LibMediaPlayerImpl::openMediaPlayer(
     std::string&       t_openParams, 
     const std::string& t_sessionType)
{
    bool retValue = false;
    
    setEnvVariables();

    m_sessionType = t_sessionType;

    if(m_sessionType != "MANAGE_NO_TUNER")
    {
        if(nullptr != m_libMediaPlayer)
        {
            LOGERR("LibMediaplayer instance is already avalailable");
            return retValue;
        }

        if(0 != mediaplayer::initialize(QAM, true, true))
        {
            LOGERR("Could not initialize QAM support");
        }
        else
        {
            m_libMediaPlayer = std::unique_ptr <mediaplayer>(mediaplayer::createMediaPlayer(QAM, t_openParams, CAS_TYPE_ANYCAS));
            if(nullptr == m_libMediaPlayer)
            {
                LOGERR("LibMediaPlayer creation failed.");
            }
            else
            {
                m_libMediaPlayer->registerEventCallbacks(LibMediaPlayerImpl::eventCallBack, LibMediaPlayerImpl::errorCallBack, this);
                LOGINFO(" Successfully initialized and registered for callbacks with LibMediaPlayer");
                retValue = true;
            }
        }
    }
    else
    {
        /* NO_TUNE Management session does not require a tuner or a media pipeline so, creating AnyCasCASService instance directly*/
        m_anyCasCASServiceInst = std::make_shared <AnyCasCASServiceImpl>(t_openParams);
        if(nullptr != m_anyCasCASServiceInst)
        {
            if(true != m_anyCasCASServiceInst->initializeCasService(nullptr, nullptr))
            {
                LOGERR("Failed to initialize AnyCasCASServiceImpl.");
            }
            else
            {
                m_anyCasCASServiceInst->registerCallbacks(LibMediaPlayerImpl::eventCallBack, LibMediaPlayerImpl::errorCallBack, this);
                LOGINFO(" Successfully initialized and registered for callbacks with AnyCasCASServiceImpl");
                retValue = true;
            }
        }
        else
        {
            LOGERR("Failed to create instance of AnyCasCASServiceImpl.");
        }
    }
    return retValue;
}

bool LibMediaPlayerImpl::closeMediaPlayer(void)
{
    bool retValue = false;

    if(m_sessionType != "MANAGE_NO_TUNER")
    {
        if(nullptr == m_libMediaPlayer)
        {
            LOGERR("LibMediaPlayer instance not found.");
        }
        else
        {
            if(0 != m_libMediaPlayer->stop())
            {
                LOGERR("Failed to stop libmediaplayer.");
            }
            else
            {
                m_libMediaPlayer.reset();
                retValue = true;
                LOGERR("libmediaplayer stopped.");
            }
        }
    }
    else
    {
        if(nullptr != m_anyCasCASServiceInst)
        {
            if(false == m_anyCasCASServiceInst->stopCasService())
            {
                LOGERR("stopCasService failed");
            }
            else
            {
                m_anyCasCASServiceInst.reset();
                m_sessionType.clear();
                retValue = true;
                LOGERR("stopCasService success.");
            }
        }
        else
        {
            LOGERR("AnyCasCASServiceImpl instance not found");
        }
    }
    return retValue;
}

bool LibMediaPlayerImpl::requestCASData(std::string& t_data)
{
    bool retValue = false;

    if(m_sessionType != "MANAGE_NO_TUNER")
    {
        if(nullptr == m_libMediaPlayer)
        {
            LOGERR("LibMediaPlayer instance not found.");
        }
        else
        {
            std::weak_ptr<CASService>   tmpPtr = m_libMediaPlayer->getCasServiceInstance();
            std::shared_ptr<CASService> casService = tmpPtr.lock();
            AnyCasCASServiceImpl*       anyCasService = nullptr;

            if (nullptr != casService)
            {
                anyCasService = dynamic_cast<AnyCasCASServiceImpl *>(casService.get());
                if (nullptr != anyCasService)
                {
                    anyCasService->sendCASData(t_data);
                    LOGINFO(" Successfully sent CASData using sendCASData method");
                    retValue = true;
                }
                else
                {
                    LOGERR("Could not get AnyCasCASServiceImpl instance");
                }
            }
            else
            {
                LOGERR("Could not get CASService instance");
            }
        }
    }
    else
    {
        if(nullptr != m_anyCasCASServiceInst)
        {
            m_anyCasCASServiceInst->sendCASData(t_data);
            LOGINFO(" Successfully sent CASData using sendCASData method");
            retValue = true;
        }
        else
        {
            LOGERR("AnyCasCASServiceImpl instance not found");
        }
    }
    return retValue;
}

void LibMediaPlayerImpl::eventCallBack(
     notification_payload * t_payload,
     void*                  t_data)
{
    LibMediaPlayerImpl * instance = reinterpret_cast<LibMediaPlayerImpl *>(t_data);
    if(nullptr != instance)
    {
        UnifiedCASManagement * session = reinterpret_cast<UnifiedCASManagement *>(instance->m_unifiedCasMgmt);
        LOGINFO("Received mediaPlayerEvent. casData is %s", t_payload->m_message.c_str());
        session->event_data(t_payload->m_message, "PUBLIC");
    }
    else
    {
        LOGERR("Received invalid LibMediaPlayer instance");
    }
}

void LibMediaPlayerImpl::errorCallBack(
     notification_payload * t_payload,
     void*                  t_data)
{
    LibMediaPlayerImpl * instance = reinterpret_cast<LibMediaPlayerImpl *>(t_data);
    if(nullptr != instance)
    {
        LOGINFO("Received mediaPlayerError. status is %lld", t_payload->m_code);
    }
    else
    {
        LOGERR("Received invalid LibMediaPlayer instance");
    }
}

} // namespace Plugin

} // namespace WPEFramework
