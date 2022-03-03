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

#ifndef RDKSERVICES_SQLITESTORE_H
#define RDKSERVICES_SQLITESTORE_H

#include "Module.h"

#include "CountingLock.h"

#include "IStoreListing.h"

#include <interfaces/IStore.h>
#include <interfaces/IStoreCache.h>

namespace WPEFramework {
namespace Plugin {

class SqliteStore
        : public Exchange::IStore
        , public Exchange::IStoreCache
        , public IStoreListing
{
private:
    SqliteStore(const SqliteStore &) = delete;
    SqliteStore &operator=(const SqliteStore &) = delete;

public:
    SqliteStore();
    virtual ~SqliteStore() = default;

    uint32_t Open(const string &path, const string &key, uint32_t maxSize, uint32_t maxValue);
    uint32_t Term();

public:
    // IStore methods

    virtual uint32_t Register(Exchange::IStore::INotification *notification) override;
    virtual uint32_t Unregister(Exchange::IStore::INotification *notification) override;
    virtual uint32_t SetValue(const string &ns, const string &key, const string &value) override;
    virtual uint32_t GetValue(const string &ns, const string &key, string &value) override;
    virtual uint32_t DeleteKey(const string &ns, const string &key) override;
    virtual uint32_t DeleteNamespace(const string &ns) override;

    // IStoreListing methods

    virtual uint32_t GetKeys(const string &ns, std::vector<string> &keys) override;
    virtual uint32_t GetNamespaces(std::vector<string> &namespaces) override;
    virtual uint32_t GetStorageSize(std::map<string, uint64_t> &namespaceSizes) override;

    // IStoreCache methods

    virtual uint32_t FlushCache() override;

    BEGIN_INTERFACE_MAP(SqliteStore)
    INTERFACE_ENTRY(Exchange::IStore)
    INTERFACE_ENTRY(Exchange::IStoreCache)
    END_INTERFACE_MAP

protected:
    virtual void ValueChanged(const string &ns, const string &key, const string &value);
    virtual void StorageExceeded();

private:
    int Encrypt(const std::vector<uint8_t> &key);
    int CreateTables();
    int Vacuum();
    int Close();

private:
    bool IsOpen() const;
    bool IsValid() const;
    bool IsEncrypted() const;

private:
    void *_data;
    string _path;
    string _key;
    uint32_t _maxSize;
    uint32_t _maxValue;
    std::list<Exchange::IStore::INotification *> _clients;
    Core::CriticalSection _clientLock;
    CountingLock _lock;
};

} // namespace Plugin
} // namespace WPEFramework

#endif //RDKSERVICES_SQLITESTORE_H
