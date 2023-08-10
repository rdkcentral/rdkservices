#pragma once
class NetworkmanagerAdapter {
public:
    virtual int32_t IsConnectedToInternet(const string &message, string &result /* @out */) = 0;
//    int32_t GetIPSettings (jsonobject a, jsonobject b) = 0;
    virtual uint32_t getIPSettings(const JsonObject& parameters, JsonObject& response) = 0;
};

