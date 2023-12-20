#include "Dobby.h"
#include <gmock/gmock.h>

std::shared_ptr<AI_IPC::IIpcService> AI_IPC::createIpcService(const std::string& address, const std::string& serviceName, int defaultTimeoutMs)
{
    return std::make_shared<IpcService>(address, serviceName, defaultTimeoutMs);
}
    IDobbyProxy* DobbyProxy::impl = nullptr;

    DobbyProxy::DobbyProxy() {}

    void DobbyProxy::setImpl(IDobbyProxy* newImpl)
    {
       // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
       EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
       impl = newImpl;
    }

    DobbyProxy::DobbyProxy(const std::shared_ptr<AI_IPC::IIpcService>& ipcService,
               const std::string& serviceName,
               const std::string& objectName)
    {
    }
    bool  DobbyProxy::shutdown() const
    {
        EXPECT_NE(impl, nullptr);
        return impl->shutdown();
    }

    bool DobbyProxy::ping() const
    {
        EXPECT_NE(impl, nullptr);
        return impl->ping();
    }

    bool DobbyProxy::isAlive(const std::chrono::milliseconds& timeout) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->isAlive(timeout);
    }

    bool DobbyProxy::setLogMethod(uint32_t method, int pipeFd) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->setLogMethod(method, pipeFd);
    }

    bool DobbyProxy::setLogLevel(int level) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->setLogLevel(level);
    }

    bool DobbyProxy::setAIDbusAddress(bool privateBus,
                          const std::string& address) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->setAIDbusAddress(privateBus, address);
    }

    // Control interface
    int32_t DobbyProxy::startContainerFromSpec(const std::string& id,
                                   const std::string& jsonSpec,
                                   const std::list<int>& files,
                                   const std::string& command ,
                                   const std::string& displaySocket ,
                                   const std::vector<std::string>& envVars ) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->startContainerFromSpec(id, jsonSpec, files, command, displaySocket, envVars);
    }


    int32_t DobbyProxy::startContainerFromBundle(const std::string& id,
                                     const std::string& bundlePath,
                                     const std::list<int>& files,
                                     const std::string& command ,
                                     const std::string& displaySocket,
                                     const std::vector<std::string>& envVars ) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->startContainerFromBundle(id, bundlePath, files, command, displaySocket, envVars);
    }

    bool DobbyProxy::stopContainer(int32_t cd, bool withPrejudice) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->stopContainer(cd, withPrejudice);
    }

    bool DobbyProxy::pauseContainer(int32_t cd) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->pauseContainer(cd);
    }

    bool DobbyProxy::resumeContainer(int32_t cd) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->resumeContainer(cd);
    }

    bool DobbyProxy::execInContainer(int32_t cd,
                         const std::string& options,
                         const std::string& command) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->execInContainer(cd, options, command);
    }

    int DobbyProxy::getContainerState(int32_t cd) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->getContainerState(cd);
    }

    int DobbyProxy::registerListener(const StateChangeListener &listener, const void* cbParams)
    {
        EXPECT_NE(impl, nullptr);
        return impl->registerListener(listener, cbParams);
    }

    void DobbyProxy::unregisterListener(int tag)
    {
        EXPECT_NE(impl, nullptr);
        impl->unregisterListener(tag);
    }

    std::string DobbyProxy::getContainerInfo(int32_t descriptor) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->getContainerInfo(descriptor);
    }

    std::list<std::pair<int32_t, std::string>> DobbyProxy::listContainers() const
    {
        EXPECT_NE(impl, nullptr);
        return impl->listContainers();
    }

    AI_IPC::IIpcService* IpcService::impl = nullptr;

    IpcService::IpcService() {}

        IpcService::IpcService(const std::string& dbusAddress, const std::string& serviceName, int defaultTimeoutMs)
    {
    }

    void IpcService::setImpl(AI_IPC::IIpcService* newImpl)
    {
       // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
       EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
       impl = newImpl;
    }

    bool IpcService::isValid() const
    {
        EXPECT_NE(impl, nullptr);
        return impl->isValid();
    }

    void IpcService::flush()
    {
        EXPECT_NE(impl, nullptr);
        return impl->flush();
    }

    bool IpcService::start()
    {
        EXPECT_NE(impl, nullptr);
        return impl->start();
    }

    bool IpcService::stop()
    {
        EXPECT_NE(impl, nullptr);
        return impl->stop();
    }

    bool IpcService::isServiceAvailable(const std::string& serviceName) const
    {
        EXPECT_NE(impl, nullptr);
        return impl->isServiceAvailable(serviceName);
    }

    std::string IpcService::getBusAddress() const
    {
        EXPECT_NE(impl, nullptr);
        return impl->getBusAddress();
    }


