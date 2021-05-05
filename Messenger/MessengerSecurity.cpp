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

#include <interfaces/json/JsonData_Messenger.h>

#include <regex>
#include <algorithm>

// helper functions
namespace {

    bool IsAllowed(WPEFramework::PluginHost::IShell* service, const string& token, const string& designator)
    {
        bool result = false;

        auto auth = service->QueryInterfaceByCallsign<WPEFramework::PluginHost::IAuthenticate>("SecurityAgent");
        if (auth != nullptr) {
            // Token passed from Thunder isn't a token but a URL, convert it..
            JsonObject object;
            object["url"] = token;
            string payload;
            object.ToString(payload);

            string encoded;
            if (auth->CreateToken(
                    static_cast<uint16_t>(payload.length()),
                    reinterpret_cast<const uint8_t *>(payload.c_str()),
                    encoded) == WPEFramework::Core::ERROR_NONE) {
                WPEFramework::PluginHost::ISecurity *officer = auth->Officer(encoded);
                if (officer != nullptr) {
                    WPEFramework::Core::JSONRPC::Message message;
                    message.Designator = designator;

                    result = officer->Allowed(message);
                }
            }
        }

        return result;
    }

    string CreateUrlRegex(const string& input)
    {
        string regex = input;

        regex = std::regex_replace(regex, std::regex(R"([-[\]{}()+?.,\^$|#\s])"), R"(\$&)");
        regex = std::regex_replace(regex, std::regex(":\\*"), ":[0-9]+");
        regex = std::regex_replace(regex, std::regex("\\*:"), "[a-z]+:");
        regex = std::regex_replace(regex, std::regex("\\*"), "[a-zA-Z0-9\\.\\-]+");
        regex.insert(regex.begin(), '^');
        regex.insert(regex.end(), '$');

        return regex;
    }

    string GetUrlOrigin(const string& input)
    {
        // see https://tools.ietf.org/html/rfc3986
        auto path = input.find('/', input.find("//") + 2);
        auto fragment = input.rfind('#', path);
        auto end = fragment == string::npos ? path : fragment;
        auto query = input.rfind('?', end);
        if (query != string::npos)
            end = query;

        return input.substr(0, end);
    }
} // namespace

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::Messenger;

    // TokenCheckFunction

    bool Messenger::CheckToken(const string& token, const string& method, const string& parameters)
    {
        bool result = false;

        if (method != _T("join")) {
            result = true;
        } else {
            JoinParamsData params;
            params.FromString(parameters);
            auto room = params.Room.Value();
            bool secure = params.Secure.Value();
            const auto& acl = params.Acl;

            _adminLock.Lock();

            auto actualAcl = _roomACL.find(room);
            bool aclSet = actualAcl != _roomACL.end();

            if (!secure) {
                if (aclSet) {
                    TRACE(Trace::Error, (_T("Room '%s' is secure"), room.c_str()));
                } else {
                    result = true;
                }
            } else {
                bool settingAcl = acl.IsSet();
                Core::JSON::ArrayType<Core::JSON::String>::ConstIterator index = acl.Elements();
                bool roomExists = _rooms.find(room) != _rooms.end();

                if (!aclSet) {
                    TRACE(Trace::Information, (_T("Joining room '%s' w/o ACL"), room.c_str()));

                    if (!settingAcl) {
                        TRACE(Trace::Error, (_T("Room '%s' isn't secure"), room.c_str()));
                    } else if (roomExists) {
                        TRACE(Trace::Error, (_T("Can't set ACL of an active room '%s'"), room.c_str()));
                    } else if (!IsAllowed(_service, token, _service->Callsign() + ".acl")) {
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
                } else {
                    TRACE(Trace::Information, (_T("Joining room '%s' w/ ACL"), room.c_str()));

                    if (settingAcl) {
                        TRACE(Trace::Error, (_T("ACL for '%s' already set"), room.c_str()));
                    } else {
                        result = std::any_of(actualAcl->second.begin(), actualAcl->second.end(),
                                             [&token](const string &i) {
                                                 return std::regex_search(GetUrlOrigin(token),
                                                                          std::regex(CreateUrlRegex(i)));
                                             });
                    }
                }
            }

            _adminLock.Unlock();
        }

        return result;
    }

} // namespace Plugin

} // WPEFramework
