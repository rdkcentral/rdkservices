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
        mocks/thunder/Module.cpp
        mocks/Wraps.cpp
        mocks/Dobby.cpp
	mocks/rdkshell.cpp
        )

set_source_files_properties(
        tests/test_DeviceAudioCapabilities.cpp
        tests/test_DeviceVideoCapabilities.cpp
        tests/test_SystemServices.cpp
        tests/test_Warehouse.cpp
        tests/test_HdmiCecSink.cpp
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
        ../SystemServices
        ../HdmiInput
        ../HdcpProfile
        ../Network
	../WifiManager
        ../TraceControl
        ../Warehouse
        ../ActivityMonitor
        ../MotionDetection
        ../CompositeInput
        ../OCIContainer
        ../HdmiCecSink
        ../VoiceControl
        ../ControlService
        ../RemoteActionMapping
	../RDKShell
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
        ../SystemServices
        ../UserPreferences
        ../Messenger
        ../DeviceInfo
        ../HdmiInput
        ../HdcpProfile
        ../Network
	../WifiManager
        ../TraceControl
        ../Warehouse
        ../ActivityMonitor
        ../MotionDetection
        ../CompositeInput
        ../OCIContainer
        ../HdmiCecSink
        ../VoiceControl
        ../ControlService
        ../RemoteActionMapping
	../RDKShell
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
        ${NAMESPACE}UsbAccess
        ${NAMESPACE}Timer
        ${NAMESPACE}LoggingPreferences
        ${NAMESPACE}SystemServices
        ${NAMESPACE}UserPreferences
        ${NAMESPACE}Messenger
        ${NAMESPACE}DeviceInfo
        ${NAMESPACE}HdmiInput
        ${NAMESPACE}HdcpProfile
        ${NAMESPACE}Network
        ${NAMESPACE}WifiManager
        ${NAMESPACE}TraceControl
        ${NAMESPACE}Warehouse
        ${NAMESPACE}ActivityMonitor
        ${NAMESPACE}MotionDetection
        ${NAMESPACE}CompositeInput
        ${NAMESPACE}OCIContainer
        ${NAMESPACE}HdmiCecSink
        ${NAMESPACE}VoiceControl
        ${NAMESPACE}ControlService
        ${NAMESPACE}RemoteActionMapping
	${NAMESPACE}RDKShell
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
