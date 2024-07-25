#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Store2.h"
#include "SecureStorageServiceMock.h"
#include "Server.h"

using ::distp::gateway::secure_storage::v1::DeleteAllValuesRequest;
using ::distp::gateway::secure_storage::v1::DeleteAllValuesResponse;
using ::distp::gateway::secure_storage::v1::DeleteValueRequest;
using ::distp::gateway::secure_storage::v1::DeleteValueResponse;
using ::distp::gateway::secure_storage::v1::GetValueRequest;
using ::distp::gateway::secure_storage::v1::GetValueResponse;
using ::distp::gateway::secure_storage::v1::Key;
using ::distp::gateway::secure_storage::v1::Scope;
using ::distp::gateway::secure_storage::v1::UpdateValueRequest;
using ::distp::gateway::secure_storage::v1::UpdateValueResponse;
using ::distp::gateway::secure_storage::v1::Value;
using ::testing::_;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Invoke;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Le;
using ::testing::NiceMock;
using ::testing::Test;
using ::WPEFramework::Core::Time;
using ::WPEFramework::Exchange::IStore2;
using ::WPEFramework::Plugin::Grpc::Store2;

const auto kUri = "0.0.0.0:50051";
const auto kValue = "value_1";
const auto kKey = "key_1";
const auto kAppId = "app_id_1";
const auto kTtl = 100;
const auto kScope = Scope::SCOPE_ACCOUNT;

class AStore2 : public Test {
protected:
    NiceMock<SecureStorageServiceMock> service;
    Server server;
    WPEFramework::Core::ProxyType<IStore2> store2;
    AStore2()
        : server(kUri, &service)
        , store2(WPEFramework::Core::ProxyType<Store2>::Create(kUri))
    {
    }
};

TEST_F(AStore2, GetsValueWithTtl)
{
    GetValueRequest req;
    ON_CALL(service, GetValue(_, _, _))
        .WillByDefault(Invoke(
            [&](::grpc::ServerContext*, const GetValueRequest* request, GetValueResponse* response) {
                req = (*request);
                auto v = new Value();
                v->set_value(kValue);
                auto t = new google::protobuf::Duration();
                t->set_seconds(kTtl);
                v->set_allocated_ttl(t);
                auto k = new Key();
                k->set_key(request->key().key());
                k->set_app_id(request->key().app_id());
                k->set_scope(request->key().scope());
                v->set_allocated_key(k);
                response->set_allocated_value(v);
                return grpc::Status::OK;
            }));

    string v;
    uint32_t t;
    ASSERT_THAT(store2->GetValue(IStore2::ScopeType::ACCOUNT, kAppId, kKey, v, t), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(req.has_key(), IsTrue());
    EXPECT_THAT(req.key().key(), Eq(kKey));
    EXPECT_THAT(req.key().app_id(), Eq(kAppId));
    EXPECT_THAT(req.key().scope(), Eq(kScope));
    EXPECT_THAT(v, Eq(kValue));
    EXPECT_THAT(t, Eq(kTtl));
}

TEST_F(AStore2, GetsValueWithExpireTime)
{
    GetValueRequest req;
    ON_CALL(service, GetValue(_, _, _))
        .WillByDefault(Invoke(
            [&](::grpc::ServerContext*, const GetValueRequest* request, GetValueResponse* response) {
                req = (*request);
                auto v = new Value();
                v->set_value(kValue);
                auto t = new google::protobuf::Timestamp();
                t->set_seconds(kTtl + (Time::Now().Ticks() / Time::TicksPerMillisecond / 1000));
                v->set_allocated_expire_time(t);
                auto k = new Key();
                k->set_key(request->key().key());
                k->set_app_id(request->key().app_id());
                k->set_scope(request->key().scope());
                v->set_allocated_key(k);
                response->set_allocated_value(v);
                return grpc::Status::OK;
            }));

    string v;
    uint32_t t;
    ASSERT_THAT(store2->GetValue(IStore2::ScopeType::ACCOUNT, kAppId, kKey, v, t), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(req.has_key(), IsTrue());
    EXPECT_THAT(req.key().key(), Eq(kKey));
    EXPECT_THAT(req.key().app_id(), Eq(kAppId));
    EXPECT_THAT(req.key().scope(), Eq(kScope));
    EXPECT_THAT(v, Eq(kValue));
    EXPECT_THAT(t, Le(kTtl));
    EXPECT_THAT(t, Gt(0));
}

TEST_F(AStore2, SetsValueWithTtl)
{
    UpdateValueRequest req;
    ON_CALL(service, UpdateValue(_, _, _))
        .WillByDefault(Invoke(
            [&](::grpc::ServerContext*, const UpdateValueRequest* request, UpdateValueResponse*) {
                req = (*request);
                return grpc::Status::OK;
            }));

    ASSERT_THAT(store2->SetValue(IStore2::ScopeType::ACCOUNT, kAppId, kKey, kValue, kTtl), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(req.has_value(), IsTrue());
    EXPECT_THAT(req.value().value(), Eq(kValue));
    ASSERT_THAT(req.value().has_key(), IsTrue());
    EXPECT_THAT(req.value().key().key(), Eq(kKey));
    EXPECT_THAT(req.value().key().app_id(), Eq(kAppId));
    EXPECT_THAT(req.value().key().scope(), Eq(kScope));
    ASSERT_THAT(req.value().has_ttl(), IsTrue());
    EXPECT_THAT(req.value().ttl().seconds(), Eq(kTtl));
}

TEST_F(AStore2, DeletesKey)
{
    DeleteValueRequest req;
    ON_CALL(service, DeleteValue(_, _, _))
        .WillByDefault(Invoke(
            [&](::grpc::ServerContext*, const DeleteValueRequest* request, DeleteValueResponse*) {
                req = (*request);
                return grpc::Status::OK;
            }));

    ASSERT_THAT(store2->DeleteKey(IStore2::ScopeType::ACCOUNT, kAppId, kKey), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(req.has_key(), IsTrue());
    EXPECT_THAT(req.key().key(), Eq(kKey));
    EXPECT_THAT(req.key().app_id(), Eq(kAppId));
    EXPECT_THAT(req.key().scope(), Eq(kScope));
}

TEST_F(AStore2, DeletesNamespace)
{
    DeleteAllValuesRequest req;
    ON_CALL(service, DeleteAllValues(_, _, _))
        .WillByDefault(Invoke(
            [&](::grpc::ServerContext*, const DeleteAllValuesRequest* request, DeleteAllValuesResponse*) {
                req = (*request);
                return grpc::Status::OK;
            }));

    ASSERT_THAT(store2->DeleteNamespace(IStore2::ScopeType::ACCOUNT, kAppId), Eq(WPEFramework::Core::ERROR_NONE));
    ASSERT_THAT(req.app_id(), Eq(kAppId));
    EXPECT_THAT(req.scope(), Eq(kScope));
}
