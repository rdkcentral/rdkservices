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

#ifndef __MODULE_PLUGIN_OPENCDMI_MODULE_H
#define __MODULE_PLUGIN_OPENCDMI_MODULE_H

#ifndef MODULE_NAME
#define MODULE_NAME Plugin_OCDM
#endif

#include <plugins/plugins.h>


#ifdef USE_THUNDER_R4
#include <interfaces/IOCDM.h>
#else
// Get in the definitions required for access to the OCDM
// counter part living in the applications
#include <ocdm/IOCDM.h>
#include <ocdm/open_cdm.h>
#endif /* USE_THUNDER_R4 */

#undef EXTERNAL
#define EXTERNAL

#endif // __MODULE_PLUGIN_OPENCDMI_MODULE_H
