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

    class CryptographyImplementation : virtual public Exchange::IConfiguration
                                     , virtual public Exchange::IDeviceObjects {
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
                //: RPC::Communicator(source, proxyStubPath, Core::ProxyType<Core::IIPCServer>(engine), _T("@Svalbard"))
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
            void* Acquire(const string& className VARIABLE_IS_NOT_USED, const uint32_t interfaceId, const uint32_t versionId) override
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
                , Connector(_T("/tmp/svalbard"))
                , Location(_T("objects"))
            {
                Add(_T("connector"), &Connector);
                Add(_T("location"), &Location);
            }
            ~Config() override = default;

        public:
            Core::JSON::String Connector;
            Core::JSON::String Location;
        };

    public:
        CryptographyImplementation(const CryptographyImplementation&) = delete;
        CryptographyImplementation& operator=(const CryptographyImplementation&) = delete;

        CryptographyImplementation()
            : _adminLock()
            , _cryptography(nullptr)
            , _rpcLink(nullptr)
            , _objects()
        {
            TRACE(Trace::Information, (_T("Constructing CryptographyImplementation Service: %p"), this));
        }
        ~CryptographyImplementation() override
        {
            _adminLock.Lock();

            if (_rpcLink != nullptr) {
                delete _rpcLink;
            }

            if (_cryptography != nullptr) {
                _cryptography->Release();
            }

            _adminLock.Unlock();

            TRACE(Trace::Information, (_T("Destructed CryptographyImplementation Service: %p"), this));
        }

    public:
        /*********************************************************************************************
         * Exchange::IConfiguration
         *********************************************************************************************/
        uint32_t Configure(PluginHost::IShell* service) override
        {
            uint32_t result(Core::ERROR_NONE);
            Config config;

            ASSERT(service != nullptr);

            config.FromString(service->ConfigLine());

            _adminLock.Lock();

            ASSERT(_cryptography == nullptr);
            ASSERT(_rpcLink == nullptr);

            _cryptography = Exchange::ICryptography::Instance("");
            ASSERT(_cryptography != nullptr);

            Core::ProxyType<RPC::InvokeServer> server = Core::ProxyType<RPC::InvokeServer>::Create(&Core::IWorkerPool::Instance());

            _rpcLink = new ExternalAccess(Core::NodeId(config.Connector.Value().c_str()), this, _T(""), server);
            ASSERT(_rpcLink != nullptr);

            if (_rpcLink->IsListening() == false) {
                delete _rpcLink;
                _rpcLink = nullptr;

                _cryptography->Release();
                _cryptography = nullptr;

                result = Core::ERROR_GENERAL;
            }
            else {
                string persistentPath;
                PluginHost::IShell* provisioning = service->QueryInterfaceByCallsign<PluginHost::IShell>(_T("VaultProvisioning"));

                if (provisioning != nullptr) {
                    persistentPath = provisioning->PersistentPath();
                    ASSERT(persistentPath.empty() == false);
                    provisioning->Release();
                }
                else {
                    persistentPath = service->PersistentPath();
                }

                ASSERT(persistentPath.empty() == false);

                if ((config.Location.Value().empty() == false) && (persistentPath.empty() == false)) {
                    persistentPath += (config.Location.Value() + _T("/"));
                }

                const uint16_t count = ImportObjects(persistentPath);
                TRACE(Trace::Error, (_T("Imported %d sealed object(s) from '%s'"), count, persistentPath.c_str()));
            }

            _adminLock.Unlock();

            return (result);
        }

    public:
        uint32_t Id(const string& label, Exchange::IVault*& outVault) override
        {
            uint32_t result = 0;

            outVault = nullptr;

            _adminLock.Lock();

            ASSERT(_cryptography != nullptr);

            if (_cryptography != nullptr) {
                auto it = _objects.find(label);

                if (it != _objects.end()) {
                    const Exchange::CryptographyVault& vaultId = (*it).second.first;

                    Exchange::IVault* vault = _cryptography->Vault(vaultId);
                    ASSERT(vault != nullptr);

                    if (vault != nullptr) {
                        result = (*it).second.second;
                        ASSERT(result != 0);

                        outVault = vault;
                    };
                }
            }

            _adminLock.Unlock();

            return (result);
        }

    public:
        BEGIN_INTERFACE_MAP(CryptographyImplementation)
            INTERFACE_ENTRY(Exchange::IConfiguration)
            INTERFACE_ENTRY(Exchange::IDeviceObjects)
            INTERFACE_AGGREGATE(Exchange::ICryptography, _cryptography)
        END_INTERFACE_MAP

    private:

        Exchange::CryptographyVault VaultId(std::string vault)
        {
            if (vault == "platform")
                return Exchange::CRYPTOGRAPHY_VAULT_PLATFORM;

            if (vault == "provisioning")
                return Exchange::CRYPTOGRAPHY_VAULT_PROVISIONING;

            if (vault == "netflix")
                return Exchange::CRYPTOGRAPHY_VAULT_NETFLIX;

            return Exchange::CRYPTOGRAPHY_VAULT_DEFAULT;
        }

        uint16_t ImportObjects(const string& path)
        {
            // Preload ICryptography vaults with named device-bound objects (encryption keys or other secret data).
            
            TRACE(Trace::Error, (_T("Importing objects from path '%s'"), path.c_str()));

            uint16_t count = 0;

            ASSERT(_cryptography != nullptr);

            Core::Directory storageDir(path.c_str(), _T("*.json"));

            while (storageDir.Next() == true) {

                int32_t blobId = 0;

                const string fileName = Core::File::FileNameExtended(storageDir.Current());

                TRACE(Trace::Error, (_T("Importing a sealed object from '%s'..."), fileName.c_str()));

                Core::File file(storageDir.Current().c_str());

                if (file.Open(true) == true) {
                    ObjectFile obj;
                    obj.IElement::FromFile(file);
                    
                    const Exchange::CryptographyVault vaultId = VaultId(obj.Vault.Value());

                    if ((vaultId != static_cast<Exchange::CryptographyVault>(~0))
                            && (obj.Data.Value().empty() == false) && (obj.Data.Value().size() <= ((0xFFFF * 4) / 3))) {

                        Exchange::IVault* const vault = _cryptography->Vault(vaultId);

                        if (vault != nullptr) {
                            uint16_t blobLength = 0xFFFF;

                            uint8_t* const blob = static_cast<uint8_t*>(ALLOCA(blobLength));
                            ASSERT(blob != nullptr);

                            if (Core::FromString(obj.Data.Value(), blob, blobLength) == obj.Data.Value().size()) {

                                if (blobLength != 0) {
                                    blobId = vault->Set(blobLength, blob);
                                }
                            }

                            vault->Release();
                        }
                    }

                    if (blobId != 0) {
                        const string label = (obj.Label.Value().empty() == false?
                                                    obj.Label.Value() : Core::File::FileName(storageDir.Current()));

                        _objects.emplace(label, std::make_pair(vaultId, blobId));
                        count++;
                    }
                    else {
                        TRACE(Trace::Error, (_T("Failed to import '%s'!"), fileName.c_str()));
                    }

                    file.Close();
                }
                else {
                    TRACE(Trace::Error, (_T("Failed to open file '%s'!"), fileName.c_str()));
                }
            }

            return (count);
        }

    private:
        class ObjectFile : public Core::JSON::Container {
        public:
            ObjectFile(const ObjectFile&) = delete;
            ObjectFile& operator=(const ObjectFile&) = delete;
            ObjectFile()
                : Core::JSON::Container()
                , Vault()
                , Label()
                , Data()
            {
                Add(_T("vault"), &Vault);
                Add(_T("label"), &Label);
                Add(_T("data"), &Data);

            }
            ~ObjectFile() = default;

        public:
            Core::JSON::String Vault;
            Core::JSON::String Label;
            Core::JSON::String Data;
        };

    private:
        Core::CriticalSection _adminLock;
        Exchange::ICryptography* _cryptography;
        ExternalAccess* _rpcLink;
        std::map<string, std::pair<Exchange::CryptographyVault, uint32_t>> _objects;
    };

    SERVICE_REGISTRATION(CryptographyImplementation, 1, 0)

} // namespace Plugin
}
