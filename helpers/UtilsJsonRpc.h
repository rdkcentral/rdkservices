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

/**
 * DO NOT USE THIS.
 *
 * You should be capable of just using "Notify".
 */

#define sendNotify(event,params) { \
    std::string json; \
    params.ToString(json); \
    LOGINFO("Notify %s %s", event, json.c_str()); \
    for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Notify(event,params); \
}
#define sendNotifyMaskParameters(event,params) { \
    std::string json; \
    params.ToString(json); \
    LOGINFO("Notify %s <***>", event); \
    for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Notify(event,params); \
}

/**
 * DO NOT USE THIS.
 *
 * Instead, add YOURPLUGINNAME.json to https://github.com/rdkcentral/ThunderInterfaces
 * and use the generated classes from <interfaces/json/JsonData_YOURPLUGINNAME.h>
 */

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
