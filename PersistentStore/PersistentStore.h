#pragma once

#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"

#include <vector>
#include <map>

namespace WPEFramework {

    namespace Plugin {

        class PersistentStore :  public AbstractPlugin {
        public:
            PersistentStore();
            virtual ~PersistentStore();
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

        public/*members*/:
            static PersistentStore* _instance;

        public /*constants*/:
            static const short API_VERSION_NUMBER_MAJOR;
            static const short API_VERSION_NUMBER_MINOR;
            static const string SERVICE_NAME;
            //methods
            static const string METHOD_SET_VALUE;
            static const string METHOD_GET_VALUE;
            static const string METHOD_DELETE_KEY;
            static const string METHOD_DELETE_NAMESPACE;
            static const string METHOD_GET_KEYS;
            static const string METHOD_GET_NAMESPACES;
            static const string METHOD_GET_STORAGE_SIZE;
            //events
            static const string EVT_ON_STORAGE_EXCEEDED;
            //other
            static const char* STORE_NAME;
            static const char* STORE_KEY;
            static const int64_t MAX_SIZE_BYTES;
            static const int64_t MAX_VALUE_SIZE_BYTES;

        private/*registered methods (wrappers)*/:

            //methods ("parameters" here is "params" from the curl request)
            uint32_t setValueWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getValueWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t deleteKeyWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t deleteNamespaceWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getKeysWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getNamespacesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getStorageSizeWrapper(const JsonObject& parameters, JsonObject& response);

        private/*internal methods*/:
            PersistentStore(const PersistentStore&) = delete;
            PersistentStore& operator=(const PersistentStore&) = delete;

            bool setValue(const string& ns, const string& key, const string& value);
            bool getValue(const string& ns, const string& key, string& value);
            bool deleteKey(const string& ns, const string& key);
            bool deleteNamespace(const string& ns);
            bool getKeys(const string& ns, std::vector<string>& keys);
            bool getNamespaces(std::vector<string>& namespaces);
            bool getStorageSize(std::map<string, uint64_t>& namespaceSizes);

            void term();
            void vacuum();
            bool init(const char* filename, const char* key = nullptr);

            void* mData;
        };
    } // namespace Plugin
} // namespace WPEFramework
