# If not stated otherwise in this file or this component's license file the
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

set(PLUGIN_NAME WifiManager)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_WIFI_STARTUPORDER "" CACHE STRING "To configure startup order of WifiManager plugin")

find_package(${NAMESPACE}Plugins REQUIRED)

find_library(WPA_SUPLICANT_LIBRARIES NAMES wpa_client)

add_library(${MODULE_NAME} SHARED
        WifiManager.cpp
        Module.cpp
        impl/WifiManagerWPS.cpp
        impl/WifiManagerSignalThreshold.cpp
        impl/WifiManagerState.cpp
        impl/WifiManagerConnect.cpp
        impl/WifiManagerScan.cpp
        impl/WifiManagerEvents.cpp)

set_source_files_properties(
        WifiManager.cpp
        impl/WifiManagerConnect.cpp
        impl/WifiManagerScan.cpp
        PROPERTIES COMPILE_FLAGS "-fexceptions")

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS} ../helpers)
target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES})

if(WPA_SUPLICANT_LIBRARIES)
    target_link_libraries(${MODULE_NAME} PRIVATE ${WPA_SUPLICANT_LIBRARIES})
endif()

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
