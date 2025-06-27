/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Metrological
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

#include <cryptography/cryptography.h>
#include <interfaces/IConfiguration.h>

namespace WPEFramework {

namespace Plugin {
    class CryptographyImplementation : virtual public Exchange::IConfiguration {
    private:
        class ExternalAccess : public RPC::Communicator {
        public:
            ExternalAccess() = delete;
            ExternalAccess(const ExternalAccess&) = delete;
            ExternalAccess& operator=(const ExternalAccess&) = delete;

            ExternalAccess(
                const Core::NodeId& source,
                CryptographyImplementation* parent,
                const string& proxyStubPath,
                const Core::ProxyType<RPC::InvokeServer>& engine)
                : RPC::Communicator(source, proxyStubPath, Core::ProxyType<Core::IIPCServer>(engine))
                , _parent(*parent)
            {
                Open(Core::infinite);
            }
            ~ExternalAccess()
            {
                Close(Core::infinite);
            }

        private:
            void* Acquire(const string& className, const uint32_t interfaceId, const uint32_t versionId) override
            {
                void* result = nullptr;

                if ((versionId == 1) || (versionId == static_cast<uint32_t>(~0))) {
                    result = _parent.QueryInterface(interfaceId);
                    TRACE(Trace::Information, ("Acquired interface(0x%08X) => %p", interfaceId, result));
                }

                return (result);
            }

        private:
            CryptographyImplementation& _parent;
        };
        class Config : public Core::JSON::Container {
        public:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

            Config()
                : Core::JSON::Container()
                , Connector(_T("/tmp/icryptography"))
            {
                Add(_T("connector"), &Connector);
            }
            ~Config() override = default;

        public:
            Core::JSON::String Connector;
        };

    public:
        CryptographyImplementation(const CryptographyImplementation&) = delete;
        CryptographyImplementation& operator=(const CryptographyImplementation&) = delete;

        CryptographyImplementation()
            : _cryptography(nullptr)
            , _rpcLink(nullptr)
        {
            TRACE(Trace::Information, (_T("Constructing CryptographyImplementation Service: %p"), this));
        }
        ~CryptographyImplementation() override
        {
            if (_rpcLink != nullptr) {
                delete _rpcLink;
            }

            if (_cryptography != nullptr) {
                _cryptography->Release();
            }

            TRACE(Trace::Information, (_T("Destructed CryptographyImplementation Service: %p"), this));
        }

        /*********************************************************************************************
         * Exchange::IConfiguration
         *********************************************************************************************/
        uint32_t Configure(PluginHost::IShell* framework) override
        {
            uint32_t result(Core::ERROR_NONE);
            Config config;

            config.FromString(framework->ConfigLine());

            _cryptography = Exchange::ICryptography::Instance("");

            Core::ProxyType<RPC::InvokeServer> server = Core::ProxyType<RPC::InvokeServer>::Create(&Core::IWorkerPool::Instance());

            _rpcLink = new ExternalAccess(Core::NodeId(config.Connector.Value().c_str()), this, _T(""), server);

            if (_rpcLink != nullptr) {
                if (_rpcLink->IsListening() == false) {
                    delete _rpcLink;
                    _rpcLink = nullptr;

                    if (_cryptography != nullptr) {
                        _cryptography->Release();
                        _cryptography = nullptr;
                    }
                }
            }

            return result;
        }

        BEGIN_INTERFACE_MAP(CryptographyImplementation)
            INTERFACE_ENTRY(Exchange::IConfiguration)
            INTERFACE_AGGREGATE(Exchange::ICryptography, _cryptography)
        END_INTERFACE_MAP

    private:
        Exchange::ICryptography* _cryptography;
        ExternalAccess* _rpcLink;
    };

    SERVICE_REGISTRATION(CryptographyImplementation, 1, 0);
}
} /* namespace WPEFramework::Plugin */
