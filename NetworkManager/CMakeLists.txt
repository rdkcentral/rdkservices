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


set(PLUGIN_NAME NetworkManagerPlugin)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_IMPLEMENTATION ${MODULE_NAME}Implementation)

find_package(${NAMESPACE}Plugins REQUIRED)

# Build two libraries to make it obvious what is running in and out of process
# https://wiki.rdkcentral.com/pages/viewpage.action?pageId=198262351

# Build the main plugin that runs inside the WPEFramework daemon
add_library(${MODULE_NAME} SHARED
        NetworkManagerPlugin.cpp
        NetworkManagerPluginJsonRpc.cpp
        Module.cpp
)

target_link_libraries(${MODULE_NAME}
        PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
)

install(TARGETS
        ${MODULE_NAME}
        DESTINATION
        ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins
)

set(PUBLIC_HEADERS
        adapter/nm_adapter.h
        adapter/iarm/iarm_adapter.h
        )

# Build the implementation that runs out-of-process behind a COM-RPC interface
add_library(${PLUGIN_IMPLEMENTATION} SHARED
#       adapter/iarm/iarm_adapter.cpp
        NetworkManagerPluginImplementation.cpp
        Module.cpp
        ${PUBLIC_HEADERS}
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

find_package(IARMBus)

add_definitions (-DNET_DEFINED_INTERFACES_ONLY)
target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${IARMBUS_INCLUDE_DIRS})
target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ../helpers)

target_link_libraries(${PLUGIN_IMPLEMENTATION}
        PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${IARMBUS_LIBRARIES}
)

install(TARGETS
        ${PLUGIN_IMPLEMENTATION}
        DESTINATION
        ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)


write_config(${PLUGIN_NAME})
