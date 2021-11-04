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

// telemetry
#ifdef ENABLE_TELEMETRY_LOGGING
#include <telemetry_busmessage_sender.h>
#endif

// IARM
#include "rdk/iarmbus/libIARM.h"

// std
#include <string>
#include <thread>

#define UNUSED(expr)(void)(expr)
#define C_STR(x) (x).c_str()

#define LOGINFO(fmt, ...) do { fprintf(stderr, "[%d] INFO [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define LOGDBG(fmt, ...) do { fprintf(stderr, "[%d] DEBUG [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define LOGWARN(fmt, ...) do { fprintf(stderr, "[%d] WARN [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define LOGERR(fmt, ...) do { fprintf(stderr, "[%d] ERROR [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); Utils::Telemetry::sendError(fmt, ##__VA_ARGS__); } while (0)

#define LOGINFOMETHOD() { std::string json; parameters.ToString(json); LOGINFO( "params=%s", json.c_str() );  }
#define LOGTRACEMETHODFIN() do { std::string json; response.ToString(json); LOGINFO( "response=%s", json.c_str() );  } while (0)

#define LOG_DEVICE_EXCEPTION0() LOGWARN("Exception caught: code=%d message=%s", err.getCode(), err.what());
#define LOG_DEVICE_EXCEPTION1(param1) LOGWARN("Exception caught" #param1 "=%s code=%d message=%s", param1.c_str(), err.getCode(), err.what());
#define LOG_DEVICE_EXCEPTION2(param1, param2) LOGWARN("Exception caught " #param1 "=%s " #param2 "=%s code=%d message=%s", param1.c_str(), param2.c_str(), err.getCode(), err.what());

//this set of macros are used in the method handlers to make the code more consistent and easier to read
#define vectorSet(v,s) \
    if (find(begin(v), end(v), s) == end(v)) \
        v.emplace_back(s);

#define returnResponse(success) \
    { \
        response["success"] = success; \
        LOGTRACEMETHODFIN(); \
        return (Core::ERROR_NONE); \
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
    if (!param.HasLabel(name) || param[name].Content() != Core::JSON::Variant::type::STRING) \
    {\
        LOGERR("No argument '%s' or it has incorrect type", name); \
        returnResponse(false); \
    }

#define returnIfBooleanParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != Core::JSON::Variant::type::BOOLEAN) \
    { \
        LOGERR("No argument '%s' or it has incorrect type", name); \
        returnResponse(false); \
    }

#define returnIfArrayParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != Core::JSON::Variant::type::ARRAY) \
    { \
        LOGERR("No argument '%s' or it has incorrect type", name); \
        returnResponse(false); \
    }

#define returnIfNumberParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != Core::JSON::Variant::type::NUMBER) \
    { \
        LOGERR("No argument '%s' or it has incorrect type", name); \
        returnResponse(false); \
    }

#define returnIfObjectParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != Core::JSON::Variant::type::OBJECT) \
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
    if (Core::JSON::Variant::type::NUMBER == parameters[paramName].Content()) \
        param = parameters[paramName].Number(); \
    else \
        try { param = std::stoi( parameters[paramName].String()); } \
        catch (...) { param = 0; } \
}

#define getDefaultNumberParameter(paramName, param, default) { \
    if (parameters.HasLabel(paramName)) { \
        if (Core::JSON::Variant::type::NUMBER == parameters[paramName].Content()) \
            param = parameters[paramName].Number(); \
        else \
            try { param = std::stoi( parameters[paramName].String()); } \
            catch (...) { param = default; } \
    } else param = default; \
}

#define getDefaultStringParameter(paramName, param, default) { \
    if (parameters.HasLabel(paramName)) { \
        if (Core::JSON::Variant::type::STRING == parameters[paramName].Content()) \
            param = parameters[paramName].String(); \
        else \
            param = default; \
    } else param = default; \
}

#define getNumberParameterObject(parameters, paramName, param) { \
    if (Core::JSON::Variant::type::NUMBER == parameters[paramName].Content()) \
        param = parameters[paramName].Number(); \
    else \
        try {param = std::stoi( parameters[paramName].String());} \
        catch (...) { param = 0; } \
}

#define getBoolParameter(paramName, param) { \
    if (Core::JSON::Variant::type::BOOLEAN == parameters[paramName].Content()) \
        param = parameters[paramName].Boolean(); \
    else \
        param = parameters[paramName].String() == "true" || parameters[paramName].String() == "1"; \
}

#define getStringParameter(paramName, param) { \
    if (Core::JSON::Variant::type::STRING == parameters[paramName].Content()) \
        param = parameters[paramName].String(); \
}

#define IARM_CHECK(FUNC) { \
    if ((res = FUNC) != IARM_RESULT_SUCCESS) { \
        LOGINFO("IARM %s: %s", #FUNC, \
            res == IARM_RESULT_INVALID_PARAM ? "invalid param" : ( \
            res == IARM_RESULT_INVALID_STATE ? "invalid state" : ( \
            res == IARM_RESULT_IPCCORE_FAIL ? "ipcore fail" : ( \
            res == IARM_RESULT_OOM ? "oom" : "unknown")))); \
    } \
    else \
    { \
        LOGINFO("IARM %s: success", #FUNC); \
    } \
}

namespace Utils
{
    struct IARM {
        static bool init();
        static bool isConnected();

        static const char* NAME;
    };

    namespace String
    {
        // locale-wise comparison
        template<typename charT>
        struct loc_equal {
            explicit loc_equal( const std::locale& loc ) : loc_(loc) {}
            bool operator()(charT ch1, charT ch2) {
                return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
            }
        private:
            const std::locale& loc_;
        };

        // Case-insensitive substring lookup.
        // Returns the substring position or -1
        // Example: int pos = find_substr_ci(string, substring, std::locale());
        template<typename T>
        int find_substr_ci( const T& string, const T& substring, const std::locale& loc = std::locale() )
        {
            typename T::const_iterator it = std::search( string.begin(), string.end(),
                                                         substring.begin(), substring.end(), loc_equal<typename T::value_type>(loc) );
            if ( it != string.end() ) return it - string.begin();
            else return -1; // not found
        }

        // Case-insensitive substring inclusion lookup.
        // Example: if (Utils::String::contains(result, processName)) {..}
        template<typename T>
        bool contains( const T& string, const T& substring, const std::locale& loc = std::locale() )
        {
            int pos = find_substr_ci(string, substring, loc);
            return pos != -1;
        }

        // Case-insensitive substring inclusion lookup.
        // Example: if(Utils::String::contains(tmp, "grep -i")) {..}
        template<typename T>
        bool contains( const T& string, const char* c_substring, const std::locale& loc = std::locale() )
        {
            std::string substring(c_substring);
            int pos = find_substr_ci(string, substring, loc);
            return pos != -1;
        }

        // Case-insensitive string comparison
        // returns true if the strings are equal, otherwise returns false
        // Example: if (Utils::String::equal(line, provisionType)) {..}
        template<typename T>
        bool equal(const T& string, const T& string2, const std::locale& loc = std::locale() )
        {
            int pos = find_substr_ci(string, string2, loc);
            bool res = (pos == 0) && (string.length() == string2.length());
            return res;
        }

        // Case-insensitive string comparison
        // returns true if the strings are equal, otherwise returns false
        // Example: if(Utils::String::equal(line,"CRYPTANIUM")) {..}
        template<typename T>
        bool equal(const T& string, const char* c_string2, const std::locale& loc = std::locale() )
        {
            std::string string2(c_string2);
            int pos = find_substr_ci(string, string2, loc);
            bool res = (pos == 0) && (string.length() == string2.length());
            return res;
        }

        // Trim space characters (' ', '\n', '\v', '\f', \r') on the left side of string
        inline void ltrim(std::string &s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
            }));
        }

        // Trim space characters (' ', '\n', '\v', '\f', \r') on the right side of string
        inline void rtrim(std::string &s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        }

        // Trim space characters (' ', '\n', '\v', '\f', \r') on both sides of string
        inline void trim(std::string &s) {
            ltrim(s);
            rtrim(s);
        }

        inline void toUpper(std::string &s) {
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        }

        inline void toLower(std::string &s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        }

        // case insensitive comparison of strings
        inline bool stringContains(const std::string& s1, const std::string& s2) {
            return search(s1.begin(), s1.end(), s2.begin(), s2.end(), [](char c1, char c2) { return toupper(c1) == toupper(c2); }) != s1.end();
        }

        // case insensitive comparison of strings
        inline bool stringContains(const std::string& s1, const char * s2) {
            return stringContains(s1, std::string(s2));
        }
    }

    /**
     * @brief Format an IARM_Result_t value for error reporting.
     *
     * @param result The result of an IARM bus call.
     * @return       A corresponding string.
     *
     */
    std::string formatIARMResult(IARM_Result_t result);

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
    void syncPersistFile (const string file);
    void persistJsonSettings(const string file, const string strKey, const JsonValue& jsValue);

    //class for std::thread RAII
    class ThreadRAII 
    {
        public:
            ThreadRAII() {}
            ThreadRAII(std::thread&& t);
            ~ThreadRAII(); 
            
            //support moving
            ThreadRAII(ThreadRAII&&) = default;
            ThreadRAII& operator=(ThreadRAII&&) = default;

            std::thread& get() { return t; }

        private:
            std::thread t;
    };

    struct Telemetry
    {
        static void init()
        {
#ifdef ENABLE_TELEMETRY_LOGGING
            t2_init("Thunder_Plugins");
#endif
        };

        static void sendMessage(const char* message)
        {
#ifdef ENABLE_TELEMETRY_LOGGING
            t2_event_s("THUNDER_MESSAGE", message);
#endif
        };

        static void sendMessage(const char *marker, const char* message)
        {
#ifdef ENABLE_TELEMETRY_LOGGING
            t2_event_s(marker, message);
#endif
        };

        static void sendError(const char* format, ...)
        {
#ifdef ENABLE_TELEMETRY_LOGGING
            va_list parameters;
            va_start(parameters, format);
            std::string message;
            WPEFramework::Trace::Format(message, format, parameters);
            va_end(parameters);

            // get rid of const for t2_event_s
            char* error = strdup(message.c_str());
            t2_event_s("THUNDER_ERROR", error);
            if (error)
            {
                free(error);
            }
#endif
        };
    };
} // namespace Utils
