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
            string encoded;
            if (auth->CreateToken(
                    static_cast<uint16_t>(token.length()),
                    reinterpret_cast<const uint8_t *>(token.c_str()),
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

    // Note, token is assumed to be a URL
#ifndef USE_THUNDER_R4
    bool Messenger::CheckToken(const string& token, const string& method, const string& parameters)
#else
    PluginHost::JSONRPC::classification Messenger::CheckToken(const string& token, const string& method, const string& parameters)
#endif
    {
        bool result = false;

        if (method != _T("join")) {
            result = true;
        } else if (token.empty()) {
            TRACE(Trace::Warning, (_T("Security ignored: no token")));

            result = true;
        } else {
            JoinParamsData params;
            params.FromString(parameters);
            const auto& user = params.User.Value();
            const auto& room = params.Room.Value();
            const auto secure = params.Secure.Value();
            const auto& acl = params.Acl;

            if (secure != SecureType::SECURE) {
                result = true;
            } else if (user.empty() || room.empty()) {
                result = true;
            } else {
                _adminLock.Lock();

                auto actualAcl = _roomACL.find(room);
                bool aclSet = actualAcl != _roomACL.end();
                bool settingAcl = acl.IsSet();
                Core::JSON::ArrayType<Core::JSON::String>::ConstIterator index = acl.Elements();
                bool roomExists = _rooms.find(room) != _rooms.end();

                if (!aclSet) {
                    TRACE(Trace::Information, (_T("Joining room '%s' w/o ACL"), room.c_str()));

                    if (!settingAcl || index.Count() == 0) {
                        TRACE(Trace::Error, (_T("ACL is empty")));
                    } else if (roomExists) {
                        TRACE(Trace::Error, (_T("Can't set ACL of an active room '%s'"), room.c_str()));
                    } else if (!IsAllowed(_service, token, _service->Callsign() + ".acl")) {
                        TRACE(Trace::Error, (_T("Not permitted to set ACL")));
                    } else {
                        auto retval = _roomACL.emplace(std::piecewise_construct,
                                                       std::make_tuple(room),
                                                       std::make_tuple());
                        while (index.Next()) {
                            TRACE(Trace::Information,
                                  (_T("Adding '%s' to room '%s' ACL"),
                                          index.Current().Value().c_str(),
                                          room.c_str()));
                            retval.first->second.emplace_back(index.Current().Value());
                        }

                        result = true;
                    }
                } else {
                    TRACE(Trace::Information, (_T("Joining room '%s' w/ ACL"), room.c_str()));

                    if (settingAcl) {
                        TRACE(Trace::Error, (_T("ACL for '%s' already set"), room.c_str()));
                    } else {
                        auto origin = GetUrlOrigin(token);
                        auto it = std::find_if(actualAcl->second.begin(), actualAcl->second.end(),
                                              [&origin](const string &i) {
                                                  return std::regex_search(origin, std::regex(CreateUrlRegex(i)));
                                              });
                        if (it == actualAcl->second.end()) {
                            TRACE(Trace::Warning,
                                  (_T("Origin '%s' doesn't match room '%s' ACL"), origin.c_str(), room.c_str()));
                        } else {
                            TRACE(Trace::Information,
                                  (_T("Origin '%s' matches '%s' in room '%s' ACL"),
                                          origin.c_str(),
                                          it->c_str(),
                                          room.c_str()));

                            result = true;
                        }
                    }
                }

                _adminLock.Unlock();
            }
        }
#ifndef USE_THUNDER_R4
        return result;
#else
        return (result ? PluginHost::JSONRPC::classification::VALID : PluginHost::JSONRPC::classification::INVALID);
#endif
    }

} // namespace Plugin

} // WPEFramework
