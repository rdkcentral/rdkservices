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

namespace WPEFramework {

namespace Plugin {

    class RoomImpl;

    class RoomMaintainer : public Exchange::IRoomAdministrator {
    public:
        RoomMaintainer(const RoomMaintainer&) = delete;
        RoomMaintainer& operator=(const RoomMaintainer&) = delete;

        RoomMaintainer()
            : _observers()
            , _roomMap()
            , _adminLock()
        { /* empty */}

        // IRoomAdministrator methods
        virtual IRoom* Join(const string& roomId, const string& userId, IRoom::IMsgNotification* messageSink) override;
        virtual void Register(INotification* sink) override;
        virtual void Unregister(const INotification* sink) override;

        // RoomMaintainer methods
        void Exit(const RoomImpl* roomUser);
        void Send(const string& message, RoomImpl* roomUser);
        void Notify(RoomImpl* roomUser);

        // QueryInterface implementation
        BEGIN_INTERFACE_MAP(RoomMaintainer)
            INTERFACE_ENTRY(Exchange::IRoomAdministrator)
        END_INTERFACE_MAP

    private:
        std::list<INotification*> _observers;
        std::map<string, std::list<RoomImpl*>> _roomMap;
        mutable Core::CriticalSection _adminLock;
    };

} // namespace Plugin

} // namespace WPEFramework
