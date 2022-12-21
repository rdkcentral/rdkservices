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

# double free or corruption (!prev)
# with LLVM
# #15 0x00007fd9fdabd9c7 in std::map<int, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::less<int>, std::allocator<std::pair<int const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >::~map (this=0x7fd9fdb0b0a8 <ErrCodeMap[abi:cxx11]>) at /usr/bin/../lib/gcc/x86_64-linux-gnu/11/../../../../include/c++/11/bits/stl_map.h:302
#16 0x00007fd9fb3bfa56 in __cxa_finalize (d=0x7fd9fce21750) at ./stdlib/cxa_finalize.c:83
#17 0x00007fd9fcc07027 in __do_global_dtors_aux () from libWPEFrameworkSystemServices.so
list(REMOVE_ITEM TESTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_SystemServices.cpp")
message("${TESTS}")

add_executable(${PROJECT_NAME}
        ${TESTS}
        mocks/thunder/Module.cpp
        mocks/Wraps.cpp
        )

set_source_files_properties(
        tests/test_DeviceAudioCapabilities.cpp
        tests/test_DeviceVideoCapabilities.cpp
#        tests/test_SystemServices.cpp
        tests/test_Warehouse.cpp
        PROPERTIES COMPILE_FLAGS "-fexceptions")

include_directories(../LocationSync
        ../PersistentStore
        ../SecurityAgent
        ../DeviceIdentification
        ../DeviceDiagnostics
        ../FrameRate
        ../AVInput
        ../Telemetry
        ../ScreenCapture
        ../DataCapture
        ../UsbAccess
        ../Timer
        ../LoggingPreferences
        ../UserPreferences
        ../Messenger
        ../DeviceInfo
        ../helpers
#        ../SystemServices
        ../HdmiInput
        ../HdcpProfile
        ../TraceControl
        ../Network
	../WifiManager
        ../Warehouse
        ../ActivityMonitor
        )
link_directories(../LocationSync
        ../PersistentStore
        ../SecurityAgent
        ../DeviceIdentification
        ../DeviceDiagnostics
        ../FrameRate
        ../AVInput
        ../Telemetry
        ../ScreenCapture
        ../DataCapture
        ../UsbAccess
        ../Timer
        ../LoggingPreferences
#        ../SystemServices
        ../UserPreferences
        ../Messenger
        ../DeviceInfo
        ../HdmiInput
        ../HdcpProfile
        ../TraceControl
        ../Network
	../WifiManager
        ../Warehouse
        ../ActivityMonitor
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
        ${NAMESPACE}ScreenCapture
        ${NAMESPACE}Telemetry
        ${NAMESPACE}ScreenCapture
        ${NAMESPACE}UsbAccess
        ${NAMESPACE}Timer
        ${NAMESPACE}LoggingPreferences
#        ${NAMESPACE}SystemServices
        ${NAMESPACE}UserPreferences
        ${NAMESPACE}Messenger
        ${NAMESPACE}DeviceInfo
        ${NAMESPACE}HdmiInput
        ${NAMESPACE}HdcpProfile
        ${NAMESPACE}TraceControl
        ${NAMESPACE}Network
        ${NAMESPACE}WifiManager
        ${NAMESPACE}Warehouse
        ${NAMESPACE}ActivityMonitor
        )

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        mocks
        mocks/devicesettings
        mocks/thunder
        )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
