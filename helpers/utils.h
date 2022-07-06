/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include <fstream>
#include <algorithm>
#include "tracing/Logging.h"
#include <syscall.h>
#include <plugins/plugins.h>
#include <tracing/tracing.h>
#include "rfcapi.h"
#include <math.h>
#include "rdk_logger_milestone.h"

// telemetry
#include "UtilsTelemetry.h"

#define UNUSED(expr)(void)(expr)
#define C_STR(x) (x).c_str()

#define LOGINFO(fmt, ...) do { fprintf(stderr, "[%d] INFO [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define LOGDBG(fmt, ...) do { fprintf(stderr, "[%d] DEBUG [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define LOGWARN(fmt, ...) do { fprintf(stderr, "[%d] WARN [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define LOGERR(fmt, ...) do { fprintf(stderr, "[%d] ERROR [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); Utils::Telemetry::sendError(fmt, ##__VA_ARGS__); } while (0)

#define LOGINFOMETHOD() { std::string json; parameters.ToString(json); LOGINFO( "params=%s", json.c_str() );  }
#define LOGTRACEMETHODFIN() do { std::string json; response.ToString(json); LOGINFO( "response=%s", json.c_str() );  } while (0)

#define LOG_DEVICE_EXCEPTION0() LOGWARN("Exception caught: code=%d message=%s", err.getCode(), err.what());
#define LOG_DEVICE_EXCEPTION1(param1) LOGWARN("Exception caught" #param1 "=%s code=%d message=%s", param1.c_str(), err.getCode(), err.what());
#define LOG_DEVICE_EXCEPTION2(param1, param2) LOGWARN("Exception caught " #param1 "=%s " #param2 "=%s code=%d message=%s", param1.c_str(), param2.c_str(), err.getCode(), err.what());

#define LOG_MILESTONE(milestone) logMilestone(milestone);

/* a=target variable, b=bit number to act upon 0-n */
#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

#define BITMASK_SET(x, mask) ((x) |= (mask))
#define BITMASK_CLEAR(x, mask) ((x) &= (~(mask)))
#define BITMASK_FLIP(x, mask) ((x) ^= (mask))
#define BITMASK_CHECK_ALL(x, mask) (!(~(x) & (mask)))
#define BITMASK_CHECK_ANY(x, mask) ((x) & (mask))

#define GET_BITMASK(a) (((short)pow(2,a))&0xFFFF)

//this set of macros are used in the method handlers to make the code more consistent and easier to read
#define vectorSet(v,s) \
    if (find(begin(v), end(v), s) == end(v)) \
        v.emplace_back(s);

#define returnResponse(success) \
    { \
        response["success"] = success; \
        LOGTRACEMETHODFIN(); \
        return (WPEFramework::Core::ERROR_NONE); \
    }

#define returnIfWrongApiVersion(version)\
    if(m_apiVersionNumber < version) \
    { \
        LOGWARN("method %s not supported. version required=%u actual=%u", __FUNCTION__, version, m_apiVersionNumber); \
        returnResponse(false); \
    }

#define returnIfParamNotFound(param, name) \
    if (!param.HasLabel(name)) \
    { \
        LOGERR("No argument '%s'", name); \
        returnResponse(false); \
    }

#define returnIfStringParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != WPEFramework::Core::JSON::Variant::type::STRING) \
    {\
        LOGERR("No argument '%s' or it has incorrect type", name); \
        returnResponse(false); \
    }

#define returnIfBooleanParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != WPEFramework::Core::JSON::Variant::type::BOOLEAN) \
    { \
        LOGERR("No argument '%s' or it has incorrect type", name); \
        returnResponse(false); \
    }

#define returnIfArrayParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != WPEFramework::Core::JSON::Variant::type::ARRAY) \
    { \
        LOGERR("No argument '%s' or it has incorrect type", name); \
        returnResponse(false); \
    }

#define returnIfNumberParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != WPEFramework::Core::JSON::Variant::type::NUMBER) \
    { \
        LOGERR("No argument '%s' or it has incorrect type", name); \
        returnResponse(false); \
    }

#define returnIfObjectParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != WPEFramework::Core::JSON::Variant::type::OBJECT) \
    { \
        LOGERR("No argument '%s' or it has incorrect type", name); \
        returnResponse(false); \
    }

#define sendNotify(event,params) { \
    std::string json; \
    params.ToString(json); \
    LOGINFO("Notify %s %s", event, json.c_str()); \
    Notify(event,params); \
}

#define getNumberParameter(paramName, param) { \
    if (WPEFramework::Core::JSON::Variant::type::NUMBER == parameters[paramName].Content()) \
        param = parameters[paramName].Number(); \
    else \
        try { param = std::stoi( parameters[paramName].String()); } \
        catch (...) { param = 0; } \
}

#define getDefaultNumberParameter(paramName, param, default) { \
    if (parameters.HasLabel(paramName)) { \
        if (WPEFramework::Core::JSON::Variant::type::NUMBER == parameters[paramName].Content()) \
            param = parameters[paramName].Number(); \
        else \
            try { param = std::stoi( parameters[paramName].String()); } \
            catch (...) { param = default; } \
    } else param = default; \
}

#define getDefaultStringParameter(paramName, param, default) { \
    if (parameters.HasLabel(paramName)) { \
        if (WPEFramework::Core::JSON::Variant::type::STRING == parameters[paramName].Content()) \
            param = parameters[paramName].String(); \
        else \
            param = default; \
    } else param = default; \
}

#define getNumberParameterObject(parameters, paramName, param) { \
    if (WPEFramework::Core::JSON::Variant::type::NUMBER == parameters[paramName].Content()) \
        param = parameters[paramName].Number(); \
    else \
        try {param = std::stoi( parameters[paramName].String());} \
        catch (...) { param = 0; } \
}

#define getFloatParameter(paramName, param) { \
    if (Core::JSON::Variant::type::FLOAT == parameters[paramName].Content()) \
        param = parameters[paramName].Float(); \
    else \
        try { param = std::stof( parameters[paramName].String()); } \
        catch (...) { param = 0; } \
}

#define getBoolParameter(paramName, param) { \
    if (WPEFramework::Core::JSON::Variant::type::BOOLEAN == parameters[paramName].Content()) \
        param = parameters[paramName].Boolean(); \
    else \
        param = parameters[paramName].String() == "true" || parameters[paramName].String() == "1"; \
}

#define getDefaultBoolParameter(paramName, param, default) { \
    if (parameters.HasLabel(paramName)) { \
        if (WPEFramework::Core::JSON::Variant::type::BOOLEAN == parameters[paramName].Content()) \
            param = parameters[paramName].Boolean(); \
        else \
            param = parameters[paramName].String() == "true" || parameters[paramName].String() == "1"; \
     } else param = default; \
}

#define getStringParameter(paramName, param) { \
    if (WPEFramework::Core::JSON::Variant::type::STRING == parameters[paramName].Content()) \
        param = parameters[paramName].String(); \
}

namespace Utils
{
    /***
     * @brief	: Execute shell script and get response
     * @param1[in]	: script to be executed with args
     * @return		: string; response.
     */
    std::string cRunScript(const char *cmd);

    /***
     * @brief	: Checks that file exists
     * @param1[in]	: pFileName name of file
     * @return		: true if file exists.
     */
    bool fileExists(const char *pFileName);

    /***
     * @brief	: Checks that file exists and modified at least pointed seconds ago
     * @param1[in]	: pFileName name of file
     * @param1[in]	: age modification age in seconds
     * @return		: true if file exists and modifies 'age' seconds ago.
     */
    bool isFileExistsAndOlderThen(const char *pFileName, long age = -1);

    struct SecurityToken
    {
        static void getSecurityToken(std::string& token);
        static bool isThunderSecurityConfigured();

    private:
        static std::string m_sToken;
        static bool m_sThunderSecurityChecked;
    };

    // Thunder Plugin Communication
    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> getThunderControllerClient(std::string callsign="");

    void activatePlugin(const char* callSign);

    bool isPluginActivated(const char* callSign);

    bool getRFCConfig(char* paramName, RFC_ParamData_t& paramOutput);
    bool isValidInt(char* x);
    bool isValidUnsignedInt(char* x);
    void syncPersistFile (const string file);
    void persistJsonSettings(const string file, const string strKey, const JsonValue& jsValue);

} // namespace Utils
