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

project(SecurityAgent)

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

project_version(1.0.0)

set(PLUGIN_NAME ${PROJECT_NAME})
set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

message("Setup ${MODULE_NAME} v${PROJECT_VERSION}")

set(PLUGIN_SECURITYAGENT_STARTUPORDER "" CACHE STRING "To configure startup order of SecurityAgent plugin")
set(PLUGIN_SECURITYAGENT_ACL_FILE_NAME "acl.json" CACHE STRING "SecurityAgent ACL file name")
set(PLUGIN_SECURITYAGENT_DAC_FOLDER "" CACHE STRING "Folder with ACL files for DAC apps")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

find_library(SECAPI NAMES ${SECAPI_LIB})
if (SECAPI)
    set(SECAPI_RELATED_SOURCES SecapiToken.cpp)
endif()

add_library(${MODULE_NAME} SHARED 
    AccessControlList.cpp
    SecurityAgent.cpp
    SecurityContext.cpp
    SecurityAgentJsonRpc.cpp
    ${SECAPI_RELATED_SOURCES}
    Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

if (SECAPI)
    target_link_libraries(${MODULE_NAME} PRIVATE ${SECAPI_LIB})
    add_definitions(-DENABLE_SECAPI)
endif()

install(TARGETS ${MODULE_NAME} 
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

if(USE_THUNDER_R4)
    write_config()
else()
    write_config(${PROJECT_NAME})
endif(USE_THUNDER_R4)
