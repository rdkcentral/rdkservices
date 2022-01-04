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

#pragma once

#include "Module.h"

namespace RdkServicesTest {
namespace Fixtures {

typedef WPEFramework::JSONRPC::LinkType <WPEFramework::Core::JSON::IElement> JsonRpcLink;

typedef WPEFramework::Core::ProxyType <JsonRpcLink> JsonRpcLinkProxy;

class LinkMap {
public:
  LinkMap() = default;

  LinkMap(const LinkMap &) = delete;

  LinkMap &operator=(const LinkMap &) = delete;

public:
  auto get(const string &callsign) -> JsonRpcLinkProxy;

private:
  std::map <string, JsonRpcLinkProxy> links;
};

class Link {
public:
  Link(const string &callsign);

  template<typename PARAMETERS, typename RESPONSE>
  auto invoke(
      const string &method,
      const PARAMETERS &parameters,
      RESPONSE &inbound,
      const uint32_t waitTime = 1000) -> uint32_t {

    return map.get(_callsign)->Invoke(
        waitTime, method, JsonObject(parameters), inbound);
  }

  auto activate(const uint32_t waitTime = 1000) -> uint32_t;

  auto operator->() -> JsonRpcLink* {
    return map.get(_callsign).operator->();
  }

private:
  string _callsign;
  LinkMap map;
};

} // namespace Fixtures
} // namespace RdkServicesTest
