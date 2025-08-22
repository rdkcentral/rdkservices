/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

#include "Module.h"
#include "RialtoServerManagerConfig.h"
#include <interfaces/IConfiguration.h>

#include <rialto/IServerManagerService.h>
#include <rialto/ServerManagerServiceFactory.h>

#include <condition_variable>
#include <mutex>

namespace {
    static const string s_defaultAppName = "ExampleApp";
    static const unsigned s_defaultRialtoServerStartupTimeoutMs = 5000; // 5s
}

namespace WPEFramework {
namespace Plugin {
    class RialtoServerManagerImpl : public Exchange::IConfiguration {
    private:
        RialtoServerManagerImpl(const RialtoServerManagerImpl&) = delete;
        RialtoServerManagerImpl& operator=(const RialtoServerManagerImpl&) = delete;

    public:
        class RialtoServerManagerStateObserver : public rialto::servermanager::service::IStateObserver {
        public:
            RialtoServerManagerStateObserver(RialtoServerManagerImpl& parent) : _parent(parent) {}
            virtual ~RialtoServerManagerStateObserver() = default;

            void stateChanged(const std::string &appId, const firebolt::rialto::common::SessionServerState &state) override {
                _parent.stateChanged(appId, state);
            }
        private:
            RialtoServerManagerImpl& _parent;
        };

        RialtoServerManagerImpl() {}
        virtual ~RialtoServerManagerImpl() {
            TRACE(Trace::Information, (_T("[RialtoServerManagerImpl] destroyed")));
        }

        uint32_t Configure(PluginHost::IShell* service) {
            if (_config.FromString(service->ConfigLine()) == false) {
                TRACE(Trace::Error, (_T("[RialtoServerManagerImpl] Failed to parse config line")));
                return 1;
            }

            _stateObserver = std::make_shared<RialtoServerManagerStateObserver>(*this);
            _serverManagerService = rialto::servermanager::service::create(_stateObserver, getRialtoServerConfig());
            if (!_serverManagerService) {
                TRACE(Trace::Error, (_T("[RialtoServerManagerImpl] Failed to create ServerManagerService")));
                return 1;
            }

            const string appName = s_defaultAppName;
            if (!_serverManagerService->initiateApplication(appName, firebolt::rialto::common::SessionServerState::ACTIVE, {})) {
                TRACE(Trace::Error, (_T("[RialtoServerManagerImpl] Failed to initiate application")));
                return 1;
            }

            if (!waitState(appName, firebolt::rialto::common::SessionServerState::ACTIVE, s_defaultRialtoServerStartupTimeoutMs)) {
                TRACE(Trace::Error, (_T("[RialtoServerManagerImpl] Failed to wait for app state")));
                return 1;
            }

            auto socket = _serverManagerService->getAppConnectionInfo(appName);
            TRACE(Trace::Information, (_T("[RialtoServerManagerImpl] initialized, app connection info: %s"), socket.c_str()));

            return 0;
        }
    private:
        void stateChanged(const std::string &appId, const firebolt::rialto::common::SessionServerState &state) {
            TRACE(Trace::Information, (_T("[RialtoServerManagerImpl] state changed for app %s: %d"), appId.c_str(), state));
            std::lock_guard<std::mutex> lock(_serverMutex);
            _appStates[appId] = state;
            _serverCv.notify_all();
        }

        bool waitState(const std::string &appId, const firebolt::rialto::common::SessionServerState &state, unsigned timeoutMs = -1) {
            std::unique_lock<std::mutex> lock(_serverMutex);
            auto isAppStateReached = [&] { return _appStates.find(appId) != _appStates.end() && _appStates[appId] == state; };
            if (isAppStateReached()) {
                return true;
            }

            if (timeoutMs < 0) {
                _serverCv.wait(lock, isAppStateReached);
                return true;
            }
            return _serverCv.wait_for(lock, std::chrono::milliseconds(timeoutMs), isAppStateReached);
        }

        firebolt::rialto::common::ServerManagerConfig getRialtoServerConfig() {
            firebolt::rialto::common::ServerManagerConfig config;
            config.sessionServerEnvVars = {};
            auto sessionEnvs = RialtoServerManagerConfig::parseEnvs(_config.SessionEnvs);
            for (const auto& env : sessionEnvs) {
                config.sessionServerEnvVars.push_back(env.first + "=" + env.second);
            }

            auto platformEnvs = RialtoServerManagerConfig::parseEnvs(_config.SessionPlatformEnvs);
            if (platformEnvs.empty()) {
                // Pass everything from current environment
                for (char **env = environ; *env != 0; env++) {
                    config.sessionServerEnvVars.push_back(*env);
                }
            } else {
                for (const auto& env : platformEnvs) {
                    config.sessionServerEnvVars.push_back(env.first + "=" + env.second);
                }
            }
            return config;
        }

    private:
        std::unique_ptr<rialto::servermanager::service::IServerManagerService> _serverManagerService;
        std::shared_ptr<RialtoServerManagerStateObserver> _stateObserver;
        RialtoServerManagerConfig::Config _config;

        std::map<std::string, firebolt::rialto::common::SessionServerState> _appStates;
        std::mutex _serverMutex;
        std::condition_variable _serverCv;

    public:
        BEGIN_INTERFACE_MAP(RialtoServerManagerImpl)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP
    };

    SERVICE_REGISTRATION(RialtoServerManagerImpl, 1, 0);
}
} /* namespace WPEFramework::Plugin */
