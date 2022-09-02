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

namespace WPEFramework {
namespace Plugin {

    class OCDM : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    private:

        class Notification : public RPC::IRemoteConnection::INotification {
        public:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

            explicit Notification(OCDM* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~Notification() override = default;

        public:
            virtual void Activated(RPC::IRemoteConnection*)
            {
            }
            virtual void Deactivated(RPC::IRemoteConnection* connection)
            {
                _parent.Deactivated(connection);
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            OCDM& _parent;
        };

    public:
        class Data : public Core::JSON::Container {
        public:
            class System : public Core::JSON::Container {
            private:
                System& operator=(const System&) = delete;

            public:
                System()
                    : Core::JSON::Container()
                    , Name()
                    , Designators()
                {
                    Add(_T("name"), &Name);
                    Add(_T("designators"), &Designators);
                }
                System(const string& name, RPC::IStringIterator* entries)
                    : Core::JSON::Container()
                    , Name()
                    , Designators()
                {
                    Add(_T("name"), &Name);
                    Add(_T("designators"), &Designators);

                    ASSERT(entries != nullptr);

                    Name = name;
                    Load(entries);
                }
                System(const System& copy)
                    : Core::JSON::Container()
                    , Name(copy.Name)
                    , Designators(copy.Designators)
                {
                    Add(_T("name"), &Name);
                    Add(_T("designators"), &Designators);
                }
                ~System() override = default;

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
            Data(const Data&) = delete;
            Data& operator=(const Data&) = delete;
            Data()
                : Core::JSON::Container()
            {
                Add(_T("systems"), &Systems);
            }
            ~Data() = default;

        public:
            Core::JSON::ArrayType<System> Systems;
        };

    public:
        OCDM(const OCDM&) = delete;
        OCDM& operator=(const OCDM&) = delete;

        OCDM()
            : _connectionId(0)
            , _service(nullptr)
            , _opencdmi(nullptr)
            , _memory(nullptr)
            , _notification(this)
        {
        }

        ~OCDM() override = default;

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
        const string Initialize(PluginHost::IShell* service) override;

        // The plugin is unloaded from the webbridge. This is call allows the module to notify clients
        // or to persist information if needed. After this call the plugin will unlink from the service path
        // and be deactivated. The Service object is the same as passed in during the Initialize.
        // After theis call, the lifetime of the Service object ends.
        void Deinitialize(PluginHost::IShell* service) override;

        // Returns an interface to a JSON struct that can be used to return specific metadata information with respect
        // to this plugin. This Metadata can be used by the MetData plugin to publish this information to the ouside world.
        string Information() const override;

        //  IWeb methods
        // -------------------------------------------------------------------------------------------------------
        void Inbound(Web::Request& request) override;
        Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

    private:
        void Deactivated(RPC::IRemoteConnection* process);

        bool KeySystems(const string& name, Core::JSON::ArrayType<Core::JSON::String>& response) const;

        // JsonRpc
        void RegisterAll();
        void UnregisterAll();
        uint32_t get_drms(Core::JSON::ArrayType<JsonData::OCDM::DrmData>& response) const;
        uint32_t get_keysystems(const string& index, Core::JSON::ArrayType<Core::JSON::String>& response) const;

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
