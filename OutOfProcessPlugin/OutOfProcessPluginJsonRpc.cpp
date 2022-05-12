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

#include "OutOfProcessPlugin.h"
#include "Module.h"

namespace WPEFramework {
namespace Plugin {


// Registration
//
void OutOfProcessPlugin::RegisterAll() {

  Property < Core::JSON::DecUInt32 > (_T("fps"), &OutOfProcessPlugin::get_fps, nullptr, this);
  Property < Core::JSON::DecUInt32 > (_T("getpluginid"), &OutOfProcessPlugin::getpluginid, nullptr, this);
  
}

void OutOfProcessPlugin::UnregisterAll() {
  Unregister(_T("fps"));
  Unregister(_T("getpluginid"));
}

// API implementation
//
// Property: fps
// Return codes:
//  - ERROR_NONE: value
uint32_t OutOfProcessPlugin::get_fps(Core::JSON::DecUInt32 &response) const
{
  response = 32;		/* for testing */
  return Core::ERROR_NONE;
}

// Property: getpluginid
// Return codes:
//  - ERROR_NONE: value
uint32_t OutOfProcessPlugin::getpluginid(Core::JSON::DecUInt32 &response) const
{
  /* send default response*/
  response = 6501; 			/* for testing */
  return Core::ERROR_NONE;
}

}  // namespace Plugin
}  // namespace WPEFramework
