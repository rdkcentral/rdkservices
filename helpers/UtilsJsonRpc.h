#pragma once

#include "UtilsLogging.h"

#define LOGINFOMETHOD() { std::string json; parameters.ToString(json); LOGINFO( "params=%s", json.c_str() ); }
#define LOGTRACEMETHODFIN() { std::string json; response.ToString(json); LOGINFO( "response=%s", json.c_str() ); }
#define returnResponse(success) \
    { \
        response["success"] = success; \
        LOGTRACEMETHODFIN(); \
        return (WPEFramework::Core::ERROR_NONE); \
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
#define returnIfNumberParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != WPEFramework::Core::JSON::Variant::type::NUMBER) \
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
#define getNumberParameterObject(parameters, paramName, param) { \
    if (WPEFramework::Core::JSON::Variant::type::NUMBER == parameters[paramName].Content()) \
        param = parameters[paramName].Number(); \
    else \
        try {param = std::stoi( parameters[paramName].String());} \
        catch (...) { param = 0; } \
}
#define getBoolParameter(paramName, param) { \
    if (WPEFramework::Core::JSON::Variant::type::BOOLEAN == parameters[paramName].Content()) \
        param = parameters[paramName].Boolean(); \
    else \
        param = parameters[paramName].String() == "true" || parameters[paramName].String() == "1"; \
}
#define getStringParameter(paramName, param) { \
    if (WPEFramework::Core::JSON::Variant::type::STRING == parameters[paramName].Content()) \
        param = parameters[paramName].String(); \
}
#define getFloatParameter(paramName, param) { \
    if (Core::JSON::Variant::type::FLOAT == parameters[paramName].Content()) \
        param = parameters[paramName].Float(); \
    else \
        try { param = std::stof( parameters[paramName].String()); } \
        catch (...) { param = 0; } \
}
#define vectorSet(v,s) \
    if (find(begin(v), end(v), s) == end(v)) \
        v.emplace_back(s);
#define returnIfWrongApiVersion(version)\
    if(m_apiVersionNumber < version) \
    { \
        LOGWARN("method %s not supported. version required=%u actual=%u", __FUNCTION__, version, m_apiVersionNumber); \
        returnResponse(false); \
    }
#define returnIfArrayParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != WPEFramework::Core::JSON::Variant::type::ARRAY) \
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
#define getDefaultBoolParameter(paramName, param, default) { \
    if (parameters.HasLabel(paramName)) { \
        if (WPEFramework::Core::JSON::Variant::type::BOOLEAN == parameters[paramName].Content()) \
            param = parameters[paramName].Boolean(); \
        else \
            param = parameters[paramName].String() == "true" || parameters[paramName].String() == "1"; \
     } else param = default; \
}
