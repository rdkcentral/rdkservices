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

#ifndef __OPENCDMI_H
#define __OPENCDMI_H

#include "Module.h"
#include <interfaces/IContentDecryption.h>
#include <interfaces/IMemory.h>
#include <interfaces/json/JsonData_OCDM.h>
#include "utils.h"

namespace WPEFramework {
namespace Plugin {

    class OCDM : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    private:
        OCDM(const OCDM&) = delete;
        OCDM& operator=(const OCDM&) = delete;

        class Notification : public RPC::IRemoteConnection::INotification,
                             public Exchange::IContentDecryption::INotification {

        private:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

        public:
            explicit Notification(OCDM* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~Notification()
            {
            }

        public:
            virtual void Activated(RPC::IRemoteConnection*)
            {
            }
            virtual void Deactivated(RPC::IRemoteConnection* connection)
            {
                _parent.Deactivated(connection);
            }
            void initializationStatus(const std::string& keySystem,
                                      Exchange::IContentDecryption::Status status) override
            {
                _parent.OnEvent(keySystem, static_cast<JsonData::OCDM::DrminitializationstatusParamsData::StatusType>(status));
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            INTERFACE_ENTRY(Exchange::IContentDecryption::INotification)
            END_INTERFACE_MAP

        private:
            OCDM& _parent;
        };

    public:
        class Data : public Core::JSON::Container {
        private:
            Data(const Data&) = delete;
            Data& operator=(const Data&) = delete;

        public:
            class System : public Core::JSON::Container {
            private:
                System& operator=(const System&) = delete;

            public:
                System()
                    : Name()
                    , Designators()
                {
                    Add(_T("name"), &Name);
                    Add(_T("designators"), &Designators);
                }
                System(const string& name, RPC::IStringIterator* entries)
                    : Name()
                    , Designators()
                {
                    Add(_T("name"), &Name);
                    Add(_T("designators"), &Designators);

                    ASSERT(entries != nullptr);

                    Name = name;
                    Load(entries);
                }
                System(const System& copy)
                    : Name(copy.Name)
                    , Designators(copy.Designators)
                {
                    Add(_T("name"), &Name);
                    Add(_T("designators"), &Designators);
                }
                virtual ~System()
                {
                }

            public:
                Core::JSON::String Name;
                Core::JSON::ArrayType<Core::JSON::String> Designators;

                inline void Load(RPC::IStringIterator* entries)
                {
                    Designators.Clear();
                    TRACE(Trace::Information, (_T("Adding Designators: %d"), __LINE__));
                    string entry;
                    while (entries->Next(entry) == true) {
                        TRACE(Trace::Information, (_T("Designator: %s"), entry.c_str()));
                        Designators.Add() = entry;
                    }
                }
            };

        public:
            Data()
                : Core::JSON::Container()
            {
                Add(_T("systems"), &Systems);
            }
            ~Data()
            {
            }

        public:
            Core::JSON::ArrayType<System> Systems;
        };

    public:
        #ifdef __WINDOWS__
        #pragma warning(disable : 4355)
        #endif
        OCDM()
            : _service(nullptr)
            , _opencdmi(nullptr)
            , _memory(nullptr)
            , _notification(this)
        {
            RegisterAll();
        }
        #ifdef __WINDOWS__
        #pragma warning(default : 4355)
        #endif
        virtual ~OCDM()
        {
            UnregisterAll();
        }

        void OnEvent(const std::string& keySystem,
                     const JsonData::OCDM::DrminitializationstatusParamsData::StatusType status)
        {
            if (status == JsonData::OCDM::DrminitializationstatusParamsData::StatusType::BUSY)
            {
                JsonData::OCDM::SessionInfo data;
                data.Drm = keySystem;
                sendNotify(_T("drmalreadyinitialized"), data);
            }

            JsonData::OCDM::DrminitializationstatusParamsData data;
            data.Status = status;
            data.Drm = keySystem;
            sendNotify(_T("drminitializationstatus"), data);
        }

    public:
        BEGIN_INTERFACE_MAP(OCDM)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IContentDecryption, _opencdmi)
        INTERFACE_AGGREGATE(Exchange::IMemory, _memory)
        END_INTERFACE_MAP

    public:
        //  IPlugin methods
        // -------------------------------------------------------------------------------------------------------

        // First time initialization. Whenever a plugin is loaded, it is offered a Service object with relevant
        // information and services for this particular plugin. The Service object contains configuration information that
        // can be used to initialize the plugin correctly. If Initialization succeeds, return nothing (empty string)
        // If there is an error, return a string describing the issue why the initialisation failed.
        // The Service object is *NOT* reference counted, lifetime ends if the plugin is deactivated.
        // The lifetime of the Service object is guaranteed till the deinitialize method is called.
        virtual const string Initialize(PluginHost::IShell* service);

        // The plugin is unloaded from the webbridge. This is call allows the module to notify clients
        // or to persist information if needed. After this call the plugin will unlink from the service path
        // and be deactivated. The Service object is the same as passed in during the Initialize.
        // After theis call, the lifetime of the Service object ends.
        virtual void Deinitialize(PluginHost::IShell* service);

        // Returns an interface to a JSON struct that can be used to return specific metadata information with respect
        // to this plugin. This Metadata can be used by the MetData plugin to publish this information to the ouside world.
        virtual string Information() const;

        //  IWeb methods
        // -------------------------------------------------------------------------------------------------------
        virtual void Inbound(Web::Request& request);
        virtual Core::ProxyType<Web::Response> Process(const Web::Request& request);

    private:
        void Deactivated(RPC::IRemoteConnection* process);

        bool KeySystems(const string& name, Core::JSON::ArrayType<Core::JSON::String>& response) const;

        // JsonRpc
        void RegisterAll();
        void UnregisterAll();
        uint32_t get_drms(Core::JSON::ArrayType<JsonData::OCDM::DrmData>& response) const;
        uint32_t get_keysystems(const string& index, Core::JSON::ArrayType<Core::JSON::String>& response) const;
        uint32_t get_sessions(Core::JSON::ArrayType<JsonData::OCDM::SessionInfo>& response) const;

    private:
        uint8_t _skipURL;
        uint32_t _connectionId;
        PluginHost::IShell* _service;
        Exchange::IContentDecryption* _opencdmi;
        Exchange::IMemory* _memory;
        Core::Sink<Notification> _notification;
    };
} //namespace Plugin

} //namespace Solution

#endif // __OPENCDMI_H
