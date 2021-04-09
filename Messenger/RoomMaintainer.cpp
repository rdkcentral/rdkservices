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
 
#include "Module.h"
#include "RoomMaintainer.h"
#include "RoomImpl.h"

namespace WPEFramework {

namespace Plugin {

    SERVICE_REGISTRATION(RoomMaintainer, 1, 0);

    /* virtual */ Exchange::IRoomAdministrator::IRoom* RoomMaintainer::Join(const string& roomId, const string& userId,
                                                                            Exchange::IRoomAdministrator::IRoom::IMsgNotification* messageSink)
    {
        // Note: Nullptr message sink is allowed (e.g. for broadcast-only users).

        RoomImpl* newRoomUser = nullptr;

        _adminLock.Lock();

        auto  it(_roomMap.find(roomId));

        if (it == _roomMap.end()) {
            // Room not found, so create one, already emplacing the first user.
            newRoomUser = Core::Service<RoomImpl>::Create<RoomImpl>(this, roomId, userId, messageSink);
            it = _roomMap.emplace(roomId, std::list<RoomImpl*>({newRoomUser})).first;

            TRACE(Trace::Information, (_T("Room Maintainer: Room '%s' created"), roomId.c_str()));
            if (roomId.size() == 0) {
                TRACE(Trace::Warning, (_T("Room Maintainer: Created a room with empty roomId")));
            }

            // Notify the observers about a new room.
            for (auto& observer : _observers) {
                observer->Created(roomId);
            }
        }
        else {
            // Room already created; try to add another user.
            std::list<RoomImpl*>& users = (*it).second;

            if (std::find_if(users.begin(), users.end(), [&userId](const RoomImpl* user) { return (user->UserId() == userId);}) == users.end()) {
                newRoomUser = Core::Service<RoomImpl>::Create<RoomImpl>(this, roomId, userId, messageSink);

                // Notify the room about a joining user.
                // No point in sending the notification to the joining user as it cannot have its callback registered yet.
                for (auto& user : users) {
                    user->UserJoined(userId);
                }

                users.push_back(newRoomUser);
            }
            else {
                TRACE(Trace::Error, (_T("Room Maintainer: User '%s' has already joined room '%s'"),
                        userId.c_str(), roomId.c_str()));
            }
        }

        if (newRoomUser) {
            TRACE(Trace::Information, (_T("Room Maintainer: User '%s' has joined room '%s'"),
                    userId.c_str(), roomId.c_str()));
        }

        _adminLock.Unlock();

        // May be nullptr if the user has already joined the room earlier.
        return newRoomUser;
    }

    void RoomMaintainer::Exit(const RoomImpl* roomUser)
    {
        ASSERT(roomUser != nullptr);

        _adminLock.Lock();

        auto it(_roomMap.find(roomUser->RoomId()));
        ASSERT(it != _roomMap.end());

        if (it != _roomMap.end()) {
            std::list<RoomImpl*>& users = (*it).second;

            auto uit(std::find(users.begin(), users.end(), roomUser));
            ASSERT(uit != users.end());

            if (uit != users.end()) {
                TRACE(Trace::Information, (_T("Room Maintainer: User '%s' is leaving room '%s'"),
                        roomUser->UserId().c_str(), roomUser->RoomId().c_str()));

                // Notify the room members about a leaving user.
                for (auto& user : users) {
                    user->UserLeft(roomUser->UserId());
                }

                users.erase(uit);

                // Was it the last user?
                if (users.size() == 0) {
                    _roomMap.erase(it);

                    TRACE(Trace::Information, (_T("Room Maintainer: Room '%s' has been destroyed"), roomUser->RoomId().c_str()));

                    // Notify the observers about the destruction of this room.
                    for (auto& observer : _observers) {
                        observer->Destroyed(roomUser->RoomId());
                    }
                }
            }
        }

        _adminLock.Unlock();
    }

    void RoomMaintainer::Notify(RoomImpl* roomUser)
    {
        ASSERT(roomUser != nullptr);

        _adminLock.Lock();

        auto it = _roomMap.find(roomUser->RoomId());
        ASSERT(it != _roomMap.end());

        if (it != _roomMap.end()) {
            for (auto& user : (*it).second) {
                roomUser->UserJoined(user->UserId());
            }
        }

        _adminLock.Unlock();
    }

    void RoomMaintainer::Send(const string& message, RoomImpl* roomUser)
    {
        ASSERT(roomUser != nullptr);

        _adminLock.Lock();

        auto it(_roomMap.find(roomUser->RoomId()));
        ASSERT(it != _roomMap.end());

        if (it != _roomMap.end()) {
            for (RoomImpl* user : (*it).second) {
                user->MessageReceived(roomUser->UserId(), message);
            }
        }

        _adminLock.Unlock();
    }

    /* virtual */ void RoomMaintainer::Register(INotification* sink)
    {
        ASSERT(sink != nullptr);

        _adminLock.Lock();

        // Make sure it's not registered multiple times.
        ASSERT(std::find(_observers.cbegin(), _observers.cend(), sink) == _observers.cend());

        _observers.push_back(sink);
        sink->AddRef();

        // Notify the caller about all rooms created to date.
        for (auto const& room : _roomMap) {
            sink->Created(room.first);
        }

        _adminLock.Unlock();

        TRACE(Trace::Information, (_T("Room Maintainer: Registered a notification sink")));
    }

    /* virtual */ void RoomMaintainer::Unregister(const INotification* sink)
    {
        ASSERT(sink != nullptr);

        _adminLock.Lock();

        auto it(std::find(_observers.begin(), _observers.end(), sink));

        // Make sure it was really registered.
        ASSERT(it != _observers.cend());

        if (it != _observers.cend()) {
            (*it)->Release();
            _observers.erase(it);
        }

        _adminLock.Unlock();

        TRACE(Trace::Information, (_T("Room Maintainer: Unregistered a notification sink")));
    }

} // namespace Plugin

} // namespace WPEFramework
