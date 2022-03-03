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

project(LocationSync)

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

set(PLUGIN_NAME LocationSync)
set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

set(PLUGIN_LOCATIONSYNC_AUTOSTART "true" CACHE STRING "Automatically start LocationSync plugin")
set(PLUGIN_LOCATIONSYNC_URI "location.webplatformforembedded.org" CACHE STRING "URI to request the location information")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED 
    Module.cpp
    LocationSync.cpp
    LocationService.cpp
    LocationSyncJsonRpc.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

install(TARGETS ${MODULE_NAME} 
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
