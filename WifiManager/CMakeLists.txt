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
set(PLUGIN_WIFI_IMPL "impl" CACHE STRING "To select the implementation folder")

find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
        WifiManager.cpp
        Module.cpp
        ${PLUGIN_WIFI_IMPL}/WifiManagerWPS.cpp
        ${PLUGIN_WIFI_IMPL}/WifiManagerSignalThreshold.cpp
        ${PLUGIN_WIFI_IMPL}/WifiManagerState.cpp
        ${PLUGIN_WIFI_IMPL}/WifiManagerConnect.cpp
        ${PLUGIN_WIFI_IMPL}/WifiManagerScan.cpp
        ${PLUGIN_WIFI_IMPL}/WifiManagerEvents.cpp)

# for LG variant also add dbus implementation
if(${PLUGIN_WIFI_IMPL} STREQUAL "impl_lg")
        find_package(LibGlib REQUIRED)
        find_package(LibGio REQUIRED)
        file(GLOB DBUS_SOURCES_CPP "impl_lg/dbus/*.cpp")
        file(GLOB DBUS_SOURCES_C "impl_lg/dbus/*.c")
        target_sources(${MODULE_NAME} PRIVATE ${DBUS_SOURCES_CPP} ${DBUS_SOURCES_C})
        include_directories(${LIBGLIB_INCLUDE_DIRS})
        include_directories(${LIBGIO_INCLUDE_DIRS})
        target_link_libraries(${MODULE_NAME}
        PRIVATE
                ${LIBGIO_LIBRARIES}
                ${LIBGLIB_LIBRARIES})
        add_definitions( -DIMPL_LGI )
endif()

set_source_files_properties(
        WifiManager.cpp
        ${PLUGIN_WIFI_IMPL}/WifiManagerConnect.cpp
        ${PLUGIN_WIFI_IMPL}/WifiManagerScan.cpp
        PROPERTIES COMPILE_FLAGS "-fexceptions")

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS} ../helpers ${PLUGIN_WIFI_IMPL})

target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES})


install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
