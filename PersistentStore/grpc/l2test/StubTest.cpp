#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "secure_storage.grpc.pb.h"
#include <grpcpp/create_channel.h>

using ::distp::gateway::secure_storage::v1::DeleteAllValuesRequest;
using ::distp::gateway::secure_storage::v1::DeleteAllValuesResponse;
using ::distp::gateway::secure_storage::v1::DeleteValueRequest;
using ::distp::gateway::secure_storage::v1::DeleteValueResponse;
using ::distp::gateway::secure_storage::v1::GetValueRequest;
using ::distp::gateway::secure_storage::v1::GetValueResponse;
using ::distp::gateway::secure_storage::v1::Key;
using ::distp::gateway::secure_storage::v1::Scope;
using ::distp::gateway::secure_storage::v1::SecureStorageService;
using ::distp::gateway::secure_storage::v1::UpdateValueRequest;
using ::distp::gateway::secure_storage::v1::UpdateValueResponse;
using ::distp::gateway::secure_storage::v1::Value;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Le;
using ::testing::Test;

const auto kUri = "ss.eu.prod.developer.comcast.com:443";
const auto kDevUri = "ss.dev.developer.comcast.com:443";
const auto kValue = "value_1";
const auto kKey = "key_1";
const auto kAppId = "app_id_1";
const auto kTtl = 2;
const auto kTtlFault = 1;
const auto kScope = Scope::SCOPE_ACCOUNT;
const auto kEmpty = "";
const auto kUnknown = "unknown";
const auto kToken = "Bearer TOKEN";

class AStub : public Test {
protected:
    std::unique_ptr<SecureStorageService::Stub> stub;
    AStub()
        : stub(SecureStorageService::NewStub(grpc::CreateChannel(
              kUri,
              grpc::SslCredentials(grpc::SslCredentialsOptions()))))
    {
    }
};

class ADevStub : public Test {
protected:
    std::unique_ptr<SecureStorageService::Stub> stub;
    ADevStub()
        : stub(SecureStorageService::NewStub(grpc::CreateChannel(
              kDevUri,
              grpc::SslCredentials(grpc::SslCredentialsOptions()))))
    {
    }
};

TEST_F(AStub, DoesNotUpdateValueWhenAppIdEmpty)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    UpdateValueRequest request;
    auto v = new Value();
    v->set_value(kValue);
    auto k = new Key();
    k->set_app_id(kEmpty);
    k->set_key(kKey);
    k->set_scope(kScope);
    v->set_allocated_key(k);
    request.set_allocated_value(v);
    UpdateValueResponse response;
    auto status = stub->UpdateValue(&context, request, &response);
    ASSERT_THAT(status.ok(), IsFalse());
    EXPECT_THAT(status.error_code(), Eq(3));
    EXPECT_THAT(status.error_message(), Eq("key's key and app_id fields are required"));
}

TEST_F(AStub, DoesNotUpdateValueWhenKeyEmpty)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    UpdateValueRequest request;
    auto v = new Value();
    v->set_value(kValue);
    auto k = new Key();
    k->set_app_id(kAppId);
    k->set_key(kEmpty);
    k->set_scope(kScope);
    v->set_allocated_key(k);
    request.set_allocated_value(v);
    UpdateValueResponse response;
    auto status = stub->UpdateValue(&context, request, &response);
    ASSERT_THAT(status.ok(), IsFalse());
    EXPECT_THAT(status.error_code(), Eq(3));
    EXPECT_THAT(status.error_message(), Eq("invalid UpdateValueRequest.Value: embedded message failed validation | caused by: invalid Value.Key: embedded message failed validation | caused by: invalid Key.Key: value length must be at least 1 runes"));
}

TEST_F(AStub, DoesNotGetValueWhenAppIdEmpty)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    GetValueRequest request;
    auto k = new Key();
    k->set_app_id(kEmpty);
    k->set_key(kKey);
    k->set_scope(kScope);
    request.set_allocated_key(k);
    GetValueResponse response;
    auto status = stub->GetValue(&context, request, &response);
    ASSERT_THAT(status.ok(), IsFalse());
    EXPECT_THAT(status.error_code(), Eq(3));
    EXPECT_THAT(status.error_message(), Eq("key's key and app_id fields are required"));
}

TEST_F(AStub, DoesNotGetValueWhenKeyEmpty)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    GetValueRequest request;
    auto k = new Key();
    k->set_app_id(kAppId);
    k->set_key(kEmpty);
    k->set_scope(kScope);
    request.set_allocated_key(k);
    GetValueResponse response;
    auto status = stub->GetValue(&context, request, &response);
    ASSERT_THAT(status.ok(), IsFalse());
    EXPECT_THAT(status.error_code(), Eq(3));
    EXPECT_THAT(status.error_message(), Eq("invalid GetValueRequest.Key: embedded message failed validation | caused by: invalid Key.Key: value length must be at least 1 runes"));
}

TEST_F(AStub, DoesNotDeleteValueWhenAppIdEmpty)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    DeleteValueRequest request;
    auto k = new Key();
    k->set_app_id(kEmpty);
    k->set_key(kKey);
    k->set_scope(kScope);
    request.set_allocated_key(k);
    DeleteValueResponse response;
    auto status = stub->DeleteValue(&context, request, &response);
    ASSERT_THAT(status.ok(), IsFalse());
    EXPECT_THAT(status.error_code(), Eq(3));
    EXPECT_THAT(status.error_message(), Eq("key's key and app_id fields are required"));
}

TEST_F(AStub, DoesNotDeleteValueWhenKeyEmpty)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    DeleteValueRequest request;
    auto k = new Key();
    k->set_app_id(kAppId);
    k->set_key(kEmpty);
    k->set_scope(kScope);
    request.set_allocated_key(k);
    DeleteValueResponse response;
    auto status = stub->DeleteValue(&context, request, &response);
    ASSERT_THAT(status.ok(), IsFalse());
    EXPECT_THAT(status.error_code(), Eq(3));
    EXPECT_THAT(status.error_message(), Eq("invalid DeleteValueRequest.Key: embedded message failed validation | caused by: invalid Key.Key: value length must be at least 1 runes"));
}

TEST_F(AStub, DeletesAllValuesWhenAppIdEmpty)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    DeleteAllValuesRequest request;
    request.set_app_id(kEmpty);
    request.set_scope(kScope);
    DeleteAllValuesResponse response;
    auto status = stub->DeleteAllValues(&context, request, &response);
    EXPECT_THAT(status.ok(), IsTrue());
}

TEST_F(AStub, GetsValueWhenValueEmpty)
{
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        UpdateValueRequest request;
        auto v = new Value();
        v->set_value(kEmpty);
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        v->set_allocated_key(k);
        request.set_allocated_value(v);
        UpdateValueResponse response;
        auto status = stub->UpdateValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
    }
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        GetValueRequest request;
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        request.set_allocated_key(k);
        GetValueResponse response;
        auto status = stub->GetValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
        ASSERT_THAT(response.has_value(), IsTrue());
        EXPECT_THAT(response.value().value(), Eq(kEmpty));
        EXPECT_THAT(response.value().has_ttl(), IsFalse());
        EXPECT_THAT(response.value().has_expire_time(), IsFalse());
    }
}

TEST_F(AStub, DoesNotGetValueWhenAppIdUnknown)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    GetValueRequest request;
    auto k = new Key();
    k->set_app_id(kUnknown);
    k->set_key(kKey);
    k->set_scope(kScope);
    request.set_allocated_key(k);
    GetValueResponse response;
    auto status = stub->GetValue(&context, request, &response);
    ASSERT_THAT(status.ok(), IsTrue());
    EXPECT_THAT(response.has_value(), IsFalse());
}

TEST_F(ADevStub, DoesNotGetValueWhenAppIdUnknown)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    GetValueRequest request;
    auto k = new Key();
    k->set_app_id(kUnknown);
    k->set_key(kKey);
    k->set_scope(kScope);
    request.set_allocated_key(k);
    GetValueResponse response;
    auto status = stub->GetValue(&context, request, &response);
    ASSERT_THAT(status.ok(), IsFalse());
    EXPECT_THAT(status.error_code(), Eq(5));
    EXPECT_THAT(status.error_message(), Eq("requested key scope does not exist"));
}

TEST_F(AStub, DoesNotGetValueWhenKeyUnknown)
{
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        UpdateValueRequest request;
        auto v = new Value();
        v->set_value(kValue);
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        v->set_allocated_key(k);
        request.set_allocated_value(v);
        UpdateValueResponse response;
        auto status = stub->UpdateValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
    }
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        GetValueRequest request;
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kUnknown);
        k->set_scope(kScope);
        request.set_allocated_key(k);
        GetValueResponse response;
        auto status = stub->GetValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
        EXPECT_THAT(response.has_value(), IsFalse());
    }
}

TEST_F(ADevStub, DoesNotGetValueWhenKeyUnknown)
{
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        UpdateValueRequest request;
        auto v = new Value();
        v->set_value(kValue);
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        v->set_allocated_key(k);
        request.set_allocated_value(v);
        UpdateValueResponse response;
        auto status = stub->UpdateValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
    }
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        GetValueRequest request;
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kUnknown);
        k->set_scope(kScope);
        request.set_allocated_key(k);
        GetValueResponse response;
        auto status = stub->GetValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsFalse());
        EXPECT_THAT(status.error_code(), Eq(5));
        EXPECT_THAT(status.error_message(), Eq("requested key scope does not exist"));
    }
}

TEST_F(AStub, DeletesValueWhenAppIdUnknown)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    DeleteValueRequest request;
    auto k = new Key();
    k->set_app_id(kUnknown);
    k->set_key(kKey);
    k->set_scope(kScope);
    request.set_allocated_key(k);
    DeleteValueResponse response;
    auto status = stub->DeleteValue(&context, request, &response);
    EXPECT_THAT(status.ok(), IsTrue());
}

TEST_F(AStub, DeletesValueWhenKeyUnknown)
{
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        UpdateValueRequest request;
        auto v = new Value();
        v->set_value(kValue);
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        v->set_allocated_key(k);
        request.set_allocated_value(v);
        UpdateValueResponse response;
        auto status = stub->UpdateValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
    }
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        DeleteValueRequest request;
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kUnknown);
        k->set_scope(kScope);
        request.set_allocated_key(k);
        DeleteValueResponse response;
        auto status = stub->DeleteValue(&context, request, &response);
        EXPECT_THAT(status.ok(), IsTrue());
    }
}

TEST_F(AStub, DeletesAllValuesWhenAppIdUnknown)
{
    grpc::ClientContext context;
    context.AddMetadata("authorization", std::string(kToken));
    DeleteAllValuesRequest request;
    request.set_app_id(kUnknown);
    request.set_scope(kScope);
    DeleteAllValuesResponse response;
    auto status = stub->DeleteAllValues(&context, request, &response);
    EXPECT_THAT(status.ok(), IsTrue());
}

TEST_F(AStub, GetsValueWhenTtlDidNotExpire)
{
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        UpdateValueRequest request;
        auto v = new Value();
        v->set_value(kValue);
        auto t = new ::google::protobuf::Duration();
        t->set_seconds(kTtl);
        v->set_allocated_ttl(t);
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        v->set_allocated_key(k);
        request.set_allocated_value(v);
        UpdateValueResponse response;
        auto status = stub->UpdateValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
    }
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        GetValueRequest request;
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        request.set_allocated_key(k);
        GetValueResponse response;
        auto status = stub->GetValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
        ASSERT_THAT(response.has_value(), IsTrue());
        EXPECT_THAT(response.value().value(), Eq(kValue));
        EXPECT_THAT(response.value().has_ttl(), IsFalse());
        ASSERT_THAT(response.value().has_expire_time(), IsTrue());
        ::google::protobuf::Timestamp now;
        now.set_seconds(time(nullptr));
        now.set_nanos(0);
        EXPECT_THAT(response.value().expire_time().seconds(), Gt(now.seconds()));
        EXPECT_THAT(response.value().expire_time().seconds(), Le(now.seconds() + kTtl + kTtlFault));
    }
}

TEST_F(AStub, DoesNotGetValueWhenTtlExpired)
{
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        UpdateValueRequest request;
        auto v = new Value();
        v->set_value(kValue);
        auto t = new ::google::protobuf::Duration();
        t->set_seconds(kTtl);
        v->set_allocated_ttl(t);
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        v->set_allocated_key(k);
        request.set_allocated_value(v);
        UpdateValueResponse response;
        auto status = stub->UpdateValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
    }
    sleep(kTtl + 1);
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        GetValueRequest request;
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        request.set_allocated_key(k);
        GetValueResponse response;
        auto status = stub->GetValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
        EXPECT_THAT(response.has_value(), IsFalse());
    }
}

TEST_F(ADevStub, DoesNotGetValueWhenTtlExpired)
{
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        UpdateValueRequest request;
        auto v = new Value();
        v->set_value(kValue);
        auto t = new ::google::protobuf::Duration();
        t->set_seconds(kTtl);
        v->set_allocated_ttl(t);
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        v->set_allocated_key(k);
        request.set_allocated_value(v);
        UpdateValueResponse response;
        auto status = stub->UpdateValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsTrue());
    }
    sleep(kTtl + kTtlFault);
    {
        grpc::ClientContext context;
        context.AddMetadata("authorization", std::string(kToken));
        GetValueRequest request;
        auto k = new Key();
        k->set_app_id(kAppId);
        k->set_key(kKey);
        k->set_scope(kScope);
        request.set_allocated_key(k);
        GetValueResponse response;
        auto status = stub->GetValue(&context, request, &response);
        ASSERT_THAT(status.ok(), IsFalse());
        EXPECT_THAT(status.error_code(), Eq(5));
        EXPECT_THAT(status.error_message(), Eq("requested key scope does not exist"));
    }
}
