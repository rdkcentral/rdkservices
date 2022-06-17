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

#include "Module.h"
#include "UnifiedCASManagement.h"
#include "UnifiedError.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::UnifiedCASManagement;
    SERVICE_REGISTRATION(UnifiedCASManagement, 1, 0);
    // Registration
    //

    void UnifiedCASManagement::RegisterAll()
    {
        Register<ManagerequestData,ResultInfo>(_T("manage"), &UnifiedCASManagement::endpoint_manage, this);
        Register<Core::JSON::String,ResultInfo>(_T("unmanage"), &UnifiedCASManagement::endpoint_unmanage, this);
        Register<XferinfoInfo,ResultInfo>(_T("send"), &UnifiedCASManagement::endpoint_send, this);
    }

    void UnifiedCASManagement::UnregisterAll()
    {
        Unregister(_T("send"));
        Unregister(_T("unmanage"));
        Unregister(_T("manage"));
    }

    // API implementation
    //

    // Method: manage - Manage a well-known CAS
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t UnifiedCASManagement::endpoint_manage(const ManagerequestData& params, ResultInfo& response)
    {
        uint32_t result = Core::ERROR_NONE;
        rtError _error = RT_OK;
        const string& mediaurl = params.Mediaurl.Value();
        const ManagerequestData::ModeType& mode = params.Mode.Value();
        const ManagerequestData::ManageType& manage = params.Manage.Value();
        const string& casinitdata = params.Casinitdata.Value();
        const string& casocdmid = params.Casocdmid.Value();

        LOGDBG("media URL:%s, ocdmid = %s", mediaurl.c_str(), casocdmid.c_str());
	//sessionId exists - management session already available
	if(m_sessionId != UNDEFINED_SESSION_ID)
	{
	    LOGINFO("Management session already exists - returning fail");
	    _error = RT_ERROR;
	} 
        if(mode != ManagerequestData::ModeType::MODE_NONE) {
            LOGINFO("mode must be MODE_NONE for CAS Management");
            _error = RT_ERROR;
        }
        else if (manage != ManagerequestData::ManageType::MANAGE_FULL && manage != ManagerequestData::ManageType::MANAGE_NO_PSI && manage != ManagerequestData::ManageType::MANAGE_NO_TUNER) {
            LOGINFO("manage must be MANAGE_ ... FULL, NO_PSI or NO_TUNER for CAS MAnagement");
            _error = RT_ERROR;
        }
        else if(casocdmid.empty()) {
            LOGINFO("ocdmcasid is mandatory for CAS management session");
            _error = RT_ERROR;
        }

        if(_error != RT_OK)
        {
            LOGDBG("UnifiedPlayer Open Session Failed");
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);
            return result;
        }

        JsonObject jsonParams;
        jsonParams["mediaurl"] = mediaurl;
        jsonParams["mode"] = "MODE_NONE";
        switch(manage){
            case ManagerequestData::ManageType::MANAGE_FULL:
                jsonParams["manage"] = "MANAGE_FULL";
                break;
            case ManagerequestData::ManageType::MANAGE_NO_PSI:
                jsonParams["manage"] = "MANAGE_NO_PSI";
                break;
            case ManagerequestData::ManageType::MANAGE_NO_TUNER:
                jsonParams["manage"] = "MANAGE_NO_TUNER";
                break;
            default:
                LOGWARN("Unexpected manage type: %d", manage);
                break;
        }
        jsonParams["casinitdata"] = casinitdata;
        jsonParams["casocdmid"] = casocdmid;

        std::string openParams;
        uint32_t sessionID;
        jsonParams.ToString(openParams);
        LOGDBG("OpenData = %s\n", openParams.c_str());
        _error = m_RTPlayer->open(openParams, &sessionID);

        if (_error == RT_OK ) {
	    m_sessionId = sessionID;
            LOGWARN("Opening media session succeeded... sessionId - %d", m_sessionId);
        }
	else
	{
            LOGWARN("Error in Opening media session ..");
	}
        result = RTERROR_TO_WPEERROR(_error);
        RTERROR_TO_RESPONSE(_error);
        return result;
    }

    // Method: unmanage - Destroy a management session
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t UnifiedCASManagement::endpoint_unmanage(const Core::JSON::String& params, ResultInfo& response)
    {
            rtError _error = RT_OK;
            uint32_t result;

            _error = m_RTPlayer->destroy(m_sessionId);
            if (_error == RT_OK) {
                LOGDBG("UnifiedPlayer destroy CAS Management Session succeeded..\n");
		m_sessionId = UNDEFINED_SESSION_ID;
            }
            else
            {
                LOGWARN("Error in destroying CAS Management Session...\n");
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
    }

    // Method: send - Sends data to the remote CAS
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t UnifiedCASManagement::endpoint_send(const XferinfoInfo& params, ResultInfo& response)
    {
        uint32_t result = Core::ERROR_NONE;
	rtError _error = RT_OK;
        const string& payload = params.Payload.Value();
        const SourceType& source = params.Source.Value();
	JsonObject jsonParams;
            jsonParams["payload"] = payload;
            jsonParams["source"] = (source == SourceType::PUBLIC)?"PUBLIC":"PRIVATE";
	    jsonParams["sessionId"] = m_sessionId;
            std:string data;
            jsonParams.ToString(data);
            LOGWARN("Send Data = %s\n", data);
            LOGWARN("Send Data m_sessionId = %d\n", m_sessionId);

            _error = m_RTPlayer->sendData(data);
            if (_error == RT_OK) {
                LOGDBG("UnifiedPlayer send Data succeeded.. Calling Play\n");
            }
            else
            {
                LOGWARN("Error in setting send Data...\n");
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
    }

    void UnifiedCASManagement::event_oncasdata(const uint32_t& sessionid, const string& casData) {
        LOGWARN("event_oncasdata sessionId = %d, m_sessionId = %d\n", sessionid, m_sessionId);
	if(sessionid == m_sessionId)
            event_data(casData, SourceType::PUBLIC);
    }

    // Event: data - Sent when the CAS needs to send data to the caller
    void UnifiedCASManagement::event_data(const string& payload, const SourceType& source)
    {
        XferinfoInfo params;
        params.Payload = payload;
        params.Source = static_cast<SourceType>(source);

        Notify(_T("data"), params);
    }

} // namespace Plugin

}

