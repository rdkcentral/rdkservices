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

#include <interfaces/IStore2.h>
#include <sqlite3.h>

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        template <typename ACTUALHANDLE>
        class StoreInspectorType : public Exchange::IStoreInspector, protected ACTUALHANDLE {
        private:
            StoreInspectorType(const StoreInspectorType<ACTUALHANDLE>&) = delete;
            StoreInspectorType<ACTUALHANDLE>& operator=(const StoreInspectorType<ACTUALHANDLE>&) = delete;

            typedef RPC::IteratorType<INamespaceSizeIterator> NamespaceSizeIterator;

        public:
            template <typename... Args>
            StoreInspectorType(Args&&... args)
                : Exchange::IStoreInspector()
                , ACTUALHANDLE(std::forward<Args>(args)...)
            {
            }
            ~StoreInspectorType() override = default;

        public:
            // IStoreInspector methods

            uint32_t GetKeys(const ScopeType scope, const string& ns, RPC::IStringIterator*& keys) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(*this, "SELECT key"
                                          " FROM item"
                                          " where ns in (select id from namespace where name = ?)"
                                          ";",
                    -1, &stmt, NULL);

                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

                std::list<string> list;
                do {
                    auto rc = sqlite3_step(stmt);
                    if (rc == SQLITE_ROW) {
                        list.emplace_back((const char*)sqlite3_column_text(stmt, 0));
                    } else {
                        if (rc == SQLITE_DONE) {
                            keys = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
                            result = Core::ERROR_NONE;
                        } else {
                            TRACE(Trace::Error, (_T("Sqlite select keys error %d"), rc));
                            result = Core::ERROR_GENERAL;
                        }
                        break;
                    }
                } while (true);

                sqlite3_finalize(stmt);

                return result;
            }
            uint32_t GetNamespaces(const ScopeType scope, RPC::IStringIterator*& namespaces) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(*this, "SELECT name FROM namespace;", -1, &stmt, NULL);

                std::list<string> list;
                do {
                    auto rc = sqlite3_step(stmt);
                    if (rc == SQLITE_ROW) {
                        list.emplace_back((const char*)sqlite3_column_text(stmt, 0));
                    } else {
                        if (rc == SQLITE_DONE) {
                            namespaces = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
                            result = Core::ERROR_NONE;
                        } else {
                            TRACE(Trace::Error, (_T("Sqlite select namespaces error %d"), rc));
                            result = Core::ERROR_GENERAL;
                        }
                        break;
                    }
                } while (true);

                sqlite3_finalize(stmt);

                return result;
            }
            uint32_t GetStorageSizes(const ScopeType scope, INamespaceSizeIterator*& storageList) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(*this, "SELECT name, sum(length(key)+length(value))"
                                          " FROM item"
                                          " INNER JOIN namespace ON namespace.id = item.ns"
                                          " GROUP BY name"
                                          ";",
                    -1, &stmt, NULL);

                std::list<NamespaceSize> list;
                do {
                    auto rc = sqlite3_step(stmt);
                    if (rc == SQLITE_ROW) {
                        NamespaceSize namespaceSize;
                        namespaceSize.ns = (const char*)sqlite3_column_text(stmt, 0);
                        namespaceSize.size = sqlite3_column_int(stmt, 1);
                        list.emplace_back(namespaceSize);
                    } else {
                        if (rc == SQLITE_DONE) {
                            storageList = (Core::Service<NamespaceSizeIterator>::Create<INamespaceSizeIterator>(list));
                            result = Core::ERROR_NONE;
                        } else {
                            TRACE(Trace::Error, (_T("Sqlite select sizes error %d"), rc));
                            result = Core::ERROR_GENERAL;
                        }
                        break;
                    }
                } while (true);

                sqlite3_finalize(stmt);

                return result;
            }

            BEGIN_INTERFACE_MAP(StoreInspectorType)
            INTERFACE_ENTRY(Exchange::IStoreInspector)
            END_INTERFACE_MAP
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
