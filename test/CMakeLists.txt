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

find_package(${NAMESPACE}Plugins REQUIRED)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

file(GLOB TESTS tests/*.cpp)
add_executable(${PROJECT_NAME}
        ${TESTS}
        source/Module.cpp
        source/Wraps.cpp
        )

include_directories(../LocationSync
        ../PersistentStore
        ../SecurityAgent
        ../DeviceIdentification
        ../DeviceDiagnostics
        ../FrameRate
        ../AVInput
        ../Telemetry
        ../DataCapture
        ../UsbAccess
        ../Timer
        ../helpers
        )
link_directories(../LocationSync
        ../PersistentStore
        ../SecurityAgent
        ../DeviceIdentification
        ../DeviceDiagnostics
        ../FrameRate
        ../AVInput
        ../Telemetry
        ../DataCapture
        ../UsbAccess
        ../Timer
        )

target_link_libraries(${PROJECT_NAME}
        gmock_main
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}DataCapture
        ${NAMESPACE}DeviceDiagnostics
        ${NAMESPACE}LocationSync
        ${NAMESPACE}PersistentStore
        ${NAMESPACE}SecurityAgent
        ${NAMESPACE}DeviceIdentification
        ${NAMESPACE}FrameRate
        ${NAMESPACE}AVInput
        ${NAMESPACE}Telemetry
        ${NAMESPACE}UsbAccess
        ${NAMESPACE}Timer
        )

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        source
        mocks
        )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
