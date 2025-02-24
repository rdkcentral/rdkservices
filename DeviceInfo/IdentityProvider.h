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
 
#pragma once

extern "C" {
   // The return value returns a pointer, pointing to the system identity. The length 
   // of the identity can be found in the length parameter. If the return value a
   // nullptr, the length parameter is treated as an error code.
   //
   // This function needs to be implemented by specific code for the platform.
   // this code can be found in this plugin under the implementation folder.

   extern const unsigned char* GetIdentity(unsigned char* length_error);
}
