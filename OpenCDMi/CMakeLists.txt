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

project(OCDM)

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

project_version(1.0.0)

set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

message("Setup ${MODULE_NAME} v${PROJECT_VERSION}")

set(PLUGIN_OPENCDMI_AUTOSTART "true" CACHE STRING "Automatically start OpenCDMi plugin")
set(PLUGIN_OPENCDMI_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")

# deprecated/legacy flags support
if(PLUGIN_OPENCDMI_OOP STREQUAL "false")
    set(PLUGIN_OPENCDMI_MODE "Off" CACHE STRING "Process mode"  FORCE)
    unset(PLUGIN_OPENCDMI_OOP CACHE)
elseif(PLUGIN_OPENCDMI_OUTOFPROCESS STREQUAL "false")
    set(PLUGIN_OPENCDMI_MODE "Off" CACHE STRING "Process mode"  FORCE)
    unset(PLUGIN_OPENCDMI_OUTOFPROCESS CACHE)
endif()

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED 
        OCDM.cpp
        OCDMJsonRpc.cpp
        CENCParser.cpp
        FrameworkRPC.cpp
        Module.cpp)

# avoid -as-needed flag being set, this will break linking to libocdm.so
string(REPLACE "-Wl,--as-needed" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-gnu-unique")

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME} 
        PRIVATE
                CompileSettingsDebug::CompileSettingsDebug
                ${NAMESPACE}Plugins::${NAMESPACE}Plugins )
# Library definition section
install(TARGETS ${MODULE_NAME} 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)


# Library installation section
string(TOLOWER ${NAMESPACE} STORAGENAME)
install(TARGETS ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGENAME}/plugins)

write_config()
