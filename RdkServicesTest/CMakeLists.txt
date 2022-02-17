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

cmake_minimum_required(VERSION 3.8)
project(RdkServicesTest)

set(CMAKE_CXX_STANDARD 11)

find_package(${NAMESPACE}Protocols REQUIRED)
find_package(${NAMESPACE}SecurityUtil REQUIRED)

find_package(GTest REQUIRED)

file(GLOB TEST_FILES
        ../PersistentStore/PersistentStoreTest.cpp
        ../SecurityAgent/SecurityAgentTest.cpp
        ../LoggingPreferences/LoggingPreferencesTest.cpp
        )

add_executable(${PROJECT_NAME}
        ${TEST_FILES}
        Module.cpp
        Source/Link.cpp
        Source/Web.cpp
        Source/Token.cpp
        )

target_link_libraries(${PROJECT_NAME}
        GTest::GTest GTest::Main
        ${NAMESPACE}Protocols::${NAMESPACE}Protocols
        ${NAMESPACE}SecurityUtil::${NAMESPACE}SecurityUtil
        )

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        Source
        )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
