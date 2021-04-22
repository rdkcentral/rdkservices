/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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
**/

#pragma once

#include <unordered_map>
#include "utils.h"

namespace WPEFramework {

    namespace Plugin {

		// This is a server for a JSONRPC communication channel.
		// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
		// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
		// This realization of this interface implements, by default, the following methods on this plugin
		// - exists
		// - register
		// - unregister
		// Any other methood to be handled by this plugin  can be added can be added by using the
		// templated methods Register on the PluginHost::JSONRPC class.
		// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
		// this class exposes a public method called, Notify(), using this methods, all subscribed clients
		// will receive a JSONRPC message as a notification, in case this method is called.
        class AbstractPlugin : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            // We do not allow this plugin to be copied !!
            AbstractPlugin(const AbstractPlugin&) = delete;
            AbstractPlugin& operator=(const AbstractPlugin&) = delete;

            //Begin methods
            virtual uint32_t getQuirks(const JsonObject& parameters, JsonObject& response)
            {
                JsonArray array;
                response["quirks"] = array;
                returnResponse(true);
            }
            //End methods

        protected:

            //registerMethod to register a method in all versions
            template <typename METHOD, typename REALOBJECT>
            void registerMethod(const string& methodName, const METHOD& method, REALOBJECT* objectPtr)
            {
                for(uint8_t ver = 1; ver <= m_currVersion; ver++)  
                {
                    auto handler = m_versionHandlers.find(ver);
                    if(handler != m_versionHandlers.end())
                    {
                        handler->second->Register<WPEFramework::Core::JSON::VariantContainer, WPEFramework::Core::JSON::VariantContainer, METHOD, REALOBJECT>(methodName, method, objectPtr);
                        m_versionAPIs[ver].push_back(methodName);
                    }
                }
            }

            //registerMethod to register a method in specific versions
            template <typename METHOD, typename REALOBJECT>
            void registerMethod(const string& methodName, const METHOD& method, REALOBJECT* objectPtr, const std::vector<uint8_t> versions)
            {
                for(auto ver : versions)
                {
                    auto handler = m_versionHandlers.find(ver);
                    if(handler != m_versionHandlers.end())
                    {
                        handler->second->Register<WPEFramework::Core::JSON::VariantContainer, WPEFramework::Core::JSON::VariantContainer, METHOD, REALOBJECT>(methodName, method, objectPtr);
                        m_versionAPIs[ver].push_back(methodName);
                    }
                } 
            }

            void LOGT2(char* message)
            {
                Utils::Telemetry::sendMessage(message);
            }

        public:
            AbstractPlugin() : PluginHost::JSONRPC(), m_currVersion(1)
            {
                // For default constructor assume that only version 1 is supported.
                // Also version 1 handler would always be the current object.
                m_versionHandlers[1] = GetHandler(1);

                registerMethod("getQuirks", &AbstractPlugin::getQuirks, this);

                Utils::Telemetry::init();
            }

            AbstractPlugin(const uint8_t currVersion) : PluginHost::JSONRPC(), m_currVersion(currVersion)
            {
                // Create handlers for all the versions upto m_currVersion
                m_versionHandlers[1] = GetHandler(1);
                for(uint8_t i = 2; i <= m_currVersion; i++)
                {
                    std::vector<uint8_t> vec;
                    vec.push_back(i);
                    CreateHandler(vec);
                    m_versionHandlers[i] = GetHandler(i);
                }

                registerMethod("getQuirks", &AbstractPlugin::getQuirks, this);

                Utils::Telemetry::init();
            }

            virtual ~AbstractPlugin()
            {
            }

            //Build QueryInterface implementation, specifying all possible interfaces to be returned.
            BEGIN_INTERFACE_MAP(MODULE_NAME)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service)
            {
                // On success return empty, to indicate there is no error text.
                return(string());
            }

            virtual void Deinitialize(PluginHost::IShell* service)
            {
                // unregister all registered APIs from all supported versions
                for (const auto& kv : m_versionHandlers) 
                {
                    auto handler = kv.second;
                    for ( auto api : m_versionAPIs[kv.first] )
                    {
                        handler->Unregister(api);
                    }
                }
            }

            virtual string Information() const
            {
                // No additional info to report.
                return(string());
            }

            // Note: PluginHost::JSONRPC::Notify aren't virtual!
            uint32_t Notify(const string& event)
            {
                uint32_t ret = Core::ERROR_UNKNOWN_KEY;

                for (auto it = m_versionHandlers.begin(); it != m_versionHandlers.end(); ++it)
                    if (it->second->Notify(event, Core::JSON::String()) == Core::ERROR_NONE)
                        ret = Core::ERROR_NONE;

                return ret;
            }
            template <typename JSONOBJECT>
            uint32_t Notify(const string& event, const JSONOBJECT& parameters)
            {
                uint32_t ret = Core::ERROR_UNKNOWN_KEY;

                for (auto it = m_versionHandlers.begin(); it != m_versionHandlers.end(); ++it)
                    if (it->second->Notify(event, parameters) == Core::ERROR_NONE)
                        ret = Core::ERROR_NONE;

                return ret;
            }
            template <typename JSONOBJECT, typename SENDIFMETHOD>
            uint32_t Notify(const string& event, const JSONOBJECT& parameters, SENDIFMETHOD method)
            {
                uint32_t ret = Core::ERROR_UNKNOWN_KEY;

                for (auto it = m_versionHandlers.begin(); it != m_versionHandlers.end(); ++it)
                    if (it->second->Notify(event, parameters, method) == Core::ERROR_NONE)
                        ret = Core::ERROR_NONE;

                return ret;
            }

        private:
            std::unordered_map<uint8_t, WPEFramework::Core::JSONRPC::Handler*> m_versionHandlers;
            std::unordered_map<uint8_t, std::vector<std::string>> m_versionAPIs;
            uint8_t m_currVersion; // current supported version
        };
	} // namespace Plugin
} // namespace WPEFramework
