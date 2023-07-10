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

#ifndef LIBMEDIAPLAYERIMPL_H
#define LIBMEDIAPLAYERIMPL_H

#include "MediaPlayer.h"
#include "libmediaplayer.h"
#include "AnyCasCASServiceImpl.h"

using namespace libmediaplayer;

namespace WPEFramework
{

namespace Plugin
{

/**
 * @brief   This class will implement MediaPlayer APIs to support/enable
 *          its functionalities to work with libmediaplayer.
 * @details Any future additions to MediaPlayer class will be included in this class.
 */
class LibMediaPlayerImpl: public MediaPlayer
{

public:
    LibMediaPlayerImpl() = delete;

    LibMediaPlayerImpl(void* t_unifiedCasMgmt);

    virtual ~LibMediaPlayerImpl();

    /**
     * @brief     This method initializes libmediaplayer.
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
                 const std::string& t_sessionType) override;

    /**
     * @brief     This method destroys libmediaplayer.
     *
     * @parm[in]  None

     * @return    None
     */
    virtual bool closeMediaPlayer(void) override;

    /**
     * @brief     This method is used to request CAS specific information from the OCDM CAS plugin through the CASHelperImpl.
     *
     * @parm[in]  t_data Data which consists the command.
     *
     * @return    None
     */
    virtual bool requestCASData(std::string& t_data) override;

private:
    /**
     * @brief     This method is used to register with mediaplayer(libmediaplayer) to get 
     *            the status notifications.
     *
     * @parm[out]  t_paload data which consists the status information.
     * @parm[out]  t_data   user data.
     *
     * @return    None
     */
    static void eventCallBack(
                notification_payload * t_payload,
                void *                 t_data);

    /**
     * @brief     This method is used to register with mediaplayer(libmediaplayer) to get 
     *            the error notifications.
     *
     * @parm[out]  t_payload data which consists the error information.
     * @parm[out]  t_data    user data.
     *
     * @return    None
     */
    static void errorCallBack(
                notification_payload * t_payload,
                void *                 t_data);

    std::unique_ptr <libmediaplayer::mediaplayer> m_libMediaPlayer = nullptr; //To store the libmediaplayer instance
    std::shared_ptr <AnyCasCASServiceImpl>        m_anyCasCASServiceInst = nullptr; //To store AnyCasCASServiceImpl instance
    std::string                                   m_sessionType = "";//To store the type of management session
};

} // namespace Plugin

} // namespace WPEFramework
#endif /* LIBMEDIAPLAYERIMPL_H */
