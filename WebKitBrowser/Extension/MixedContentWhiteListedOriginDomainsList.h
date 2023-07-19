#ifndef __MIXEDCONTENTWHITELISTEDORIGINDOMAINSLIST_H
#define __MIXEDCONTENTWHITELISTEDORIGINDOMAINSLIST_H

#include "Module.h"

#include "WhitelistBase.h"

namespace WPEFramework {
namespace WebKit {

    class MixedContentWhiteListedOriginDomainsList final : public WhitelistBase {
    public:
        MixedContentWhiteListedOriginDomainsList(const char* jsonString);
        ~MixedContentWhiteListedOriginDomainsList() {}

        void AddToWebKit(WebKitWebExtension* extension);
    private:
        MixedContentWhiteListedOriginDomainsList(const MixedContentWhiteListedOriginDomainsList&) = delete;
        MixedContentWhiteListedOriginDomainsList& operator=(const MixedContentWhiteListedOriginDomainsList&) = delete;

        typedef std::vector<std::string> Domains;
        std::map<std::string, Domains> _whiteMap;
    };
}
}

#endif // __MIXEDCONTENTWHITELISTEDORIGINDOMAINSLIST_H
