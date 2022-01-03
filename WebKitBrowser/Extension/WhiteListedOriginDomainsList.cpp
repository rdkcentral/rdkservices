/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

#include "WhiteListedOriginDomainsList.h"

using std::unique_ptr;
using std::vector;

namespace WPEFramework {
namespace WebKit {

    // Parses JSON containing white listed CORS origin-domain pairs.
    static void ParseWhiteList(const string& jsonString, WhiteListedOriginDomainsList::WhiteMap& info)
    {
        // Origin/Domain pair stored in JSON string.
        class JSONEntry : public Core::JSON::Container {
        private:
            JSONEntry& operator=(const JSONEntry&) = delete;

        public:
            JSONEntry()
                : Core::JSON::Container()
                , Origin()
                , Domain()
                , SubDomain(true)
            {
                Add(_T("origin"), &Origin);
                Add(_T("domain"), &Domain);
                Add(_T("subdomain"), &SubDomain);
            }
            JSONEntry(const JSONEntry& rhs)
                : Core::JSON::Container()
                , Origin(rhs.Origin)
                , Domain(rhs.Domain)
                , SubDomain(rhs.SubDomain)
            {
                Add(_T("origin"), &Origin);
                Add(_T("domain"), &Domain);
                Add(_T("subdomain"), &SubDomain);
            }

        public:
            Core::JSON::String Origin;
            Core::JSON::ArrayType<Core::JSON::String> Domain;
            Core::JSON::Boolean SubDomain;
        };

        Core::JSON::ArrayType<JSONEntry> entries;
        entries.FromString(jsonString);
        Core::JSON::ArrayType<JSONEntry>::Iterator originIndex(entries.Elements());

        while (originIndex.Next() == true) {

            if ((originIndex.Current().Origin.IsSet() == true) && (originIndex.Current().Domain.IsSet() == true)) {

                WhiteListedOriginDomainsList::Domains& domains(info[originIndex.Current().Origin.Value()]);

                Core::JSON::ArrayType<Core::JSON::String>::Iterator domainIndex(originIndex.Current().Domain.Elements());
                bool subDomain(originIndex.Current().SubDomain.Value());

                while (domainIndex.Next()) {
                    domains.emplace_back(subDomain, domainIndex.Current().Value());
                }
            }
        }
    }

    /* static */unique_ptr<WhiteListedOriginDomainsList> WhiteListedOriginDomainsList::Parse(const char* whitelist)
    {
        unique_ptr<WhiteListedOriginDomainsList> whiteList(new WhiteListedOriginDomainsList());
        ParseWhiteList(whitelist, whiteList->_whiteMap);
        return whiteList;
    }

    // Adds stored entries to WebKit.
    void WhiteListedOriginDomainsList::AddWhiteListToWebKit(WebKitWebExtension* extension)
    {
        WhiteMap::const_iterator index(_whiteMap.begin());

        while (index != _whiteMap.end()) {

            WebKitSecurityOrigin* origin = webkit_security_origin_new_for_uri(index->first.c_str());
            for (const Domain& domainIndex : index->second) {
                WebKitSecurityOrigin* domain = webkit_security_origin_new_for_uri(domainIndex.second.c_str());
                webkit_web_extension_add_origin_access_whitelist_entry(extension,
                        origin, webkit_security_origin_get_protocol(domain),
                        webkit_security_origin_get_host(domain), domainIndex.first);
                webkit_security_origin_unref(domain);
            }
            webkit_security_origin_unref(origin);

            index++;
        }
    }
}
}
