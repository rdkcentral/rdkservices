/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#include "Link.h"

#include "Token.h"

namespace RdkServicesTest {
namespace Fixtures {

auto LinkMap::get(
    const string &callsign) -> JsonRpcLinkProxy {

  auto retval = links.emplace(
      std::piecewise_construct,
      std::make_tuple(callsign),
      std::make_tuple());

  if (retval.second == true) {
    static auto token = securityToken();
    auto query = ("token=" + token);

    retval.first->second =
        JsonRpcLinkProxy::Create(callsign, nullptr, false, query);
  }

  return retval.first->second;
}

Link::Link(const string &callsign) : _callsign(callsign) {}

auto Link::activate(
    const uint32_t waitTime) -> uint32_t {

  return map.get("")->Invoke<JsonObject, void>(
      waitTime, "activate", JsonObject("{\"callsign\":\"" + _callsign + "\"}"));
}

} // namespace Fixtures
} // namespace RdkServicesTest
