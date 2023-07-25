#pragma once
#include <string>
#include <list>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace avahi{
extern const std::string RDK_SERVICE_NAME;
const int DD_TIMEOUT_MILLIS = 5000;
const int DD_SCAN_ALREADY_IN_PROGRESS = -1;

enum IP_ADDR_TYPE
{
    IPV4,
    IPV6
};
typedef struct _RDKDevice
{

    _RDKDevice()
    {
        std::cout << "Device created." << std::endl;
    }
    ~_RDKDevice()
    {
        std::cout << "Device removed." << std::endl;
    }
    // No copying
    _RDKDevice(const _RDKDevice &) = delete;
    _RDKDevice &operator=(const _RDKDevice &) = delete;

    std::string ipAddress;
    std::string deviceName;
    IP_ADDR_TYPE addrType;
    uint port;
} RDKDevice;

bool initialize(const std::string &serviceName = RDK_SERVICE_NAME);
int discoverDevices(std::list<std::shared_ptr<RDKDevice> > &devices, int timeoutMillis = DD_TIMEOUT_MILLIS);
bool unInitialize();
bool stopDiscovery();
}