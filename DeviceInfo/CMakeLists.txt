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

set(PLUGIN_NAME DeviceInfo)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(RFC)
find_package(DS)
find_package(IARMBus)

add_library(${MODULE_NAME} SHARED
        DeviceInfo.cpp
        DeviceInfoJsonRpc.cpp
        DeviceCapabilities.cpp
        FirmwareVersion.cpp
        Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_include_directories(${MODULE_NAME} PRIVATE
        ${RFC_INCLUDE_DIRS}
        ${DS_INCLUDE_DIRS}
        ${IARMBUS_INCLUDE_DIRS}
        ../helpers)

set_source_files_properties(DeviceCapabilities.cpp PROPERTIES COMPILE_FLAGS "-fexceptions")

target_link_libraries(${MODULE_NAME}
        PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${RFC_LIBRARIES}
        ${DS_LIBRARIES}
        ${IARMBUS_LIBRARIES})

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
