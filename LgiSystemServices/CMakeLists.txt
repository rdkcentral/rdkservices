# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(PLUGIN_NAME LgiSystemServices)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

if(DEFINED LGISYSTEMSERVICE_DEVICEINFO_FILENAME)
        add_definitions(-DLGISYSTEMSERVICE_DEVICEINFO_FILENAME="${LGISYSTEMSERVICE_DEVICEINFO_FILENAME}")
endif()

if(DEFINED LGISYSTEMSERVICE_DEVICEINFO_ENVVAR_PREFIX)
        add_definitions(-DLGISYSTEMSERVICE_DEVICEINFO_ENVVAR_PREFIX="${LGISYSTEMSERVICE_DEVICEINFO_ENVVAR_PREFIX}")
endif()

set(PLUGIN_LGISYSTEMSERVICE_AUTOSTART "false" CACHE STRING "Automatically start LgiSystemServices plugin")
set(PLUGIN_LGISYSTEMSERVICE_STARTUPORDER "" CACHE STRING "To configure startup order of LgiSystemServices plugin")

find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
        LgiSystemServices.cpp
        SystemServicesHelper.cpp
        Module.cpp
        )

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})

target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins )
target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})

