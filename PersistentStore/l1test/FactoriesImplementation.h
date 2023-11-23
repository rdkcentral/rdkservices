#pragma once

#include <gmock/gmock.h>

#include "../Module.h"

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
        // Defaults:
        ON_CALL(*this, Request())
            .WillByDefault(::testing::Invoke(
                [&]() { return (_requestFactory.Element()); }));
        ON_CALL(*this, Response())
            .WillByDefault(::testing::Invoke(
                [&]() { return (_responseFactory.Element()); }));
        ON_CALL(*this, FileBody())
            .WillByDefault(::testing::Invoke(
                [&]() { return (_fileBodyFactory.Element()); }));
        ON_CALL(*this, JSONRPC())
            .WillByDefault(::testing::Invoke(
                [&]() {
                    return (WPEFramework::Core::ProxyType<WPEFramework::Web::JSONRPC::Body>(_jsonRPCFactory.Element()));
                }));
    }

    virtual ~FactoriesImplementation() = default;

    MOCK_METHOD(WPEFramework::Core::ProxyType<WPEFramework::Web::Request>, Request, (), (override));
    MOCK_METHOD(WPEFramework::Core::ProxyType<WPEFramework::Web::Response>, Response, (), (override));
    MOCK_METHOD(WPEFramework::Core::ProxyType<WPEFramework::Web::FileBody>, FileBody, (), (override));
    MOCK_METHOD(WPEFramework::Core::ProxyType<WPEFramework::Web::JSONRPC::Body>, JSONRPC, (), (override));

private:
    WPEFramework::Core::ProxyPoolType<WPEFramework::Web::Request> _requestFactory;
    WPEFramework::Core::ProxyPoolType<WPEFramework::Web::Response> _responseFactory;
    WPEFramework::Core::ProxyPoolType<WPEFramework::Web::FileBody> _fileBodyFactory;
    WPEFramework::Core::ProxyPoolType<WPEFramework::PluginHost::JSONRPCMessage> _jsonRPCFactory;
};
