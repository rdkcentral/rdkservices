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

project(Packager)

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

set(PLUGIN_NAME Packager)
set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(libprovision QUIET)
find_package(LibOPKG REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
    Module.cpp
    Packager.cpp
    PackagerImplementation.cpp)

target_link_libraries(${MODULE_NAME} PRIVATE
             CompileSettingsDebug::CompileSettingsDebug
             ${NAMESPACE}Plugins::${NAMESPACE}Plugins
             LibOPKG::LibOPKG)

if (libprovision_FOUND)
    target_link_libraries(${MODULE_NAME} PRIVATE libprovision::libprovision)
endif (libprovision_FOUND)

string(TOLOWER ${NAMESPACE} STORAGENAME)
install(TARGETS ${MODULE_NAME} 
    DESTINATION lib/${STORAGENAME}/plugins)

configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/opkg.conf.in"
        "${CMAKE_CURRENT_BINARY_DIR}/opkg.conf"
        @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/opkg.conf
        DESTINATION "/usr/share/${NAMESPACE}/${PLUGIN_NAME}")

write_config(${PLUGIN_NAME})
