/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright Synamedia, All rights reserved
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

#ifndef UNIFIEDERROR_H
#define UNIFIEDERROR_H

#define RTERROR_TO_WPEERROR(_err)       (_err == RT_OK)? Core::ERROR_NONE: Core::ERROR_GENERAL
#define RTERROR_TO_RESPONSE(_err)                           \
        do {                                                \
            response.Success = (_err == RT_OK);             \
            if (_err != RT_OK) {                            \
                response.Failurereason = (_err);    \
            }                                               \
        } while(0);

#endif /* UNIFIEDERROR_H */

