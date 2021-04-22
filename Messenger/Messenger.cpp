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

#include <interfaces/json/JsonData_Messenger.h>

#include <regex>
#include <algorithm>

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

    // TokenCheckFunction

    bool Messenger::CheckToken(const string& token, const string& method, const string& parameters)
    {
        bool result = true;

        // Only for "join"
        if (method == _T("join")) {
            JsonData::Messenger::JoinParamsData params;
            params.FromString(parameters);
            const string& room = params.Room.Value();
            const bool secure = params.Secure.Value();
            const auto& acl = params.Acl;

            // Only if "secure":true
            if (secure) {
                result = false;

                _adminLock.Lock();

                const bool settingAcl = acl.IsSet();
                auto found = _roomACL.find(room);

                if (found == _roomACL.end()) {
                    TRACE(Trace::Information, (_T("Joining room '%s' w/o ACL"), room.c_str()));

                    if (!settingAcl) {
                        TRACE(Trace::Error, (_T("Room '%s' isn't secure"), room.c_str()));
                    } else if (_rooms.find(room) != _rooms.end()) {
                        TRACE(Trace::Error, (_T("Can't set ACL of an active room '%s'"), room.c_str()));
                    } else {
                        auto auth = _service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>(
                                "SecurityAgent"); // TODO: find a way to get security callsign
                        if (auth == nullptr) {
                            TRACE(Trace::Error, (_T("No Security")));
                        } else {
                            // Token passed here isn't a token but a URL, convert it..
                            JsonObject object;
                            object["url"] = token;
                            string payload;
                            object.ToString(payload);

                            string encoded;
                            if (auth->CreateToken(
                                    static_cast<uint16_t>(payload.length()),
                                    reinterpret_cast<const uint8_t*>(payload.c_str()),
                                    encoded) != Core::ERROR_NONE) {
                                TRACE(Trace::Error, (_T("Token creation failed")));
                            } else {
                                PluginHost::ISecurity* officer = auth->Officer(encoded);
                                if (officer == nullptr) {
                                    TRACE(Trace::Error, (_T("Token isn't valid")));
                                } else {
                                    Core::JSON::ArrayType<Core::JSON::String>::ConstIterator index =
                                            acl.Elements();
                                    Core::JSONRPC::Message message;
                                    message.Designator = "Messenger.acl";

                                    if (!officer->Allowed(message)) {
                                        TRACE(Trace::Error, (_T("Not permitted to set ACL")));
                                    } else if (index.Count() == 0) {
                                        TRACE(Trace::Error, (_T("ACL is empty")));
                                    } else {
                                        auto retval = _roomACL.emplace(std::piecewise_construct,
                                                                       std::make_tuple(room),
                                                                       std::make_tuple());
                                        while (index.Next()) {
                                            retval.first->second.emplace_back(index.Current().Value());
                                        }

                                        result = true;
                                    }
                                }
                            }
                        }
                    }
                } else {
                    TRACE(Trace::Information, (_T("Joining room '%s' w/ ACL"), room.c_str()));

                    if (settingAcl) {
                        TRACE(Trace::Error, (_T("ACL for '%s' already set"), room.c_str()));
                    } else {
                        result = std::any_of(found->second.begin(), found->second.end(),
                                [&token](const string& i) {
                            std::string r = i;
                            r = std::regex_replace(r, std::regex(R"([-[\]{}()+?.,\^$|#\s])"), R"(\$&)");
                            r = std::regex_replace(r, std::regex(":\\*"), ":[0-9]+");
                            r = std::regex_replace(r, std::regex("\\*:"), "[a-z]+:");
                            r = std::regex_replace(r, std::regex("\\*"), "[a-zA-Z0-9\\.]+");
                            r.insert(r.begin(), '^');

                            return std::regex_search(token, std::regex(r));
                        });
                    }
                }

                _adminLock.Unlock();
            }
        }

        return result;
    }

} // namespace Plugin

} // WPEFramework

