#include "MixedContentWhiteListedOriginDomainsList.h"

using std::unique_ptr;
using std::vector;

namespace WPEFramework {
namespace WebKit {

    /* static */unique_ptr<MixedContentWhiteListedOriginDomainsList> MixedContentWhiteListedOriginDomainsList::Parse(const char* jsonString)
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
            {
                Add(_T("origin"), &Origin);
                Add(_T("domain"), &Domain);
            }
            JSONEntry(const JSONEntry& rhs)
                : Core::JSON::Container()
                , Origin(rhs.Origin)
                , Domain(rhs.Domain)
            {
                Add(_T("origin"), &Origin);
                Add(_T("domain"), &Domain);
            }

        public:
            Core::JSON::String Origin;
            Core::JSON::ArrayType<Core::JSON::String> Domain;
        };

        Core::JSON::ArrayType<JSONEntry> entries;
        entries.FromString(jsonString);
        Core::JSON::ArrayType<JSONEntry>::Iterator originIndex(entries.Elements());
        unique_ptr<MixedContentWhiteListedOriginDomainsList> whiteList(new MixedContentWhiteListedOriginDomainsList());

        while (originIndex.Next() == true) {
            if ((originIndex.Current().Origin.IsSet() == true) && (originIndex.Current().Domain.IsSet() == true)) {
                MixedContentWhiteListedOriginDomainsList::Domains& domains(whiteList->_whiteMap[originIndex.Current().Origin.Value()]);

                Core::JSON::ArrayType<Core::JSON::String>::Iterator domainIndex(originIndex.Current().Domain.Elements());

                while (domainIndex.Next()) {
                    domains.emplace_back(domainIndex.Current().Value());
                }
            }
        }
        return whiteList;
    }

    // Adds stored entries to WebKit.
    void MixedContentWhiteListedOriginDomainsList::AddMixedContentWhitelistToWebKit(WebKitWebExtension* extension)
    {
        auto it = _whiteMap.begin();

        while (it != _whiteMap.end()) {
            for (const std::string& domain : it->second) {
                webkit_web_extension_add_mixed_content_whitelist_entry(extension, it->first.c_str(), domain.c_str());
            }
            it++;
        }
    }
}
}
