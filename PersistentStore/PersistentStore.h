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

#include "Module.h"
#include <interfaces/IStore.h>

namespace WPEFramework {
namespace Plugin {

class PersistentStore: public PluginHost::IPlugin, public PluginHost::JSONRPC
{
private:
    class Config: public Core::JSON::Container
    {
    private:
        Config(const Config &) = delete;
        Config &operator=(const Config &) = delete;

    public:
        Config()
            : Core::JSON::Container(),
              Path(),
              Key(),
              MaxSize(0),
              MaxValue(0)
        {
            Add(_T("path"), &Path);
            Add(_T("key"), &Key);
            Add(_T("maxsize"), &MaxSize);
            Add(_T("maxvalue"), &MaxValue);
        }

    public:
        Core::JSON::String Path;
        Core::JSON::String Key;
        Core::JSON::DecUInt64 MaxSize;
        Core::JSON::DecUInt64 MaxValue;
    };

    class StoreNotification: protected Exchange::IStore::INotification
    {
    private:
        StoreNotification(const StoreNotification &) = delete;
        StoreNotification &operator=(const StoreNotification &) = delete;

    public:
        explicit StoreNotification(PersistentStore *parent)
            : _parent(*parent),
              _client(nullptr)
        {
            ASSERT(parent != nullptr);
        }
        ~StoreNotification() = default;

    public:
        void Initialize(Exchange::IStore *client)
        {
            ASSERT(_client == nullptr);
            ASSERT(client != nullptr);

            _client = client;
            _client->AddRef();
            _client->Register(this);
        }
        void Deinitialize()
        {
            ASSERT(_client != nullptr);

            if (_client != nullptr) {
                _client->Unregister(this);
                _client->Release();
                _client = nullptr;
            }
        }

    public:
        // IStore::INotification methods

        virtual void ValueChanged(const string &ns, const string &key, const string &value) override
        {
            _parent.event_onValueChanged(ns, key, value);
        }
        virtual void StorageExceeded() override
        {
            _parent.event_onStorageExceeded();
        }

        BEGIN_INTERFACE_MAP(StoreNotification)
        INTERFACE_ENTRY(Exchange::IStore::INotification)
        END_INTERFACE_MAP

    private:
        PersistentStore &_parent;
        Exchange::IStore *_client;
    };

private:
    PersistentStore(const PersistentStore &) = delete;
    PersistentStore &operator=(const PersistentStore &) = delete;

public:
    PersistentStore();
    virtual ~PersistentStore();

    // Build QueryInterface implementation, specifying all possible interfaces to be returned.
    BEGIN_INTERFACE_MAP(PersistentStore)
    INTERFACE_ENTRY(PluginHost::IPlugin)
    INTERFACE_ENTRY(PluginHost::IDispatcher)
    INTERFACE_AGGREGATE(Exchange::IStore, _store)
    END_INTERFACE_MAP

public:
    //   IPlugin methods
    // -------------------------------------------------------------------------------------------------------
    virtual const string Initialize(PluginHost::IShell *service) override;
    virtual void Deinitialize(PluginHost::IShell *service) override;
    virtual string Information() const override;

protected:
    void RegisterAll();
    void UnregisterAll();

    uint32_t endpoint_setValue(const JsonObject &parameters, JsonObject &response);
    uint32_t endpoint_getValue(const JsonObject &parameters, JsonObject &response);
    uint32_t endpoint_deleteKey(const JsonObject &parameters, JsonObject &response);
    uint32_t endpoint_deleteNamespace(const JsonObject &parameters, JsonObject &response);
    uint32_t endpoint_getKeys(const JsonObject &parameters, JsonObject &response);
    uint32_t endpoint_getNamespaces(const JsonObject &parameters, JsonObject &response);
    uint32_t endpoint_getStorageSize(const JsonObject &parameters, JsonObject &response);
    uint32_t endpoint_flushCache(const JsonObject &parameters, JsonObject &response);

    virtual void event_onValueChanged(const string &ns, const string &key, const string &value);
    virtual void event_onStorageExceeded();

protected:
    virtual std::vector<string> LegacyLocations() const;

private:
    Config _config;
    Exchange::IStore *_store;
    Core::Sink<StoreNotification> _storeSink;
};

} // namespace Plugin
} // namespace WPEFramework
