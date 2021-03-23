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
 
#include <interfaces/json/JsonData_Messenger.h>
#include "Messenger.h"
#include "Module.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::Messenger;

    // Registration
    //

    void Messenger::RegisterAll()
    {
        RegisterEventStatusListener(_T("roomupdate"), [this](const string& client, Status status) {
            // Notify of all rooms created to date.
            for (const string& room : _rooms) {
                event_roomupdate(room, JsonData::Messenger::RoomupdateParamsData::ActionType::CREATED);
            }
        });

        RegisterEventStatusListener(_T("userupdate"), [this](const string& client, Status status) {
            // Subscribe the lowe level room user to userupdate notification.
            // This may immediately sent notifications of all users already present in the room.
            const string roomId = client.substr(0, client.find('.'));
            SubscribeUserUpdate(roomId, status == Status::registered);
        });

        Register<JoinParamsData,JoinResultInfo>(_T("join"), &Messenger::endpoint_join, this);
        Register<JoinResultInfo,void>(_T("leave"), &Messenger::endpoint_leave, this);
        Register<SendParamsData,void>(_T("send"), &Messenger::endpoint_send, this);
        Register<CreateParamsData,void>(_T("create"), &Messenger::endpoint_create, this);
    }

    void Messenger::UnregisterAll()
    {
        Unregister(_T("create"));
        Unregister(_T("send"));
        Unregister(_T("leave"));
        Unregister(_T("join"));
        UnregisterEventStatusListener(_T("userupdate"));
        UnregisterEventStatusListener(_T("roomupdate"));
    }

    // API implementation
    //

    // Joins a messaging room.
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_ILLEGAL_STATE: User name is already taken (i.e. the user has already joined the room)
    //  - ERROR_BAD_REQUEST: User name or room name was invalid
    uint32_t Messenger::endpoint_join(const JoinParamsData& params, JoinResultInfo& response)
    {
        uint32_t result = Core::ERROR_BAD_REQUEST;
        const string& user = params.User.Value();
        const string& room = params.Room.Value();

        if (!user.empty() && !room.empty()) {
            string roomId = JoinRoom(room, user);
            if (!roomId.empty()) {
                response.Roomid = roomId;
                result = Core::ERROR_NONE;
            }
            else {
                result = Core::ERROR_ILLEGAL_STATE;
            }
        }

        return result;
    }

    // Leaves a messaging room.
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNKNOWN_KEY: The given room ID was invalid
    uint32_t Messenger::endpoint_leave(const JoinResultInfo& params)
    {
        const string& roomid = params.Roomid.Value();

        bool result = LeaveRoom(roomid);

        return result? Core::ERROR_NONE : Core::ERROR_UNKNOWN_KEY;
    }

    // Sends a message to a room.
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_UNKNOWN_KEY: The given room ID was invalid
    uint32_t Messenger::endpoint_send(const SendParamsData& params)
    {
        const string& roomid = params.Roomid.Value();
        const string& message = params.Message.Value();

        bool result = SendMessage(roomid, message);

        return result? Core::ERROR_NONE : Core::ERROR_UNKNOWN_KEY;
    }

    // Creates a messaging room name and URL regex.
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_BAD_REQUEST: Room name or URL regex was invalid
    uint32_t Messenger::endpoint_create(const CreateParamsData& params)
    {
        uint32_t result = Core::ERROR_BAD_REQUEST;
        const string& room = params.Room.Value();
        const string& urlRegex = params.UrlRegex.Value();

        if (!room.empty() && !urlRegex.empty()) {
            AddRoomACL(room, urlRegex);
            result = Core::ERROR_NONE;
        }

        return result;
    }

    // Notifies about room status updates.
    void Messenger::event_roomupdate(const string& room, const RoomupdateParamsData::ActionType& action)
    {
        RoomupdateParamsData params;
        params.Room = room;
        params.Action = action;

        Notify(_T("roomupdate"), params);
    }

    // Notifies about user status updates.
    void Messenger::event_userupdate(const string& id, const string& user, const UserupdateParamsData::ActionType& action)
    {
        UserupdateParamsData params;
        params.User = user;
        params.Action = action;

        Notify(_T("userupdate"), params, [&](const string& designator) -> bool {
            const string designator_id = designator.substr(0, designator.find('.'));
            return (id == designator_id);
        });
    }

    // Notifies about new messages in a room.
    void Messenger::event_message(const string& id, const string& user, const string& message)
    {
        MessageParamsData params;
        params.User = user;
        params.Message = message;

        Notify(_T("message"), params, [&](const string& designator) -> bool {
            const string designator_id = designator.substr(0, designator.find('.'));
            return (id == designator_id);
        });
    }

    // TokenCheckFunction
    bool Messenger::CheckToken(const string& token, const string& method, const string& parameters)
    {
        bool result = true;

        if (method == _T("create")) {
            CreateParamsData params;
            params.FromString(parameters);
            const string& room = params.Room.Value();

            AddRoomACL(room, token, true);
        }
        else if (method == _T("join")) {
            JoinParamsData params;
            params.FromString(parameters);
            const string& room = params.Room.Value();

            result = IsRoomAllowed(room, token);
        }

        return result;
    }

} // namespace Plugin

}

