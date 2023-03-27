#include "Dobby.h"

std::shared_ptr<AI_IPC::IIpcService> AI_IPC::createIpcService(const std::string& address, const std::string& serviceName, int defaultTimeoutMs)
{
    return std::make_shared<IpcService>(address, serviceName, defaultTimeoutMs);
}