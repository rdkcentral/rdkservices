#ifndef __WHITELISTBASE_H
#define __WHITELISTBASE_H

#include "Module.h"

#include <wpe/webkit-web-extension.h>
#include <vector>
#include <map>
#include <memory>

namespace WPEFramework {
namespace WebKit {

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

        virtual ~JSONEntry() {}

        Core::JSON::String Origin;
        Core::JSON::ArrayType<Core::JSON::String> Domain;
    };

    class WhitelistBase {
    public:
        virtual ~WhitelistBase() {}
        virtual void AddToWebKit(WebKitWebExtension* extension) = 0;

    private:
        WhitelistBase& operator=(const JSONEntry&) = delete;
    };

}
}

#endif // __WHITELISTBASE_H
