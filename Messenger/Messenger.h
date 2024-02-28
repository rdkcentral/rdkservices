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
 
#pragma once

#include "Module.h"
#include <interfaces/IMessenger.h>
#include <interfaces/json/JsonData_Messenger.h>
#include <map>
#include <set>
#include <functional>

namespace WPEFramework {

namespace Plugin {

    class Messenger : public PluginHost::IPlugin
                    , public Exchange::IRoomAdministrator::INotification
                    , public PluginHost::JSONRPCSupportsEventStatus {
#ifdef USE_THUNDER_R4
    private:
        class Notification : public RPC::IRemoteConnection::INotification {
        public:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

            explicit Notification(Messenger* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~Notification() override = default;

        public:
            void Activated(RPC::IRemoteConnection*) override
            {
            }
            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                _parent.Deactivated(connection);
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            Messenger& _parent;
        };
#endif

    public:
        Messenger(const Messenger&) = delete;
        Messenger& operator=(const Messenger&) = delete;

        Messenger()
            : PluginHost::JSONRPCSupportsEventStatus(std::bind(&Messenger::CheckToken, this,
                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
            , _connectionId(0)
            , _service(nullptr)
            , _roomAdmin(nullptr)
            , _roomIds()
            , _adminLock()
#ifdef USE_THUNDER_R4
            , _notification(this)
#endif
        {
        }

        ~Messenger() override = default;

        // IPlugin methods
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override  { return { }; }

        // Notification handling
        class MsgNotification : public Exchange::IRoomAdministrator::IRoom::IMsgNotification {
        public:
            MsgNotification(const MsgNotification&) = delete;
            MsgNotification& operator=(const MsgNotification&) = delete;

            MsgNotification(Messenger* messenger, const string& roomId)
                : _messenger(messenger)
                , _roomId(roomId)
            { /* empty */ }

            // IRoom::Notification methods
            void Message(const string& senderName, const string& message) override
            {
                ASSERT(_messenger != nullptr);
                _messenger->MessageHandler(_roomId, senderName, message);
            }

            // QueryInterface implementation
            BEGIN_INTERFACE_MAP(Callback)
                INTERFACE_ENTRY(Exchange::IRoomAdministrator::IRoom::IMsgNotification)
            END_INTERFACE_MAP

        private:
            Messenger* _messenger;
            string _roomId;
        }; // class Notification

        // Callback handling
        class Callback : public Exchange::IRoomAdministrator::IRoom::ICallback {
        public:
            Callback(const Callback&) = delete;
            Callback& operator=(const Callback&) = delete;

            Callback(Messenger* messenger, const string& roomId)
                : _messenger(messenger)
                , _roomId(roomId)
            { /* empty */}

            // IRoom::ICallback methods
            void Joined(const string& userName) override
            {
                ASSERT(_messenger != nullptr);
                _messenger->UserJoinedHandler(_roomId, userName);
            }

            void Left(const string& userName) override
            {
                ASSERT(_messenger != nullptr);
                _messenger->UserLeftHandler(_roomId, userName);
            }

            // QueryInterface implementation
            BEGIN_INTERFACE_MAP(Callback)
                INTERFACE_ENTRY(Exchange::IRoomAdministrator::IRoom::ICallback)
            END_INTERFACE_MAP

        private:
            Messenger* _messenger;
            string _roomId;
        }; // class Callback

        // QueryInterface implementation
        BEGIN_INTERFACE_MAP(Messenger)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(Exchange::IRoomAdministrator::INotification)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IRoomAdministrator, _roomAdmin)
        END_INTERFACE_MAP

        string JoinRoom(const string& roomId, const string& userName);
        bool LeaveRoom(const string& roomId);
        bool SendMessage(const string& roomId, const string& message);

        void UserJoinedHandler(const string& roomId, const string& userName)
        {
            event_userupdate(roomId, userName, JsonData::Messenger::UserupdateParamsData::ActionType::JOINED);
        }

        void UserLeftHandler(const string& roomId, const string& userName)
        {
            event_userupdate(roomId, userName, JsonData::Messenger::UserupdateParamsData::ActionType::LEFT);
        }

        void MessageHandler(const string& roomId, const string& senderName, const string& message)
        {
            event_message(roomId, senderName, message);
        }

        // IMessenger::INotification methods
        void Created(const string& roomName) override
        {
            _adminLock.Lock();
            ASSERT(_rooms.find(roomName) == _rooms.end());
            _rooms.insert(roomName);
            _adminLock.Unlock();

            event_roomupdate(roomName, JsonData::Messenger::RoomupdateParamsData::ActionType::CREATED);
        }

        void Destroyed(const string& roomName) override
        {
            event_roomupdate(roomName, JsonData::Messenger::RoomupdateParamsData::ActionType::DESTROYED);

            _adminLock.Lock();
            ASSERT(_rooms.find(roomName) != _rooms.end());
            _rooms.erase(roomName);
            _roomACL.erase(roomName);
            _adminLock.Unlock();
        }

    private:
#ifdef USE_THUNDER_R4
        void Deactivated(RPC::IRemoteConnection* connection);
#endif
        string GenerateRoomId(const string& roomName, const string& userName);
        bool SubscribeUserUpdate(const string& roomId, bool subscribe);

        // JSON-RPC
        void RegisterAll();
        void UnregisterAll();
        uint32_t endpoint_join(const JsonData::Messenger::JoinParamsData& params, JsonData::Messenger::JoinResultInfo& response);
        uint32_t endpoint_leave(const JsonData::Messenger::JoinResultInfo& params);
        uint32_t endpoint_send(const JsonData::Messenger::SendParamsData& params);
        void event_roomupdate(const string& room, const JsonData::Messenger::RoomupdateParamsData::ActionType& action);
        void event_userupdate(const string& id, const string& user, const JsonData::Messenger::UserupdateParamsData::ActionType& action);
        void event_message(const string& id, const string& user, const string& message);
#ifndef USE_THUNDER_R4
        bool CheckToken(const string& token, const string& method, const string& parameters);
#else
	PluginHost::JSONRPC::classification CheckToken(const string& token, const string& method, const string& parameters);
#endif
        uint32_t _connectionId;
        PluginHost::IShell* _service;
        Exchange::IRoomAdministrator* _roomAdmin;
        std::map<string, Exchange::IRoomAdministrator::IRoom*> _roomIds;
        std::set<string> _rooms;
        std::map<string, std::list<string>> _roomACL;
        mutable Core::CriticalSection _adminLock;
#ifdef USE_THUNDER_R4
        Core::Sink<Notification> _notification;
#endif
    }; // class Messenger

} // namespace Plugin

} // namespace WPEFramework
