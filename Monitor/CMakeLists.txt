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

project(Monitor)

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

project_version(1.0.0)

set(PLUGIN_NAME ${PROJECT_NAME})
set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

message("Setup ${MODULE_NAME} v${PROJECT_VERSION}")

set(PLUGIN_MONITOR_AUTOSTART "true" CACHE STRING "Automatically start Monitor plugin")
set(PLUGIN_MONITOR_STARTUPORDER "" CACHE STRING "Automatically start Monitor plugin")
# Plugins built from this repository that can be autmatically enabled or enabled manually when built externally
set(PLUGIN_MONITOR_OPENCDMI "${PLUGIN_OPENCDMI}" CACHE BOOL "Enable monitor for the OpenCDMI plugin")
set(PLUGIN_MONITOR_WEBKITBROWSER "${PLUGIN_WEBKITBROWSER}" CACHE BOOL "Enable monitor for the WebKitBrowser plugin")
set(PLUGIN_MONITOR_WEBKITBROWSER_APPS "${PLUGIN_WEBKITBROWSER_APPS}" CACHE BOOL "Enable monitor for the WebKitBrowser Apps plugin")
set(PLUGIN_MONITOR_WEBKITBROWSER_RESIDENT_APP "${PLUGIN_WEBKITBROWSER_RESIDENT_APP}" CACHE BOOL "Enable monitor for the WebKitBrowser Resident App plugin")
set(PLUGIN_MONITOR_WEBKITBROWSER_UX "${PLUGIN_WEBKITBROWSER_UX}" CACHE BOOL "Enable monitor for the WebKitBrowser UX plugin")
set(PLUGIN_MONITOR_WEBKITBROWSER_YOUTUBE "${PLUGIN_WEBKITBROWSER_YOUTUBE}" CACHE BOOL "Enable monitor for the WebKitBrowser Youtube plugin")
set(PLUGIN_MONITOR_SYSTEMAUDIOPLAYER "${PLUGIN_SYSTEMAUDIOPLAYER}" CACHE BOOL "Enable monitor for the SystemAudioPlayer plugin")

# Plugins built from outside of this repository have to be enabled manually
set(PLUGIN_MONITOR_AMAZON OFF CACHE BOOL "Enable monitor for the Amazon plugin")
set(PLUGIN_MONITOR_COBALT OFF CACHE BOOL "Enable monitor for the Cobalt plugin")
set(PLUGIN_MONITOR_NETFLIX OFF CACHE BOOL "Enable monitor for the Netflix plugin")
set(PLUGIN_MONITOR_OUTOFPROCESS OFF CACHE BOOL "Enable monitor for the OutOfProcess plugin")
set(PLUGIN_MONITOR_TESTUTILITY OFF CACHE BOOL "Enable monitor for the TestUtility plugin")

# Options for each plugin
set(PLUGIN_MONITOR_AMAZON_MEMORYLIMIT "614400" CACHE STRING "monitor amazon memory limit")
set(PLUGIN_MONITOR_COBALT_MEMORYLIMIT "614400" CACHE STRING "monitor cobalt memory limit")
set(PLUGIN_MONITOR_NETFLIX_MEMORYLIMIT "614400" CACHE STRING "monitor netflix memory limit")
set(PLUGIN_MONITOR_WEBKITBROWSER_MEMORYLIMIT "614400" CACHE STRING "monitor webkit memory limit")
set(PLUGIN_MONITOR_WEBKITBROWSER_APPS_MEMORYLIMIT "614400" CACHE STRING "monitor apps memory limit")
set(PLUGIN_MONITOR_WEBKITBROWSER_RESIDENT_APP_MEMORYLIMIT "614400" CACHE STRING "monitor resident app memory limit")
set(PLUGIN_MONITOR_WEBKITBROWSER_UX_MEMORYLIMIT "614400" CACHE STRING "monitor ux memory limit")
set(PLUGIN_MONITOR_WEBKITBROWSER_YOUTUBE_MEMORYLIMIT "614400" CACHE STRING "monitor youtube memory limit")

# deprecated/legacy flags support
if(PLUGIN_MONITOR_APPS_MEMORYLIMIT)
    set(PLUGIN_MONITOR_WEBKITBROWSER_APPS_MEMORYLIMIT ${PLUGIN_MONITOR_APPS_MEMORYLIMIT} CACHE STRING "monitor apps memory limit" FORCE)
    unset(PLUGIN_MONITOR_APPS_MEMORYLIMIT CACHE)
endif()
if(PLUGIN_MONITOR_RESIDENT_APP_MEMORYLIMIT)
    set(PLUGIN_MONITOR_WEBKITBROWSER_RESIDENT_APP_MEMORYLIMIT ${PLUGIN_MONITOR_RESIDENT_APP_MEMORYLIMIT} CACHE STRING "monitor resident app memory limit" FORCE)
    unset(PLUGIN_MONITOR_RESIDENT_APP_MEMORYLIMIT CACHE)
endif()
if(PLUGIN_MONITOR_UX_MEMORYLIMIT)
    set(PLUGIN_MONITOR_WEBKITBROWSER_UX_MEMORYLIMIT ${PLUGIN_MONITOR_UX_MEMORYLIMIT} CACHE STRING "monitor ux memory limit" FORCE)
    unset(PLUGIN_MONITOR_UX_MEMORYLIMIT CACHE)
endif()
if(PLUGIN_MONITOR_YOUTUBE_MEMORYLIMIT)
    set(PLUGIN_MONITOR_WEBKITBROWSER_YOUTUBE_MEMORYLIMIT ${PLUGIN_MONITOR_YOUTUBE_MEMORYLIMIT} CACHE STRING "monitor youtube memory limit" FORCE)
    unset(PLUGIN_MONITOR_YOUTUBE_MEMORYLIMIT CACHE)
endif()

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED 
    Monitor.cpp
    MonitorJsonRpc.cpp
    Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

install(TARGETS ${MODULE_NAME} 
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

if(USE_THUNDER_R4)
    write_config()
else()
    write_config(${PROJECT_NAME})
endif(USE_THUNDER_R4)
