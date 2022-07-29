#pragma once

#include <initializer_list>
#include <utility>

#include <core/JSONRPC.h>
#include <core/JSON.h>

template<typename T = JsonObject, typename U, typename... Args>
void RegisterMethod(U* obj, std::initializer_list<int> versions, Args&&... args) {
    for (auto version : versions) {
        obj->GetHandler(version)->template Register<JsonObject, T>(std::forward<Args>(args)...);
    }
}

template<typename T, typename... Args>
void UnregisterMethod(T* obj, std::initializer_list<int> versions, Args&&... args) {
    for (auto version : versions) {
        obj->GetHandler(version)->Unregister(std::forward<Args>(args)...);
    }
}

template<typename T, typename U>
void NotifyEvent(T* obj, std::initializer_list<int> versions, const char* event, const U& params) {    
    std::string json;
    params.ToString(json);
    LOGINFO("Notify %s %s", event, json.c_str());
    
    for (auto version : versions) {
        obj->GetHandler(version)->Notify(event, params);
    }
}