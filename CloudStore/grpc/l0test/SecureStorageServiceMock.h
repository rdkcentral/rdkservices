#pragma once

#include "secure_storage.grpc.pb.h"
#include <gmock/gmock.h>

class SecureStorageServiceMock : public ::distp::gateway::secure_storage::v1::SecureStorageService::Service {
public:
    ~SecureStorageServiceMock() override = default;
    MOCK_METHOD(::grpc::Status, GetValue, (::grpc::ServerContext * context, const ::distp::gateway::secure_storage::v1::GetValueRequest* request, ::distp::gateway::secure_storage::v1::GetValueResponse* response), (override));
    MOCK_METHOD(::grpc::Status, UpdateValue, (::grpc::ServerContext * context, const ::distp::gateway::secure_storage::v1::UpdateValueRequest* request, ::distp::gateway::secure_storage::v1::UpdateValueResponse* response), (override));
    MOCK_METHOD(::grpc::Status, DeleteValue, (::grpc::ServerContext * context, const ::distp::gateway::secure_storage::v1::DeleteValueRequest* request, ::distp::gateway::secure_storage::v1::DeleteValueResponse* response), (override));
    MOCK_METHOD(::grpc::Status, DeleteAllValues, (::grpc::ServerContext * context, const ::distp::gateway::secure_storage::v1::DeleteAllValuesRequest* request, ::distp::gateway::secure_storage::v1::DeleteAllValuesResponse* response), (override));
    MOCK_METHOD(::grpc::Status, SeedValue, (::grpc::ServerContext * context, const ::distp::gateway::secure_storage::v1::SeedValueRequest* request, ::distp::gateway::secure_storage::v1::SeedValueResponse* response), (override));
};
