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

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <iostream>
#include <string>

using namespace std;

namespace WPEFramework
{

namespace Plugin
{

class MediaPlayer
{

public:
    MediaPlayer() = delete;

    MediaPlayer(void* t_unifiedCasMgmt)
    {
        m_unifiedCasMgmt = t_unifiedCasMgmt;
    }

    virtual ~MediaPlayer()
    {
    }

    /**
     * @brief     This method initializes mediaplayer.
     * @details   This method sets environment varible and also registers callback methods.
     *
     * @parm[in]  t_openParams  Parameter required to create a CMI session.
     * @parm[in]  t_sessionType Parameter denoting the type of management session.
     * @parm[out] None
     *
     * @return    None
     */
    virtual bool openMediaPlayer(
                 std::string&       t_openParams, 
                 const std::string& t_sessionType)
    {
        return true;
    }

    /**
     * @brief     This method destroys mediaplayer.
     *
     * @parm[in]  None
     *
     * @return    None
     */
    virtual bool closeMediaPlayer(void)
    {
        return true;
    }

    /**
     * @brief     This method is used to request CAS specific information from the OCDM CAS plugin through the CASHelperImpl.
     *
     * @parm[in]  t_data Data which consists the command.
     *
     * @return    None
     */
    virtual bool requestCASData(std::string& t_data)
    {
        return true;
    }

protected:
    void* m_unifiedCasMgmt; //Instance of UnifiedCASManagement service
};

} // namespace Plugin

} // namespace WPEFramework
#endif /* MEDIAPLAYER_H */
