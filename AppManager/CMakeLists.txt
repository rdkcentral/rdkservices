# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
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

set(PLUGIN_NAME AppManager)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_IMPLEMENTATION ${MODULE_NAME})

set(PLUGIN_APPMANAGER_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_APPMANAGER_IMPLEMENTATION "${PLUGIN_IMPLEMENTATION}" CACHE STRING "Specify a library with a appmanager implementation." )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
    AppManager.cpp
    AppManagerImplementation.cpp
    RDKShellAdapter.cpp
    RDKShellAdapterUtils.cpp
    WebSocket.cpp
    Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

set(RDKSHELL_INCLUDES $ENV{RDKSHELL_INCLUDES})
separate_arguments(RDKSHELL_INCLUDES)
include_directories(BEFORE ${RDKSHELL_INCLUDES})

target_link_libraries(${MODULE_NAME}
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions -lrdkshell)

install(TARGETS ${MODULE_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

include_directories(
   ../helpers)

write_config(${PLUGIN_NAME})
