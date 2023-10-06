# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
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
project(RdkServicesL2Test)

set(CMAKE_CXX_STANDARD 11)
set(THUNDER_PORT 9998)
set(TEST_COMPLETION_TIMEOUT 600000)
find_package(${NAMESPACE}Plugins REQUIRED)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME}
        L2testController.cpp
        L2testController.h
        Module.cpp
        )

set_source_files_properties(
        L2testController.cpp
        L2testController.h
        Module.cpp
        PROPERTIES COMPILE_FLAGS "-fexceptions")

target_link_libraries(${PROJECT_NAME}
        gmock_main
        ${CMAKE_INSTALL_PREFIX}/lib/libWPEFrameworkCore.so
        ${CMAKE_INSTALL_PREFIX}/lib/libWPEFrameworkProtocols.so
        ${CMAKE_INSTALL_PREFIX}/lib/libWPEFrameworkTracing.so
        )

target_compile_definitions(${PROJECT_NAME}
                             PRIVATE
                             CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                             THUNDER_PORT="${THUNDER_PORT}"
                             TEST_COMPLETION_TIMEOUT=${TEST_COMPLETION_TIMEOUT}
                           )

target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${CMAKE_INSTALL_PREFIX}/include/WPEFramework
        )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
