#pragma once

#include "Module.h"

class FactoriesImplementation : public WPEFramework::PluginHost::IFactories {
public:
    FactoriesImplementation(const FactoriesImplementation&) = delete;
    FactoriesImplementation& operator=(const FactoriesImplementation&) = delete;

    FactoriesImplementation()
        : _requestFactory(5)
        , _responseFactory(5)
        , _fileBodyFactory(5)
        , _jsonRPCFactory(5)
    {
    }
    ~FactoriesImplementation() override = default;

public:
    WPEFramework::Core::ProxyType<WPEFramework::Web::Request> Request() override
    {
        return (_requestFactory.Element());
    }
    WPEFramework::Core::ProxyType<WPEFramework::Web::Response> Response() override
    {
        return (_responseFactory.Element());
    }
    WPEFramework::Core::ProxyType<WPEFramework::Web::FileBody> FileBody() override
    {
        return (_fileBodyFactory.Element());
    }
    WPEFramework::Core::ProxyType<WPEFramework::Web::JSONBodyType<WPEFramework::Core::JSONRPC::Message>> JSONRPC() override
    {
        return (WPEFramework::Core::ProxyType<WPEFramework::Web::JSONBodyType<WPEFramework::Core::JSONRPC::Message>>(_jsonRPCFactory.Element()));
    }

private:
    WPEFramework::Core::ProxyPoolType<WPEFramework::Web::Request> _requestFactory;
    WPEFramework::Core::ProxyPoolType<WPEFramework::Web::Response> _responseFactory;
    WPEFramework::Core::ProxyPoolType<WPEFramework::Web::FileBody> _fileBodyFactory;
    WPEFramework::Core::ProxyPoolType<WPEFramework::PluginHost::JSONRPCMessage> _jsonRPCFactory;
};
