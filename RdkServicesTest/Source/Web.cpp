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

#include "Web.h"

#include "Token.h"

namespace RdkServicesTest {
namespace Fixtures {

Web::Web(const string &callsign)
    : WebLink(1 /*queueSize*/, false,
              WPEFramework::Core::NodeId(), WPEFramework::Core::NodeId(), 256, 1024),
      _callsign(callsign),
      _event(false, true) {}

Web::~Web() {
  Close(WPEFramework::Core::infinite);
}

auto Web::submit(
    WPEFramework::Web::Request::type type,
    const string &url,
    uint32_t connectTimeout,
    uint32_t timeout) -> uint32_t {

  uint32_t result;

  static auto host = thunderAccess();

  static auto token = securityToken();

  auto request = RequestProxy::Create();

  request->Host = host;
  request->Verb = type;
  request->Path = ("/Service/" + _callsign + "/" + url);

  request->WebToken =
      WPEFramework::Web::Authorization(WPEFramework::Web::Authorization::BEARER, token);

  WPEFramework::Core::NodeId remote(
      host.c_str(), WPEFramework::Core::NodeId::TYPE_IPV4);

  _lock.Lock();

  _event.ResetEvent();

  Link().LocalNode(remote.AnyInterface());
  Link().RemoteNode(remote);

  result = Open(connectTimeout);

  if (result == WPEFramework::Core::ERROR_NONE) {
    Submit(request);

    result = _event.Lock(timeout);

    if (result == WPEFramework::Core::ERROR_NONE) {
      if (_errorCode != WPEFramework::Web::STATUS_OK /*200*/) {
        result = WPEFramework::Core::ERROR_BAD_REQUEST;
      }
    }
  } else {
    Close(WPEFramework::Core::infinite);
  }

  _lock.Unlock();

  return result;
}

auto Web::Received(ResponseProxy &element) -> void {
  _errorCode = element->ErrorCode;

  _event.SetEvent();
}

auto Web::thunderAccess() -> string {
  string result;

  WPEFramework::Core::SystemInfo::GetEnvironment(_T("THUNDER_ACCESS"), result);

  return result;
}

} // namespace Fixtures
} // namespace RdkServicesTest
