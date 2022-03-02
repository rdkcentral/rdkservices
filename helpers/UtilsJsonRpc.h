#pragma once

#include "UtilsLogging.h"

#define UNUSED(expr)(void)(expr)
#define LOGINFOMETHOD() { std::string json; parameters.ToString(json); LOGINFO( "params=%s", json.c_str() ); }
#define LOGTRACEMETHODFIN() { std::string json; response.ToString(json); LOGINFO( "response=%s", json.c_str() ); }
#define getNumberParameter(paramName, param) { \
    if (Core::JSON::Variant::type::NUMBER == parameters[paramName].Content()) \
        param = parameters[paramName].Number(); \
}
#define returnResponse(success) \
    { \
        response["success"] = success; \
        LOGTRACEMETHODFIN(); \
        return (Core::ERROR_NONE); \
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
#define returnIfNumberParamNotFound(param, name) \
    if (!param.HasLabel(name) || param[name].Content() != Core::JSON::Variant::type::NUMBER) \
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
