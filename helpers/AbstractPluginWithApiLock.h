#pragma once

#include <mutex>
#include <plugins/plugins.h>
#include <memory>
#include "UtilsLogging.h"

namespace WPEFramework {

    namespace Plugin {

        namespace {
            // set when inside of getFunctionToCall wrapper (or locked IARM handler - see AbstractPluginWithApiAndIARMLock)
            thread_local bool isThreadUsingLockedApi = false;
        }
        /*
            When plugin extends this instead of AbstractPlugin all API method invocations will be mutex protected.
        */
        class AbstractPluginWithApiLock : public PluginHost::JSONRPC {

        public:

            template <typename METHOD, typename REALOBJECT>
            std::function<uint32_t(REALOBJECT*, const WPEFramework::Core::JSON::VariantContainer&, WPEFramework::Core::JSON::VariantContainer&)>
            getFunctionToCall(const std::string& debugname, const METHOD& method, REALOBJECT* objectPtr) {
                return [debugname, method](REALOBJECT *obj, const WPEFramework::Core::JSON::VariantContainer& in, WPEFramework::Core::JSON::VariantContainer& out) -> uint32_t {
                    isThreadUsingLockedApi = true;
                    std::lock_guard<std::mutex> lock(getApiLock());
                    LOGINFO("calling with lock: %s\n", debugname.c_str());
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


            /* we are hiding, not overriding, Register */
            template <typename METHOD, typename REALOBJECT>
            void Register(const string& methodName, const METHOD& method, REALOBJECT* objectPtr)
            {
                using MethodType = decltype(getFunctionToCall(methodName, method, objectPtr));
                PluginHost::JSONRPC::Register<Core::JSON::VariantContainer, Core::JSON::VariantContainer, MethodType, REALOBJECT>(methodName, getFunctionToCall(methodName, method, objectPtr), objectPtr);
            }

            /* we are hiding, not overriding, Register */
            template <typename METHOD, typename REALOBJECT>
            void Register(const string& methodName, const METHOD& method, REALOBJECT* objectPtr, const std::vector<uint8_t> versions)
            {
                PluginHost::JSONRPC::Register<METHOD,REALOBJECT>(methodName, getFunctionToCall(methodName, method, objectPtr), objectPtr, versions);
            }

            struct HandlerWrapper {
                HandlerWrapper(Core::JSONRPC::Handler* wrapped, AbstractPluginWithApiLock *parent) : wrapped(wrapped), parent(parent) {
                }
                Core::JSONRPC::Handler* wrapped;
                AbstractPluginWithApiLock* parent;

                template <typename INBOUND, typename OUTBOUND, typename METHOD, typename REALOBJECT>
                void Register(const string& methodName, const METHOD& method, REALOBJECT* objectPtr)
                {
                    using MethodType = decltype(parent->getFunctionToCall(methodName, method, objectPtr));
                    wrapped->Register<INBOUND,OUTBOUND,MethodType,REALOBJECT>(methodName, parent->getFunctionToCall(methodName, method, objectPtr), objectPtr);
                }

                template <typename JSONOBJECT>
                uint32_t Notify(const string& event, const JSONOBJECT& parameters) const
                {
                    return wrapped->Notify(event, parameters);
                }

            };

            /* we are hiding, not overriding, GetHandler; need to return wrapper
               to be able to replace  Core::JSONRPC::Handler::Register */
            std::unique_ptr<HandlerWrapper> GetHandler(uint8_t version)
            {
                auto* wrapped = PluginHost::JSONRPC::GetHandler(version);
                return wrapped ? std::unique_ptr<HandlerWrapper>(new HandlerWrapper(wrapped, this)) : nullptr;
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

            /* note this will be shared by all subclasses (normally we have 1 plugin per lib, so that's enough) */
            static std::mutex& getApiLock() {
                static std::mutex apiLock;
                return apiLock;
            }
        };

    } // Plugin
} // WPEFramework
