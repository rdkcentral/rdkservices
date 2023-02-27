#pragma once

#include <list>
#include <memory>
#include <thread>
#include <mutex>
#include <set>
#include <map>
#include <functional>
#include <condition_variable>
#include <deque>


#if defined(DOBBY_SERVICE_OVERRIDE)
    #define DOBBY_SERVICE   DOBBY_SERVICE_OVERRIDE
#else
    #define DOBBY_SERVICE   "org.rdk.dobby"
#endif
#if defined(DOBBY_OBJECT_OVERRIDE)
    #define DOBBY_OBJECT   DOBBY_OBJECT_OVERRIDE
#else
    #define DOBBY_OBJECT   "/org/rdk/dobby"
#endif

namespace AI_IPC
{

class IIpcService {
public:
    virtual ~IIpcService() = default;
    virtual bool isValid() const = 0;
    virtual bool isServiceAvailable(const std::string& serviceName) const = 0;
    virtual void flush() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual std::string getBusAddress() const = 0;
};

std::shared_ptr<IIpcService> createIpcService(const std::string& address, const std::string& serviceName, int defaultTimeoutMs = -1);

} // AI_IPC


class IpcService : public AI_IPC::IIpcService
                 , public std::enable_shared_from_this<IpcService> {

public:
    static IpcService& getInstance()
    {
        static IpcService instance;
        return instance;
    }

    IIpcService* impl;

    IpcService(const std::string& dbusAddress, const std::string& serviceName, int defaultTimeoutMs = -1)
    {
    }

    IpcService()
    {
    }

    bool isValid() const 
    {
        return getInstance().impl->isValid();
    }

    void flush()
    {
        return getInstance().impl->flush();
    }

    bool start()
    {
        return getInstance().impl->start();
    }

    bool stop()
    {
        return getInstance().impl->stop();
    }

    bool isServiceAvailable(const std::string& serviceName) const
    {
        return getInstance().impl->isServiceAvailable(serviceName);
    }

    std::string getBusAddress() const
    {
        return getInstance().impl->getBusAddress();
    }
};

class IDobbyProxyEvents {

public:
    enum ContainerState {
        Invalid = 0,
        Starting = 1,
        Running = 2,
        Stopping = 3,
        Paused = 4,
        Stopped = 5,
    };

public:
    virtual ~IDobbyProxyEvents() = default;

};

typedef std::function<void(int32_t, const std::string&, IDobbyProxyEvents::ContainerState, const void*)> StateChangeListener;

class IDobbyProxy {

public:
    ~IDobbyProxy() = default;

    virtual bool shutdown() const = 0;
    virtual bool ping() const = 0;
    virtual bool isAlive(const std::chrono::milliseconds& timeout) const = 0;
    virtual bool setLogMethod(uint32_t method, int pipeFd) const = 0;
    virtual bool setLogLevel(int level) const = 0;
    virtual bool setAIDbusAddress(bool privateBus,
                                  const std::string& address) const = 0;

    inline bool isAlive() const
    {
        return isAlive(std::chrono::milliseconds::min());
    }

    inline int32_t setLogMethod(uint32_t method) const
    {
        return setLogMethod(method, -1);
    }


    virtual int32_t startContainerFromSpec(const std::string& id,
                                           const std::string& jsonSpec,
                                           const std::list<int>& files,
                                           const std::string& command = "",
                                           const std::string& displaySocket = "",
                                           const std::vector<std::string>& envVars = std::vector<std::string>()) const = 0;
    virtual int32_t startContainerFromBundle(const std::string& id,
                                             const std::string& bundlePath,
                                             const std::list<int>& files,
                                             const std::string& command = "",
                                             const std::string& displaySocket = "",
                                             const std::vector<std::string>& envVars = std::vector<std::string>()) const = 0;
    virtual bool stopContainer(int32_t descriptor,
                               bool withPrejudice) const = 0;
    virtual bool pauseContainer(int32_t descriptor) const = 0;
    virtual bool resumeContainer(int32_t descriptor) const = 0;
    virtual bool execInContainer(int32_t cd,
                                 const std::string& options,
                                 const std::string& command) const = 0;

    virtual int registerListener(const StateChangeListener &listener, const void* cbParams) = 0;
    virtual void unregisterListener(int tag) = 0;
    virtual std::string getContainerInfo(int32_t descriptor) const = 0;
    virtual std::list<std::pair<int32_t, std::string>> listContainers() const = 0;
    virtual int getContainerState(int32_t descriptor) const = 0;


};

class DobbyProxy : public IDobbyProxy {

protected:

public:
    static DobbyProxy& getInstance()
    {
        static DobbyProxy instance;
        return instance;
    }

    IDobbyProxy* impl;

    DobbyProxy()
    {
    }

    DobbyProxy(const std::shared_ptr<AI_IPC::IIpcService>& ipcService,
               const std::string& serviceName,
               const std::string& objectName)
    {
    }

    bool  shutdown() const
    {
        return getInstance().impl->shutdown();
    }

    bool ping() const
    {
        return getInstance().impl->ping();
    }

    bool isAlive(const std::chrono::milliseconds& timeout) const
    {
        return getInstance().impl->isAlive(timeout);
    }

    bool setLogMethod(uint32_t method, int pipeFd) const
    {
        return getInstance().impl->setLogMethod(method, pipeFd);
    }

    bool setLogLevel(int level) const
    {
        return getInstance().impl->setLogLevel(level);
    }

    bool setAIDbusAddress(bool privateBus,
                          const std::string& address) const
    {
        return getInstance().impl->setAIDbusAddress(privateBus, address);
    }

public:
    // Control interface
    int32_t startContainerFromSpec(const std::string& id,
                                   const std::string& jsonSpec,
                                   const std::list<int>& files,
                                   const std::string& command = "",
                                   const std::string& displaySocket = "",
                                   const std::vector<std::string>& envVars = std::vector<std::string>()) const
    {
        return getInstance().impl->startContainerFromSpec(id, jsonSpec, files, command, displaySocket, envVars);
    }


    int32_t startContainerFromBundle(const std::string& id,
                                     const std::string& bundlePath,
                                     const std::list<int>& files,
                                     const std::string& command = "",
                                     const std::string& displaySocket = "",
                                     const std::vector<std::string>& envVars = std::vector<std::string>()) const
    {
        return getInstance().impl->startContainerFromBundle(id, bundlePath, files, command, displaySocket, envVars);
    }

    bool stopContainer(int32_t cd, bool withPrejudice) const
    {
        return getInstance().impl->stopContainer(cd, withPrejudice);
    }

    bool pauseContainer(int32_t cd) const
    {
        return getInstance().impl->pauseContainer(cd);
    }

    bool resumeContainer(int32_t cd) const
    {
        return getInstance().impl->resumeContainer(cd);
    }

    bool execInContainer(int32_t cd,
                         const std::string& options,
                         const std::string& command) const
    {
        return getInstance().impl->execInContainer(cd, options, command);
    }

    int getContainerState(int32_t cd) const
    {
        return getInstance().impl->getContainerState(cd);
    }

    int registerListener(const StateChangeListener &listener, const void* cbParams)
    {
        return getInstance().impl->registerListener(listener, cbParams);
    }

    void unregisterListener(int tag)
    {
        getInstance().impl->unregisterListener(tag);
    }

    std::string getContainerInfo(int32_t descriptor) const
    {
        return getInstance().impl->getContainerInfo(descriptor);
    }

    std::list<std::pair<int32_t, std::string>> listContainers() const
    {
        return getInstance().impl->listContainers();
    }


private:
    const std::shared_ptr<AI_IPC::IIpcService> mIpcService;

};
