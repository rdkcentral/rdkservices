#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../CloudStore.h"
#include "CloudStoreImplementationMock.h"
#include "ServiceMock.h"
#include <interfaces/json/JsonData_Store2.h>

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::Test;
using ::WPEFramework::Core::PublishedServiceType;
using ::WPEFramework::Exchange::IStore2;
using ::WPEFramework::JsonData::Store2::DeleteNamespaceParamsData;
using ::WPEFramework::JsonData::Store2::GetValueParamsInfo;
using ::WPEFramework::JsonData::Store2::GetValueResultData;
using ::WPEFramework::JsonData::Store2::SetValueParamsData;
using ::WPEFramework::Plugin::CloudStore;
using ::WPEFramework::PluginHost::IDispatcher;
using ::WPEFramework::PluginHost::IPlugin;

const auto kValue = "value_1";
const auto kKey = "key_1";
const auto kAppId = "app_id_1";
const auto kTtl = 100;

class ACloudStore : public Test {
protected:
    NiceMock<ServiceMock>* service;
    IPlugin* plugin;
    ACloudStore()
        : service(WPEFramework::Core::Service<NiceMock<ServiceMock>>::Create<NiceMock<ServiceMock>>())
        , plugin(WPEFramework::Core::Service<CloudStore>::Create<IPlugin>())
    {
    }
    ~ACloudStore() override
    {
        plugin->Release();
        service->Release();
    }
};

TEST_F(ACloudStore, GetsValueInAccountScopeViaJsonRpc)
{
    class CloudStoreImplementation : public NiceMock<CloudStoreImplementationMock> {
    public:
        CloudStoreImplementation()
        {
            EXPECT_CALL(*this, GetValue(_, _, _, _, _))
                .WillRepeatedly(Invoke(
                    [](const IStore2::ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) {
                        EXPECT_THAT(scope, Eq(IStore2::ScopeType::ACCOUNT));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        value = kValue;
                        ttl = kTtl;
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<CloudStoreImplementation> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<IDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    GetValueParamsInfo params;
    params.Scope = IStore2::ScopeType::ACCOUNT;
    params.Ns = kAppId;
    params.Key = kKey;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    ASSERT_THAT(jsonRpc->Invoke(0, 0, "", "getValue", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    GetValueResultData result;
    result.FromString(resultJsonStr);
    EXPECT_THAT(result.Value.Value(), Eq(kValue));
    EXPECT_THAT(result.Ttl.Value(), Eq(kTtl));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(ACloudStore, SetsValueInAccountScopeViaJsonRpc)
{
    class CloudStoreImplementation : public NiceMock<CloudStoreImplementationMock> {
    public:
        CloudStoreImplementation()
        {
            EXPECT_CALL(*this, SetValue(_, _, _, _, _))
                .WillRepeatedly(Invoke(
                    [](const IStore2::ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) {
                        EXPECT_THAT(scope, Eq(IStore2::ScopeType::ACCOUNT));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        EXPECT_THAT(value, Eq(kValue));
                        EXPECT_THAT(ttl, Eq(kTtl));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<CloudStoreImplementation> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<IDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    SetValueParamsData params;
    params.Scope = IStore2::ScopeType::ACCOUNT;
    params.Ns = kAppId;
    params.Key = kKey;
    params.Value = kValue;
    params.Ttl = kTtl;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "setValue", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(ACloudStore, DeletesKeyInAccountScopeViaJsonRpc)
{
    class CloudStoreImplementation : public NiceMock<CloudStoreImplementationMock> {
    public:
        CloudStoreImplementation()
        {
            EXPECT_CALL(*this, DeleteKey(_, _, _))
                .WillRepeatedly(Invoke(
                    [](const IStore2::ScopeType scope, const string& ns, const string& key) {
                        EXPECT_THAT(scope, Eq(IStore2::ScopeType::ACCOUNT));
                        EXPECT_THAT(ns, Eq(kAppId));
                        EXPECT_THAT(key, Eq(kKey));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<CloudStoreImplementation> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<IDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    GetValueParamsInfo params;
    params.Scope = IStore2::ScopeType::ACCOUNT;
    params.Ns = kAppId;
    params.Key = kKey;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteKey", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}

TEST_F(ACloudStore, DeletesNamespaceInAccountScopeViaJsonRpc)
{
    class CloudStoreImplementation : public NiceMock<CloudStoreImplementationMock> {
    public:
        CloudStoreImplementation()
        {
            EXPECT_CALL(*this, DeleteNamespace(_, _))
                .WillRepeatedly(Invoke(
                    [](const IStore2::ScopeType scope, const string& ns) {
                        EXPECT_THAT(scope, Eq(IStore2::ScopeType::ACCOUNT));
                        EXPECT_THAT(ns, Eq(kAppId));
                        return WPEFramework::Core::ERROR_NONE;
                    }));
        }
    };
    PublishedServiceType<CloudStoreImplementation> metadata(WPEFramework::Core::System::MODULE_NAME, 1, 0, 0);
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto jsonRpc = plugin->QueryInterface<IDispatcher>();
    ASSERT_THAT(jsonRpc, NotNull());
    DeleteNamespaceParamsData params;
    params.Scope = IStore2::ScopeType::ACCOUNT;
    params.Ns = kAppId;
    string paramsJsonStr;
    params.ToString(paramsJsonStr);
    string resultJsonStr;
    EXPECT_THAT(jsonRpc->Invoke(0, 0, "", "deleteNamespace", paramsJsonStr, resultJsonStr), Eq(WPEFramework::Core::ERROR_NONE));
    jsonRpc->Release();
    plugin->Deinitialize(service);
}
