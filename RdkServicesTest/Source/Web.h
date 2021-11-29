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

typedef WPEFramework::Web::WebLinkType<
    WPEFramework::Core::SocketStream,
    WPEFramework::Web::Response,
    WPEFramework::Web::Request,
    WPEFramework::Core::ProxyPoolType < WPEFramework::Web::Response>> WebLink;

typedef WPEFramework::Core::ProxyType <
    WPEFramework::Web::Request> RequestProxy;

typedef WPEFramework::Core::ProxyType <
    WPEFramework::Web::Response> ResponseProxy;

class Web : public WebLink {
public:
  Web(const string &callsign);

  ~Web();

  template<typename... Args>
  auto get(Args... args) -> uint32_t {
    return submit(WPEFramework::Web::Request::HTTP_GET, args...);
  }

  template<typename... Args>
  auto post(Args... args) -> uint32_t {
    return submit(WPEFramework::Web::Request::HTTP_POST, args...);
  }

  auto submit(
      WPEFramework::Web::Request::type type,
      const string &url,
      uint32_t connectTimeout = 1000,
      uint32_t timeout = 1000) -> uint32_t;

private:
  // WebLinkType methods
  virtual void LinkBody(ResponseProxy &element) override {}
  virtual void Send(const RequestProxy &element) override {}
  virtual void StateChange() override {}
  virtual void Received(ResponseProxy &element) override;

private:
  static auto thunderAccess() -> string;

private:
  string _callsign;
  WPEFramework::Core::Event _event;
  uint16_t _errorCode;
  WPEFramework::Core::CriticalSection _lock;
};

} // namespace Fixtures
} // namespace RdkServicesTest
