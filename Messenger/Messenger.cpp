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
#include "Messenger.h"
#include "cryptalgo/Hash.h"

#include <regex>

namespace WPEFramework {

namespace Plugin {

    SERVICE_REGISTRATION(Messenger, 1, 0);

    // IPlugin methods

    /* virtual */ const string Messenger::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_roomAdmin == nullptr);
        ASSERT(_roomIds.empty() == true);
        ASSERT(_rooms.empty() == true);
        ASSERT(_roomACL.empty() == true);

        _service = service;
        _service->AddRef();

        _config.FromString(_service->ConfigLine());

        _roomAdmin = service->Root<Exchange::IRoomAdministrator>(_connectionId, 2000, _T("RoomMaintainer"));
        ASSERT(_roomAdmin != nullptr);

        _roomAdmin->Register(this);

        return { };
    }

    /* virtual */ void Messenger::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(service == _service);

        // Exit all the rooms (if any) that were joined by this client
        for (auto& room : _roomIds) {
            room.second->Release();
        }

        _roomIds.clear();

        _roomAdmin->Unregister(this);
        _rooms.clear();

        _roomAdmin->Release();
        _roomAdmin = nullptr;

        _service->Release();
        _service = nullptr;

        _roomACL.clear();
    }

    // Web request handlers

    string Messenger::JoinRoom(const string& roomName, const string& userName)
    {
        bool result = false;

        string roomId = GenerateRoomId(roomName, userName);

        MsgNotification* sink = Core::Service<MsgNotification>::Create<MsgNotification>(this, roomId);
        ASSERT(sink != nullptr);

        if (sink != nullptr) {
            Exchange::IRoomAdministrator::IRoom* room = _roomAdmin->Join(roomName, userName, sink);

            // Note: Join() can return nullptr if the user has already joined the room.
            if (room != nullptr) {

                _adminLock.Lock();
                result = _roomIds.emplace(roomId, room).second;
                _adminLock.Unlock();
                ASSERT(result);
            }

            sink->Release(); // Make room the only owner of the notification object.
        }

        return (result? roomId : string{});
    }

    bool Messenger::SubscribeUserUpdate(const string& roomId, bool subscribe)
    {
        bool result = false;

        _adminLock.Lock();

        auto it(_roomIds.find(roomId));

        if (it != _roomIds.end()) {
            Callback* cb = nullptr;

            if (subscribe) {
                cb = Core::Service<Callback>::Create<Callback>(this, roomId);
                ASSERT(cb != nullptr);
            }

            (*it).second->SetCallback(cb);

            if (cb != nullptr) {
                cb->Release(); // Make room the only owner of the callback object.
            }

            result = true;
        }

        _adminLock.Unlock();

        return result;
    }

    bool Messenger::LeaveRoom(const string& roomId)
    {
        bool result = false;

        _adminLock.Lock();

        auto it(_roomIds.find(roomId));

        if (it != _roomIds.end()) {
            // Exit the room.
            (*it).second->Release();
            // Invalidate the room ID.
            _roomIds.erase(it);
            result = true;
        }

        _adminLock.Unlock();

        return result;
    }

    bool Messenger::SendMessage(const string& roomId, const string& message)
    {
        bool result = false;

        _adminLock.Lock();

        auto it(_roomIds.find(roomId));

        if (it != _roomIds.end()) {
            // Send the message to the room.
            (*it).second->SendMessage(message);
            result = true;
        }

        _adminLock.Unlock();

        return result;
    }

    void Messenger::AddRoomACL(const string& roomId, const string& regex, bool replace)
    {
        std::string r = regex;

        // Order of replacing is important
        r = std::regex_replace(r, std::regex(R"([-[\]{}()+?.,\^$|#\s])"), R"(\$&)");
        r = std::regex_replace(r, std::regex(":\\*"), ":[0-9]+");
        r = std::regex_replace(r, std::regex("\\*:"), "[a-z]+:");
        r = std::regex_replace(r, std::regex("\\*"), "[a-zA-Z0-9\\.]+");
        r.insert(r.begin(), '^');

        // Note, empty strings '' will match an empty regex '^'

        _adminLock.Lock();

        auto retval = _roomACL.emplace(std::piecewise_construct,
                std::make_tuple(roomId),
                std::make_tuple());

        if (retval.second == false && replace) {
            retval.first->second.clear();
        }
        retval.first->second.emplace_back(r);

        _adminLock.Unlock();
    }

    bool Messenger::IsRoomAllowed(const string& roomId, const string& id) const
    {
        bool result = false;

        if (_config.RequireRoomSecurity.Value() == true) {
            _adminLock.Lock();

            auto acl = _roomACL.find(roomId);
            if (acl != _roomACL.end()) {
                for (auto const& i : acl->second) {
                    if (std::regex_search(id, std::regex(i)) == true) {
                        result = true;
                        break;
                    }
                }
            }

            _adminLock.Unlock();
        } else {
            result = true;
        }

        return result;
    }

    // Helpers

    string Messenger::GenerateRoomId(const string& roomName, const string& userName)
    {
        string timenow;
        Core::Time::Now().ToString(timenow);

        string roomIdBase = roomName + userName + timenow;
        Crypto::SHA1 digest(reinterpret_cast<const uint8_t *>(roomIdBase.c_str()), static_cast<uint16_t>(roomIdBase.length()));

        string roomId;
        Core::ToHexString(digest.Result(), (digest.Length / 2), roomId); // let's take only half of the hash

        return roomId;
    }

} // namespace Plugin

} // WPEFramework

