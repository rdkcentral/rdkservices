/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "../Module.h"
#include "secure_storage.grpc.pb.h"
#include <fstream>
#include <grpcpp/create_channel.h>
#include <interfaces/IStore2.h>
#ifdef WITH_SYSMGR
#include <libIBus.h>
#include <sysMgr.h>
#endif

namespace WPEFramework {
namespace Plugin {
    namespace Grpc {

        class Store2 : public Exchange::IStore2 {
        private:
            class Job : public Core::IDispatch {
            public:
                Job(Store2* parent, const ScopeType scope, const string& ns, const string& key, const string& value)
                    : _parent(parent)
                    , _scope(scope)
                    , _ns(ns)
                    , _key(key)
                    , _value(value)
                {
                    _parent->AddRef();
                }
                ~Job() override
                {
                    _parent->Release();
                }
                void Dispatch() override
                {
                    _parent->OnValueChanged(_scope, _ns, _key, _value);
                }

            private:
                Store2* _parent;
                const ScopeType _scope;
                const string _ns;
                const string _key;
                const string _value;
            };

        private:
            Store2(const Store2&) = delete;
            Store2& operator=(const Store2&) = delete;

        public:
            Store2()
                : Store2(getenv(URI_ENV))
            {
            }
            Store2(const string& uri)
                : IStore2()
                , _uri(uri)
                , _authorization((_uri.find("localhost") == string::npos) && (_uri.find("0.0.0.0") == string::npos))
            {
                Open();
            }
            ~Store2() override = default;

        private:
            void Open()
            {
                grpc::ChannelArguments args;
                args.SetInt(GRPC_ARG_CLIENT_IDLE_TIMEOUT_MS, IDLE_TIMEOUT);
                std::shared_ptr<grpc::ChannelCredentials> creds;
                if (_authorization) {
                    creds = grpc::SslCredentials(grpc::SslCredentialsOptions());
                } else {
                    creds = grpc::InsecureChannelCredentials();
                }
                _stub = ::distp::gateway::secure_storage::v1::SecureStorageService::NewStub(
                    grpc::CreateCustomChannel(_uri, creds, args));
            }

        private:
            bool IsTimeSynced() const
            {
#ifdef WITH_SYSMGR
                // Get actual state, as it may change at any time...
                IARM_Bus_Init(IARM_INIT_NAME);
                IARM_Bus_Connect();
                IARM_Bus_SYSMgr_GetSystemStates_Param_t param;
                if ((IARM_Bus_Call_with_IPCTimeout(
                         IARM_BUS_SYSMGR_NAME,
                         IARM_BUS_SYSMGR_API_GetSystemStates,
                         &param,
                         sizeof(param),
                         IARM_TIMEOUT) // Timeout
                        != IARM_RESULT_SUCCESS)
                    || !param.time_source.state) {
                    return false;
                }
#endif
                return true;
            }
            string GetSecurityToken() const
            {
                // Get actual token, as it may change at any time...
                string result;

                const char* endpoint = ::getenv(_T("SECURITYAGENT_PATH"));
                if (endpoint == nullptr) {
                    endpoint = SECURITY_AGENT_FILENAME;
                }
                auto engine = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
                auto client = Core::ProxyType<RPC::CommunicatorClient>::Create(
                    Core::NodeId(endpoint),
                    Core::ProxyType<Core::IIPCServer>(engine));

                auto interface = client->Open<PluginHost::IAuthenticate>(
                    _T("SecurityAgent"),
                    static_cast<uint32_t>(~0),
                    COM_RPC_TIMEOUT); // Timeout
                if (interface != nullptr) {
                    string payload = _T("http://localhost");
                    // If main process is out of threads, this can time out, and IPC will mess up...
                    auto error = interface->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t*>(payload.c_str()),
                        result);
                    if (error != Core::ERROR_NONE) {
                        TRACE(Trace::Error, (_T("security token error %d"), error));
                    }
                    interface->Release();
                }

                return result;
            }
            string GetToken() const
            {
                // Get actual token, as it may change at any time...
                string result;

                Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
                auto link = Core::ProxyType<JSONRPC::LinkType<Core::JSON::IElement>>::Create(
                    _T("org.rdk.AuthService"), _T(""), false, "token=" + GetSecurityToken());

                JsonObject json;
                auto status = link->Invoke<JsonObject, JsonObject>(
                    JSON_RPC_TIMEOUT, // Timeout
                    _T("getServiceAccessToken"),
                    JsonObject(),
                    json);
                if (status == Core::ERROR_NONE) {
                    result = json[_T("token")].String();
                } else {
                    TRACE(Trace::Error, (_T("sat status %d"), status));
                }

                return result;
            }
            string GetPartnerId() const
            {
                // Get actual id, as it may change at any time...
                std::ifstream input(PARTNER_ID_FILENAME);
                string line;
                getline(input, line);
                return line;
            }
            string GetAccountId() const
            {
                // Get actual id, as it may change at any time...
                std::ifstream input(ACCOUNT_ID_FILENAME);
                string line;
                getline(input, line);
                return line;
            }
            string GetDeviceId() const
            {
                // Get actual id, as it may change at any time...
                std::ifstream input(DEVICE_ID_FILENAME);
                string line;
                getline(input, line);
                return line;
            }

        public:
            uint32_t Register(INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

                notification->AddRef();
                _clients.push_back(notification);

                return Core::ERROR_NONE;
            }
            uint32_t Unregister(INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                std::list<INotification*>::iterator
                    index(std::find(_clients.begin(), _clients.end(), notification));

                ASSERT(index != _clients.end());

                if (index != _clients.end()) {
                    notification->Release();
                    _clients.erase(index);
                }

                return Core::ERROR_NONE;
            }

            uint32_t SetValue(const ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) override
            {
                uint32_t result;

                grpc::ClientContext context;
                if (_authorization) {
                    context.AddMetadata("authorization", "Bearer " + GetToken());
                }
                context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(GRPC_TIMEOUT)); // Timeout
                ::distp::gateway::secure_storage::v1::UpdateValueRequest request;
                request.set_partner_id(GetPartnerId());
                request.set_account_id(GetAccountId());
                request.set_device_id(GetDeviceId());
                auto v = new ::distp::gateway::secure_storage::v1::Value();
                v->set_value(value);
                if (ttl != 0) {
                    auto t = new google::protobuf::Duration();
                    t->set_seconds(ttl);
                    v->set_allocated_ttl(t);
                }
                auto k = new ::distp::gateway::secure_storage::v1::Key();
                k->set_app_id(ns);
                k->set_key(key);
                k->set_scope(scope == ScopeType::ACCOUNT
                        ? ::distp::gateway::secure_storage::v1::Scope::SCOPE_ACCOUNT
                        : (scope == ScopeType::DEVICE
                                  ? ::distp::gateway::secure_storage::v1::Scope::SCOPE_DEVICE
                                  : ::distp::gateway::secure_storage::v1::Scope::SCOPE_UNSPECIFIED));
                v->set_allocated_key(k);
                request.set_allocated_value(v);
                ::distp::gateway::secure_storage::v1::UpdateValueResponse response;
                auto status = _stub->UpdateValue(&context, request, &response);

                if (status.ok()) {
                    Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(
                        Core::ProxyType<Job>::Create(this, scope, ns, key, value))); // Decouple notification

                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, status);
                    if (status.error_code() == grpc::StatusCode::INVALID_ARGUMENT) {
                        result = Core::ERROR_INVALID_INPUT_LENGTH;
                    } else {
                        result = Core::ERROR_GENERAL;
                    }
                }

                return result;
            }
            uint32_t GetValue(const ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) override
            {
                uint32_t result;

                grpc::ClientContext context;
                if (_authorization) {
                    context.AddMetadata("authorization", "Bearer " + GetToken());
                }
                context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(GRPC_TIMEOUT)); // Timeout
                ::distp::gateway::secure_storage::v1::GetValueRequest request;
                request.set_partner_id(GetPartnerId());
                request.set_account_id(GetAccountId());
                request.set_device_id(GetDeviceId());
                auto k = new ::distp::gateway::secure_storage::v1::Key();
                k->set_app_id(ns);
                k->set_key(key);
                k->set_scope(scope == ScopeType::ACCOUNT
                        ? ::distp::gateway::secure_storage::v1::Scope::SCOPE_ACCOUNT
                        : (scope == ScopeType::DEVICE
                                  ? ::distp::gateway::secure_storage::v1::Scope::SCOPE_DEVICE
                                  : ::distp::gateway::secure_storage::v1::Scope::SCOPE_UNSPECIFIED));
                request.set_allocated_key(k);
                ::distp::gateway::secure_storage::v1::GetValueResponse response;
                auto status = _stub->GetValue(&context, request, &response);

                if (status.ok()) {
                    if (response.has_value()) {
                        auto v = response.value();
                        if (v.has_ttl()) {
                            ttl = v.ttl().seconds();
                            value = v.value();
                            result = Core::ERROR_NONE;
                        } else if (v.has_expire_time() && (v.expire_time().seconds() != 0)) {
                            if (IsTimeSynced()) {
                                ttl = v.expire_time().seconds() - time(nullptr);
                                value = v.value();
                                result = Core::ERROR_NONE;
                            } else {
                                result = Core::ERROR_PENDING_CONDITIONS;
                            }
                        } else {
                            ttl = 0;
                            value = v.value();
                            result = Core::ERROR_NONE;
                        }
                    } else {
                        result = Core::ERROR_UNKNOWN_KEY;
                    }
                } else {
                    OnError(__FUNCTION__, status);
                    if (status.error_code() == grpc::StatusCode::INVALID_ARGUMENT) {
                        result = Core::ERROR_INVALID_INPUT_LENGTH;
                    } else if (status.error_code() == grpc::StatusCode::NOT_FOUND) {
                        result = Core::ERROR_UNKNOWN_KEY;
                    } else {
                        result = Core::ERROR_GENERAL;
                    }
                }

                return result;
            }
            uint32_t DeleteKey(const ScopeType scope, const string& ns, const string& key) override
            {
                uint32_t result;

                grpc::ClientContext context;
                if (_authorization) {
                    context.AddMetadata("authorization", "Bearer " + GetToken());
                }
                context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(GRPC_TIMEOUT)); // Timeout
                ::distp::gateway::secure_storage::v1::DeleteValueRequest request;
                request.set_partner_id(GetPartnerId());
                request.set_account_id(GetAccountId());
                request.set_device_id(GetDeviceId());
                auto k = new ::distp::gateway::secure_storage::v1::Key();
                k->set_app_id(ns);
                k->set_key(key);
                k->set_scope(scope == ScopeType::ACCOUNT
                        ? ::distp::gateway::secure_storage::v1::Scope::SCOPE_ACCOUNT
                        : (scope == ScopeType::DEVICE
                                  ? ::distp::gateway::secure_storage::v1::Scope::SCOPE_DEVICE
                                  : ::distp::gateway::secure_storage::v1::Scope::SCOPE_UNSPECIFIED));
                request.set_allocated_key(k);
                ::distp::gateway::secure_storage::v1::DeleteValueResponse response;
                auto status = _stub->DeleteValue(&context, request, &response);

                if (status.ok()) {
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, status);
                    if (status.error_code() == grpc::StatusCode::INVALID_ARGUMENT) {
                        result = Core::ERROR_INVALID_INPUT_LENGTH;
                    } else {
                        result = Core::ERROR_GENERAL;
                    }
                }

                return result;
            }
            uint32_t DeleteNamespace(const ScopeType scope, const string& ns) override
            {
                uint32_t result;

                grpc::ClientContext context;
                if (_authorization) {
                    context.AddMetadata("authorization", "Bearer " + GetToken());
                }
                context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(GRPC_TIMEOUT)); // Timeout
                ::distp::gateway::secure_storage::v1::DeleteAllValuesRequest request;
                request.set_partner_id(GetPartnerId());
                request.set_account_id(GetAccountId());
                request.set_device_id(GetDeviceId());
                request.set_app_id(ns);
                request.set_scope(scope == ScopeType::ACCOUNT
                        ? ::distp::gateway::secure_storage::v1::Scope::SCOPE_ACCOUNT
                        : (scope == ScopeType::DEVICE
                                  ? ::distp::gateway::secure_storage::v1::Scope::SCOPE_DEVICE
                                  : ::distp::gateway::secure_storage::v1::Scope::SCOPE_UNSPECIFIED));
                ::distp::gateway::secure_storage::v1::DeleteAllValuesResponse response;
                auto status = _stub->DeleteAllValues(&context, request, &response);

                if (status.ok()) {
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, status);
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }

            BEGIN_INTERFACE_MAP(Store2)
            INTERFACE_ENTRY(IStore2)
            END_INTERFACE_MAP

        private:
            void OnValueChanged(const ScopeType scope, const string& ns, const string& key, const string& value)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                std::list<INotification*>::iterator
                    index(_clients.begin());

                while (index != _clients.end()) {
                    // If main process is out of threads, this can time out, and IPC will mess up...
                    (*index)->ValueChanged(scope, ns, key, value);
                    index++;
                }
            }
            void OnError(const char* fn, const grpc::Status& status) const
            {
                TRACE(Trace::Error, (_T("%s grpc error %d %s %s"), fn, status.error_code(), status.error_message().c_str(), status.error_details().c_str()));
            }

        private:
            const string _uri;
            const bool _authorization;
            std::unique_ptr<::distp::gateway::secure_storage::v1::SecureStorageService::Stub> _stub;
            std::list<INotification*> _clients;
            Core::CriticalSection _clientLock;
        };

    } // namespace Grpc
} // namespace Plugin
} // namespace WPEFramework
