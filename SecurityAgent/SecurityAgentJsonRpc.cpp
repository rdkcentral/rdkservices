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
 
#include "Module.h"
#include "SecurityAgent.h"
#include <interfaces/json/JsonData_SecurityAgent.h>

#include "TokenFactory.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::SecurityAgent;

    // Registration
    //

    void SecurityAgent::RegisterAll()
    {
        #ifdef SECURITY_TESTING_MODE
        Register<CreatetokenParamsData,CreatetokenResultInfo>(_T("createtoken"), &SecurityAgent::endpoint_createtoken, this);
        #endif  

        Register<CreatetokenResultInfo,ValidateResultData>(_T("validate"), &SecurityAgent::endpoint_validate, this);
    }

    void SecurityAgent::UnregisterAll()
    {
        Unregister(_T("validate"));
        #ifdef SECURITY_TESTING_MODE
        Unregister(_T("createtoken"));
        #endif
    }

    // API implementation
    //

    #ifdef SECURITY_TESTING_MODE
    // Method: createtoken - Creates Token
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: Token creation failed
    uint32_t SecurityAgent::endpoint_createtoken(const CreatetokenParamsData& params, CreatetokenResultInfo& response)
    {
        uint32_t result = Core::ERROR_NONE;

        string token, payload;
        params.ToString(payload);

        if (CreateToken(static_cast<uint16_t>(payload.length()), reinterpret_cast<const uint8_t*>(payload.c_str()), token) == Core::ERROR_NONE) {
            response.Token = token;
        } else {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }
    #endif

    // Method: validate - Creates Token
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t SecurityAgent::endpoint_validate(const CreatetokenResultInfo& params, ValidateResultData& response)
    {
        uint32_t result = Core::ERROR_NONE;
        const string& token = params.Token.Value();
        response.Valid = false;

        auto webToken = JWTFactory::Instance().Element();
        ASSERT(webToken != nullptr);
        uint16_t load = webToken->PayloadLength(token);

        // Validate the token
        if (load != static_cast<uint16_t>(~0)) {
            // It is potentially a valid token, extract the payload.
            uint8_t* payload = reinterpret_cast<uint8_t*>(ALLOCA(load));

            load = webToken->Decode(token, load, payload);

             if (load != static_cast<uint16_t>(~0)) {
                response.Valid = true;
            }
        }

        return result;
    }

} // namespace Plugin

}

