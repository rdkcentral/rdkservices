#pragma once

#include <initializer_list>
#include <utility>

#include <core/JSONRPC.h>
#include <core/JSON.h>

template<typename T = JsonObject, typename U, typename... Args>
void RegisterMethodVersions(U* obj, std::initializer_list<int> versions, Args&&... args) {
    for (auto version : versions) {
        obj->GetHandler(version)->template Register<JsonObject, T>(std::forward<Args>(args)...);
    }
}

template<typename T = JsonObject, typename U, typename... Args>
void RegisterMethod(U* obj, Args&&... args) {
    uint8_t id = 1;
    WPEFramework::Core::JSONRPC::Handler* handler = obj->GetHandler(id);

    // Go through all created handlers
    while (handler) {
        handler->template Register<JsonObject, T>(std::forward<Args>(args)...);
        handler = obj->GetHandler(++id);
    }
}

template<typename T, typename... Args>
void UnregisterMethodVersions(T* obj, std::initializer_list<int> versions, Args&&... args) {
    for (auto version : versions) {
        obj->GetHandler(version)->Unregister(std::forward<Args>(args)...);
    }
}

template<typename T, typename... Args>
void UnregisterMethod(T* obj, Args&&... args) {
    uint8_t id = 1;
    WPEFramework::Core::JSONRPC::Handler* handler = obj->GetHandler(id);

    // Go through all created handlers
    while (handler) {
        handler->Unregister(std::forward<Args>(args)...);
        handler = obj->GetHandler(++id);
    }
}

template<typename T, typename U>
void NotifyEvent(T* obj, const char* event, const U& params) {    
    std::string json;
    params.ToString(json);
    LOGINFO("Notify %s %s", event, json.c_str());
    
    uint8_t id = 1;;
    WPEFramework::Core::JSONRPC::Handler* handler = obj->GetHandler(id);

    // Go through all created handlers
    while (handler) {
        handler->Notify(event, params);
        handler = obj->GetHandler(++id);
    }
}