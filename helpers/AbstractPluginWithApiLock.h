#pragma once

#include <mutex>

#include <AbstractPlugin.h>

namespace WPEFramework {

    namespace Plugin {

        namespace {
            // set when inside of getFunctionToCall wrapper (or locked IARM handler - see AbstractPluginWithApiAndIARMLock)
            thread_local bool isThreadUsingLockedApi = false;
        }
        /*
            When plugin extends this instead of AbstractPlugin all API method invocations will be mutex protected.
        */
        class AbstractPluginWithApiLock : public AbstractPlugin {

        public:

            AbstractPluginWithApiLock(const uint8_t currVersion) : AbstractPlugin(currVersion) {}
            AbstractPluginWithApiLock() : AbstractPlugin() {}

        protected:

            template <typename METHOD, typename REALOBJECT>
            std::function<uint32_t(REALOBJECT*, const WPEFramework::Core::JSON::VariantContainer&, WPEFramework::Core::JSON::VariantContainer&)>
            getFunctionToCall(const METHOD& method, REALOBJECT* objectPtr) {
                return [method](REALOBJECT *obj, const WPEFramework::Core::JSON::VariantContainer& in, WPEFramework::Core::JSON::VariantContainer& out) -> uint32_t {
                    isThreadUsingLockedApi = true;
                    std::lock_guard<std::mutex> lock(getApiLock());
                    uint32_t ret;
                    try {
                        ret = (obj->*method)(in, out);
                    } catch (...) {
                        isThreadUsingLockedApi = false;
                        throw;
                    }
                    isThreadUsingLockedApi = false;
                    return ret;
                };
            }

            /* we are hiding, not overriding, AbstractPlugin::registerMethod */
            template <typename METHOD, typename REALOBJECT>
            void registerMethod(const string& methodName, const METHOD& method, REALOBJECT* objectPtr)
            {
                WPEFramework::Plugin::AbstractPlugin::registerMethod(methodName, getFunctionToCall(method, objectPtr), objectPtr);
            }

            /* we are hiding, not overriding, AbstractPlugin::registerMethod */
            template <typename METHOD, typename REALOBJECT>
            void registerMethod(const string& methodName, const METHOD& method, REALOBJECT* objectPtr, const std::vector<uint8_t> versions)
            {
                WPEFramework::Plugin::AbstractPlugin::registerMethod(methodName, getFunctionToCall(method, objectPtr), objectPtr, versions);
            }

        public:
            /*
                This guard can unlock & re-lock api mutex to prevent deadlock possible when calling other plugins via Invoke
                (could deadlock in case when that other plugin called Invoke on this plugin at the same time, or tried to call
                this plugin recursively, from the Invoke'd call).
            */
            struct UnlockApiGuard {
                UnlockApiGuard() {
                    if (isThreadUsingLockedApi) {
                        getApiLock().unlock();
                    }
                }
                ~UnlockApiGuard() {
                    if (isThreadUsingLockedApi) {
                        getApiLock().lock();
                    }
                }
            };

            static std::mutex& getApiLock() {
                static std::mutex apiLock;
                return apiLock;
            }
        };

    } // Plugin
} // WPEFramework
