/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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
#ifndef MODULE_NAME
#define MODULE_NAME Plugin_PersistentStore
#endif

#include <plugins/plugins.h>
#if (((THUNDER_VERSION_MAJOR >= 4) && (THUNDER_VERSION_MINOR == 4)) || ((THUNDER_VERSION >= 4) && !defined(THUNDER_VERSION_MINOR)))
#include <definitions/definitions.h>
#else
#include <interfaces/definitions.h>
#endif

#define URI_ENV "PERSISTENTSTORE_URI"
#define PATH_ENV "PERSISTENTSTORE_PATH"
#define MAXSIZE_ENV "PERSISTENTSTORE_MAXSIZE"
#define MAXVALUE_ENV "PERSISTENTSTORE_MAXVALUE"
#define LIMIT_ENV "PERSISTENTSTORE_LIMIT"
#define IARM_INIT_NAME "Thunder_Plugins"

#undef EXTERNAL
#define EXTERNAL
