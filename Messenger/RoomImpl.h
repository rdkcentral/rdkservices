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
#include "RoomMaintainer.h"

namespace WPEFramework {

namespace Plugin {

    class RoomImpl : public Exchange::IRoomAdministrator::IRoom {
    public:
        RoomImpl() = delete;
        RoomImpl(const RoomImpl&) = delete;
        RoomImpl& operator=(const RoomImpl&) = delete;

        RoomImpl(RoomMaintainer* admin, const string& roomId, const string& userId, IMsgNotification* messageSink)
            : _roomId(roomId)
            , _userId(userId)
            , _roomAdmin(admin)
            , _callback(nullptr)
            , _messageSink(messageSink)
            , _adminLock()
        {
            ASSERT(admin != nullptr);

            _roomAdmin->AddRef();

            if (_messageSink) {
                _messageSink->AddRef();
            }

            if (userId.size() == 0) {
                TRACE(Trace::Warning, (_T("Created a user with empty userId")));
            }
        }

        virtual ~RoomImpl()
        {
            ASSERT(_roomAdmin != nullptr);

            _roomAdmin->Exit(this);

            // Release the callback if necessary.
            SetCallback(nullptr);

            if (_messageSink) {
                _messageSink->Release();
            }

            _roomAdmin->Release();
        }

        // IRoom methods
        virtual void SendMessage(const string& message) override
        {
            ASSERT(_roomAdmin != nullptr);
            _roomAdmin->Send(message, this);

            // Note: the message will be echoed back to the sending user.
        }

        virtual void SetCallback(ICallback *callback) override
        {
            ASSERT(_roomAdmin != nullptr);

            if (callback != nullptr) {
                callback->AddRef();
            }

            _adminLock.Lock();

            if (_callback != nullptr) {
                _callback->Release();
            }

            _callback = callback;

            _adminLock.Unlock();

            TRACE(Trace::Information, (_T("User '%s': %s the callback"),
                    UserId().c_str(), (callback != nullptr? _T("Registered") : _T("Unregistered"))));

            // Find out about all users that have already joined the room to date.
            if (callback != nullptr) {
                _roomAdmin->Notify(this);
            }
        }

        // RoomImpl methods
        void UserJoined(const string& userId)
        {
            TRACE(Trace::Information, (_T("User '%s': Notified that '%s' joined room '%s'"),
                    UserId().c_str(), userId.c_str(), RoomId().c_str()));

            _adminLock.Lock();

            if (_callback != nullptr) {
                _callback->Joined(userId);
            }

            _adminLock.Unlock();
        }

        void UserLeft(const string& userId)
        {
            TRACE(Trace::Information, (_T("User '%s': Notified that '%s' left room '%s'"),
                    UserId().c_str(), userId.c_str(), RoomId().c_str()));

            _adminLock.Lock();

            if (_callback != nullptr) {
                _callback->Left(userId);
            }

            _adminLock.Unlock();
        }

        void MessageReceived(const string& userId, const string& message)
        {
            if (_messageSink != nullptr) {
                _messageSink->Message(userId, message);
            }
        }

        const string& UserId() const { return _userId; }
        const string& RoomId() const { return _roomId; }

        // QueryInterface implementation
        BEGIN_INTERFACE_MAP(RoomImpl)
            INTERFACE_ENTRY(Exchange::IRoomAdministrator::IRoom)
        END_INTERFACE_MAP

    private:
        string _roomId;
        string _userId;
        RoomMaintainer* _roomAdmin;
        Exchange::IRoomAdministrator::IRoom::ICallback* _callback;
        Exchange::IRoomAdministrator::IRoom::IMsgNotification* _messageSink;
        mutable Core::CriticalSection _adminLock;
    };

} // namespace Plugin

} // namespace WPEFramework
