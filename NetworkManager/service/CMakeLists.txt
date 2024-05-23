# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2022 RDK Management
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

set(PLUGIN_NAME NetworkManager)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_IMPLEMENTATION ${MODULE_NAME}Implementation)
set(PLUGIN_LEGACY_NETWORK ${NAMESPACE}Network)
set(PLUGIN_LEGACY_WIFI    ${NAMESPACE}WiFiManager)

find_package(${NAMESPACE}Core REQUIRED)
find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CURL)
if(ENABLE_GNOME_NETWORKMANAGER)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(LIBNM REQUIRED libnm)
else()
find_package(IARMBus REQUIRED)
endif ()

message("Setup ${PROJECT_NAME} v${PROJECT_VERSION}")

set(PLUGIN_NETWORKMANAGER_LOGLEVEL "5" CACHE STRING "To configure default loglevel NetworkManager plugin")

# Build the main plugin that runs inside the WPEFramework daemon
add_library(${MODULE_NAME} SHARED
        NetworkManager.cpp
        NetworkManagerJsonRpc.cpp
        NetworkManagerLogger.cpp
        Module.cpp
)

target_link_libraries(${MODULE_NAME}  PRIVATE
                                        ${NAMESPACE}Core::${NAMESPACE}Core
                                        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                                        ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

set_target_properties(${MODULE_NAME} PROPERTIES
                                        CXX_STANDARD 11
                                        CXX_STANDARD_REQUIRED YES)

include_directories(${PROJECT_SOURCE_DIR}/interface)
include_directories(${PROJECT_SOURCE_DIR}/service)
include_directories(${PROJECT_SOURCE_DIR}/../helpers)

install(TARGETS ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)


# Build the implementation that runs out-of-process behind a COM-RPC interface
add_library(${PLUGIN_IMPLEMENTATION} SHARED
        NetworkManagerImplementation.cpp
        NetworkManagerConnectivity.cpp
        NetworkManagerLogger.cpp
        StunClient.cpp
        WifiSignalStrengthMonitor.cpp
        Module.cpp
        ${PUBLIC_HEADERS}
)

if(ENABLE_GNOME_NETWORKMANAGER)
    target_sources(${PLUGIN_IMPLEMENTATION} PRIVATE NetworkManagerGnomeProxy.cpp NetworkManagerGnomeWIFI.cpp)
    target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBNM_INCLUDE_DIRS})
    target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${LIBNM_LIBRARIES})
else()
    target_sources(${PLUGIN_IMPLEMENTATION} PRIVATE NetworkManagerRDKProxy.cpp)
    target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${IARMBUS_LIBRARIES})
endif()

target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

set_target_properties(${PLUGIN_IMPLEMENTATION} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        FRAMEWORK FALSE)

target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${CURL_LIBRARIES})
target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${CURL_INCLUDE_DIRS})

install(TARGETS ${PLUGIN_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

add_library(${PLUGIN_LEGACY_NETWORK} SHARED
        LegacyNetwork.cpp
        NetworkManagerLogger.cpp
        Module.cpp
)

target_link_libraries(${PLUGIN_LEGACY_NETWORK}  PRIVATE
                                        ${NAMESPACE}Core::${NAMESPACE}Core
                                        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                                        ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

set_target_properties(${PLUGIN_LEGACY_NETWORK} PROPERTIES
                                        CXX_STANDARD 11
                                        CXX_STANDARD_REQUIRED YES)

install(TARGETS ${PLUGIN_LEGACY_NETWORK} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

add_library(${PLUGIN_LEGACY_WIFI} SHARED
        LegacyWiFi.cpp
        NetworkManagerLogger.cpp
        Module.cpp
)

target_link_libraries(${PLUGIN_LEGACY_WIFI}  PRIVATE
                                        ${NAMESPACE}Core::${NAMESPACE}Core
                                        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                                        ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

set_target_properties(${PLUGIN_LEGACY_WIFI} PROPERTIES
                                        CXX_STANDARD 11
                                        CXX_STANDARD_REQUIRED YES)

install(TARGETS ${PLUGIN_LEGACY_WIFI} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

write_config()
write_config(PLUGINS Network CLASSNAME Network LOCATOR lib${PLUGIN_LEGACY_NETWORK}.so)
write_config(PLUGINS WiFiManager CLASSNAME WiFiManager LOCATOR lib${PLUGIN_LEGACY_WIFI}.so)

