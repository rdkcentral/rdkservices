# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2022 RDK Management
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

project(PersistentStore)

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

project_version(1.0.0)

set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

message("Setup ${MODULE_NAME} v${PROJECT_VERSION}")

set(PLUGIN_PERSISTENTSTORE_MODE "Off" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_PERSISTENTSTORE_PATH "/opt/secure/persistent/rdkservicestore" CACHE STRING "Path")
set(PLUGIN_PERSISTENTSTORE_LEGACYPATH "/opt/persistent/rdkservicestore" CACHE STRING "Previously used path")
set(PLUGIN_PERSISTENTSTORE_KEY "" CACHE STRING "Encryption key")
set(PLUGIN_PERSISTENTSTORE_MAXSIZE "1000000" CACHE STRING "For all text data, in bytes")
set(PLUGIN_PERSISTENTSTORE_MAXVALUE "3000" CACHE STRING "For single text data, in bytes")
set(PLUGIN_PERSISTENTSTORE_LIMIT "10000" CACHE STRING "Default for all text data in namespace, in bytes")
set(PLUGIN_PERSISTENTSTORE_STARTUPORDER "" CACHE STRING "To configure startup order of PersistentStore plugin")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(SQLITE REQUIRED sqlite3)

find_library(IARMBUS_LIBRARIES NAMES IARMBus)

if (IARMBUS_LIBRARIES)
    message("Building with IARMBus")
    add_definitions(-DWITH_CLOCK_SYNC)
    find_path(IARMBUS_INCLUDE_DIRS NAMES libIBus.h PATH_SUFFIXES rdk/iarmbus REQUIRED)
    find_path(IARMSYS_INCLUDE_DIRS NAMES sysMgr.h PATH_SUFFIXES rdk/iarmmgrs/sysmgr REQUIRED)
endif ()

add_library(${MODULE_NAME} SHARED
        PersistentStore.cpp
        PersistentStoreJsonRpc.cpp
        sqlite/Handle.cpp
        Module.cpp
        )

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )

add_definitions(${SQLITE_CFLAGS_OTHER})
link_directories(${SQLITE_LIBRARY_DIRS})

target_include_directories(${MODULE_NAME} PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${SQLITE_INCLUDE_DIRS}
        ${IARMBUS_INCLUDE_DIRS}
        ${IARMSYS_INCLUDE_DIRS}
        )

target_link_libraries(${MODULE_NAME} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${SQLITE_LIBRARIES}
        ${IARMBUS_LIBRARIES}
        )

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config()
