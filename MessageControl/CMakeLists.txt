# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
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

project(MessageControl)

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

project_version(1.0.0)

set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

message("Setup ${MODULE_NAME} v${PROJECT_VERSION}")

set(PLUGIN_MESSAGECONTROL_AUTOSTART true CACHE STRING "Automatically start plugin")
set(PLUGIN_MESSAGECONTROL_ABBREVIATED true CACHE STRING "Display abbreviated messages")
set(PLUGIN_MESSAGECONTROL_MAX_EXPORTCONNECTIONS 5 CACHE STRING "Maximum allowed export connections")
set(PLUGIN_MESSAGECONTROL_REMOTE "false" CACHE STRING "Remote binding details enabled")
set(PLUGIN_MESSAGECONTROL_PORT "0" CACHE STRING "PORT address")
set(PLUGIN_MESSAGECONTROL_BINDING "0.0.0.0" CACHE STRING "Binding IP Address")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(${NAMESPACE}Messaging REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED 
    MessageControl.cpp
    MessageOutput.cpp
    MessageControlImplementation.cpp
    Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${NAMESPACE}Messaging::${NAMESPACE}Messaging)

install(TARGETS ${MODULE_NAME} 
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config()
