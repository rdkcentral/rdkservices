# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
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

find_package(${NAMESPACE}Plugins REQUIRED)

set(PLUGIN_NAME RialtoServerManager)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_RIALTO_SERVER_MANAGER_IMPLEMENTATION "${MODULE_NAME}Impl" CACHE STRING "Specify a library with a RialtoServerManager implementation." )

set(PLUGIN_RIALTO_SERVER_MANAGER_AUTOSTART "false" CACHE STRING "Automatically start RialtoServerManager plugin")
set(PLUGIN_RIALTO_SERVER_MANAGER_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_RIALTO_SERVER_MANAGER_DIR "" CACHE STRING "Home directory of RialtoServer files")
set(PLUGIN_RIALTO_SERVER_MANAGER_HOST_ENVS "" CACHE STRING "Environment variables for RialtoServer host process (WPEFramework process)")
set(PLUGIN_RIALTO_SERVER_MANAGER_SESSION_ENVS "" CACHE STRING "Environment variables for Rialto Server Session process")
set(PLUGIN_RIALTO_SERVER_MANAGER_SESSION_PLATFORM_ENVS "" CACHE STRING "Platform specific environment for Rialto Session process. If not set all Host environment variables will be passed to the session process")


add_library(${MODULE_NAME} SHARED
        RialtoServerManager.cpp
        Module.cpp)
set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)
target_link_libraries(${MODULE_NAME}
        PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

add_library(${PLUGIN_RIALTO_SERVER_MANAGER_IMPLEMENTATION} SHARED
        RialtoServerManagerImpl.cpp
        Module.cpp)

set_target_properties(${PLUGIN_RIALTO_SERVER_MANAGER_IMPLEMENTATION} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${PLUGIN_RIALTO_SERVER_MANAGER_IMPLEMENTATION}
        PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        RialtoServerManager)

add_dependencies(${MODULE_NAME} ${PLUGIN_RIALTO_SERVER_MANAGER_IMPLEMENTATION})

# Library definition section
install(TARGETS ${MODULE_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)
install(TARGETS ${PLUGIN_RIALTO_SERVER_MANAGER_IMPLEMENTATION}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
