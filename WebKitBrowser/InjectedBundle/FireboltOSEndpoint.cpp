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

#include "JavaScriptFunctionType.h"
#include "Utils.h"

#ifdef ENABLE_SECURITY_AGENT
#include <securityagent/securityagent.h>
#endif

#include <vector>

namespace WPEFramework {
namespace JavaScript {
namespace Functions {

  class endpoint {
  public:
    endpoint(const endpoint&) = delete;
    endpoint& operator= (const endpoint&) = delete;
    endpoint() = default;
    ~endpoint() = default;

    JSValueRef HandleMessage(
      JSContextRef context, JSObjectRef,
      JSObjectRef, size_t argumentCount,
      const JSValueRef arguments[], JSValueRef*) {

      string thunderAccess;
      if ( !Core::SystemInfo::GetEnvironment(_T("THUNDER_ACCESS"), thunderAccess) )
          return JSValueMakeNull(context);

      string resultString = "ws://" + thunderAccess + "/jsonrpc";
#ifdef ENABLE_SECURITY_AGENT
      string tokenAsString;
      const auto url = WebKit::Utils::GetURL();
      if (!url.empty()) {
          std::vector<uint8_t> buffer( 2 * 1024 );
          uint16_t inputLen = static_cast<uint16_t>(std::min(url.length(), buffer.size()));
          ::memcpy (buffer.data(), url.c_str(), inputLen);
          int outputLen = ::GetToken(buffer.size(), inputLen, buffer.data());
          if (outputLen > 0) {
            tokenAsString = string(reinterpret_cast<const char*>(buffer.data()), outputLen);
          }
      }
      if (!tokenAsString.empty())
          resultString += "?token=" + tokenAsString;
#endif

      JSStringRef returnMessage = JSStringCreateWithUTF8CString(resultString.c_str());
      JSValueRef result = JSValueMakeString(context, returnMessage);
      JSStringRelease(returnMessage);
      return (result);
    }
  };

  static JavaScriptFunctionType<endpoint> _instance(_T("__firebolt"));

}  // Functions
}  // JavaScript
}  // WPEFramework
