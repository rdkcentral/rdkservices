#ifndef __MIXEDCONTENTWHITELISTEDORIGINDOMAINSLIST_H
#define __MIXEDCONTENTWHITELISTEDORIGINDOMAINSLIST_H

#include "Module.h"

#include <wpe/webkit-web-extension.h>
#include <vector>
#include <map>
#include <memory>

namespace WPEFramework {
namespace WebKit {

    class MixedContentWhiteListedOriginDomainsList {
    public:
        static std::unique_ptr<MixedContentWhiteListedOriginDomainsList> Parse(const char* jsonString);

        ~MixedContentWhiteListedOriginDomainsList()
        {
        }

        void AddMixedContentWhitelistToWebKit(WebKitWebExtension* extension);

    private:
        typedef std::vector<std::string> Domains;

        MixedContentWhiteListedOriginDomainsList(const MixedContentWhiteListedOriginDomainsList&) = delete;
        MixedContentWhiteListedOriginDomainsList& operator=(const MixedContentWhiteListedOriginDomainsList&) = delete;
        
        MixedContentWhiteListedOriginDomainsList()
        {
        }

        std::map<std::string, Domains> _whiteMap;
    };
}
}

#endif // __MIXEDCONTENTWHITELISTEDORIGINDOMAINSLIST_H
