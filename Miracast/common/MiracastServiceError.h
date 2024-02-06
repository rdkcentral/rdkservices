/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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
#ifndef _MIRACAST_ERRORS_H_
#define _MIRACAST_ERRORS_H_

#ifndef RETURN_OK
#define RETURN_OK 0
#endif

#ifndef RETURN_ERR
#define RETURN_ERR -1
#endif

namespace MIRACAST
{

    enum MiracastError
    {
        MIRACAST_OK = 0,
        MIRACAST_FAIL,
        MIRACAST_NOT_ENABLED,
        MIRACAST_INVALID_CONFIGURATION,
        MIRACAST_INVALID_P2P_CTRL_IFACE,
        MIRACAST_P2P_INIT_FAILED,
        MIRACAST_CONTROLLER_INIT_FAILED,
        MIRACAST_RTSP_INIT_FAILED
    };
}
#endif
