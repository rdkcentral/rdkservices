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

cmake_minimum_required(VERSION 3.14)

set(PLUGIN_NAME SharedStorage)
find_package(WPEFramework)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_SHAREDSTORAGE_AUTOSTART "true" CACHE STRING "Automatically start SharedStorage plugin")
set(PLUGIN_SHAREDSTORAGE_STARTUPORDER "51" CACHE STRING "To configure startup order of SharedStorage plugin")

set(CMAKE_CXX_STANDARD 11)

add_library(${MODULE_NAME} SHARED
        SharedStorage.cpp
        SharedStorageJsonRpc.cpp
        Module.cpp
)

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
target_include_directories(${MODULE_NAME} PRIVATE ../helpers)
target_link_libraries(${MODULE_NAME} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
