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

project(persistentstorel1test)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

find_package(WPEFramework)
find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)

find_package(PkgConfig)
pkg_search_module(SQLITE REQUIRED sqlite3)

add_executable(${PROJECT_NAME}
        ../Module.cpp
        ../sqlite/Handle.cpp
        ../PersistentStore.cpp
        ../PersistentStoreJsonRpc.cpp
        JsonRpcTest.cpp
        PluginTest.cpp
        StoreTest.cpp
        )

add_definitions(${SQLITE_CFLAGS_OTHER})
link_directories(${SQLITE_LIBRARY_DIRS})

target_include_directories(${PROJECT_NAME} PRIVATE ../)

target_link_libraries(${PROJECT_NAME} PRIVATE
        gmock_main
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${SQLITE_LIBRARIES}
        )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
