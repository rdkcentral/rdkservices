#include "MixedContentWhiteListedOriginDomainsList.h"

using std::unique_ptr;
using std::vector;

namespace WPEFramework {
namespace WebKit {

    MixedContentWhiteListedOriginDomainsList::MixedContentWhiteListedOriginDomainsList(const char* jsonString)
    {
        Core::JSON::ArrayType<JSONEntry> entries;
        entries.FromString(jsonString);
        Core::JSON::ArrayType<JSONEntry>::Iterator originIndex(entries.Elements());

        while (originIndex.Next() == true) {
            if ((originIndex.Current().Origin.IsSet() == true) && (originIndex.Current().Domain.IsSet() == true)) {
                MixedContentWhiteListedOriginDomainsList::Domains& domains(_whiteMap[originIndex.Current().Origin.Value()]);

                Core::JSON::ArrayType<Core::JSON::String>::Iterator domainIndex(originIndex.Current().Domain.Elements());

                while (domainIndex.Next()) {
                    domains.emplace_back(domainIndex.Current().Value());
                }
            }
        }
    }

    // Adds stored entries to WebKit.
    void MixedContentWhiteListedOriginDomainsList::AddToWebKit(WebKitWebExtension* extension)
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
