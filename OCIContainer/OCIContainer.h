#pragma once

#include "Module.h"

#include <Dobby/DobbyProtocol.h>
#include <Dobby/Public/Dobby/IDobbyProxy.h>

#include <vector>
#include <map>

namespace WPEFramework
{

namespace Plugin
{
// This is a server for a JSONRPC communication channel.
// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
// This realization of this interface implements, by default, the following methods on this plugin
// - exists
// - register
// - unregister
// Any other methood to be handled by this plugin  can be added can be added by using the
// templated methods Register on the PluginHost::JSONRPC class.
// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
// this class exposes a public method called, Notify(), using this methods, all subscribed clients
// will receive a JSONRPC message as a notification, in case this method is called.
class OCIContainer : public PluginHost::IPlugin, public PluginHost::JSONRPC
{
public:
    OCIContainer();
    virtual ~OCIContainer();
    OCIContainer(const OCIContainer &) = delete;
    OCIContainer &operator=(const OCIContainer &) = delete;

    //Begin methods
    uint32_t listContainers(const JsonObject &parameters, JsonObject &response);
    uint32_t getContainerState(const JsonObject &parameters, JsonObject &response);
    uint32_t getContainerInfo(const JsonObject &parameters, JsonObject &response);
    uint32_t startContainer(const JsonObject &parameters, JsonObject &response);
    uint32_t startContainerFromDobbySpec(const JsonObject &parameters, JsonObject &response);
    uint32_t stopContainer(const JsonObject &parameters, JsonObject &response);
    uint32_t pauseContainer(const JsonObject &parameters, JsonObject &response);
    uint32_t resumeContainer(const JsonObject &parameters, JsonObject &response);
    uint32_t executeCommand(const JsonObject &parameters, JsonObject &response);
    //End methods

    //Begin events
    void onContainerStarted(int32_t descriptor, const std::string& name);
    void onContainerStopped(int32_t descriptor, const std::string& name);
    //End events

    //Build QueryInterface implementation, specifying all possible interfaces to be returned.
    BEGIN_INTERFACE_MAP(OCIContainer)
    INTERFACE_ENTRY(PluginHost::IPlugin)
    INTERFACE_ENTRY(PluginHost::IDispatcher)
    END_INTERFACE_MAP

    //IPlugin methods
    virtual const string Initialize(PluginHost::IShell *service) override;
    virtual void Deinitialize(PluginHost::IShell *service) override;
    virtual string Information() const override;

    uint32_t getApiVersionNumber() const { return 1; };

private:
    int mEventListenerId; // Dobby event listener ID
    std::shared_ptr<IDobbyProxy> mDobbyProxy; // DobbyProxy instance
    std::shared_ptr<AI_IPC::IIpcService> mIpcService; // Ipc Service instance
    const int GetContainerDescriptorFromId(const std::string& containerId);
    static const void stateListener(int32_t descriptor, const std::string& name, IDobbyProxyEvents::ContainerState state, const void* _this);
};
} // namespace Plugin
} // namespace WPEFramework
