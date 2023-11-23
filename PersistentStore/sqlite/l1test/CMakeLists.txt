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

project(sqlitel1test)

set(CMAKE_CXX_STANDARD 11)

find_package(WPEFramework)
find_package(${NAMESPACE}Plugins REQUIRED)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

find_package(PkgConfig)
pkg_search_module(SQLITE REQUIRED sqlite3)

add_executable(${PROJECT_NAME}
        ../../Module.cpp
        ../SqliteDb.cpp
        ../SqliteStore2.cpp
        ../SqliteStoreCache.cpp
        ../SqliteStoreInspector.cpp
        SqliteDbTest.cpp
        SqliteStore2Test.cpp
        SqliteStoreCacheTest.cpp
        SqliteStoreInspectorTest.cpp
        SqliteStore2WithReconnectTest.cpp
        SqliteStoreInspectorWithReconnectTest.cpp
        )

add_definitions(${SQLITE_CFLAGS_OTHER})

link_directories(${SQLITE_LIBRARY_DIRS})

target_link_libraries(${PROJECT_NAME}
        gmock_main
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${SQLITE_LIBRARIES}
        )

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        ${SQLITE_INCLUDE_DIRS}
        ../../
        )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
