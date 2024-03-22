# If not stated otherwise in this file or this component's LICENSE file the
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

project(Messenger)

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

project_version(1.0.0)

set(PLUGIN_NAME ${PROJECT_NAME})
set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

message("Setup ${MODULE_NAME} v${PROJECT_VERSION}")

set(PLUGIN_MESSENGER_AUTOSTART "true" CACHE STRING "Automatically start Messenger plugin")
set(PLUGIN_MESSENGER_STARTUPORDER "" CACHE STRING "Automatically start Messenger plugin")
set(PLUGIN_MESSENGER_MODE "Off" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")

# deprecated/legacy flags support
if(PLUGIN_MESSENGER_OUTOFPROCESS STREQUAL "false")
    set(PLUGIN_MESSENGER_MODE "Off" CACHE STRING "Process mode"  FORCE)
    unset(PLUGIN_MESSENGER_OUTOFPROCESS CACHE)
endif()

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
    Messenger.cpp
    MessengerJsonRpc.cpp
    MessengerSecurity.cpp
    RoomMaintainer.cpp
    Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME}
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

install(TARGETS ${MODULE_NAME}
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

if(USE_THUNDER_R4)
    write_config()
else()
    write_config(${PROJECT_NAME})
endif(USE_THUNDER_R4)
