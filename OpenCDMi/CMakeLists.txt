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

set(PLUGIN_NAME OCDM)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_OCDM_IMPLEMENTATION "${MODULE_NAME}Impl" CACHE STRING "Specify a library with a OCDM implementation." )

set(PLUGIN_OPENCDMI_AUTOSTART "true" CACHE STRING "Automatically start OpenCDMi plugin")
set(PLUGIN_OPENCDMI_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")

# deprecated/legacy flags support
if(PLUGIN_OPENCDMI_OOP)
    unset(PLUGIN_OPENCDMI_MODE CACHE)
elseif(PLUGIN_OPENCDMI_OUTOFPROCESS STREQUAL "false")
    set(PLUGIN_OPENCDMI_MODE "Off" CACHE STRING "Process mode"  FORCE)
    unset(PLUGIN_OPENCDMI_OUTOFPROCESS CACHE)
endif()

find_package(ocdm REQUIRED)
find_package(${NAMESPACE}Plugins REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
        OCDM.cpp
        OCDMJsonRpc.cpp
        Module.cpp)

add_library(${PLUGIN_OCDM_IMPLEMENTATION} SHARED
        CENCParser.cpp
        FrameworkRPC.cpp
        Module.cpp)

add_dependencies(${MODULE_NAME} ${PLUGIN_OCDM_IMPLEMENTATION})

# avoid -as-needed flag being set, this will break linking to libocdm.so
string(REPLACE "-Wl,--as-needed" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

set_target_properties(${PLUGIN_OCDM_IMPLEMENTATION} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME}
        PRIVATE
                CompileSettingsDebug::CompileSettingsDebug
                ${NAMESPACE}Plugins::${NAMESPACE}Plugins)
target_link_libraries(${PLUGIN_OCDM_IMPLEMENTATION}
        PRIVATE
                CompileSettingsDebug::CompileSettingsDebug
                ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                ocdm::ocdm)

# Library definition section
install(TARGETS ${MODULE_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

install(TARGETS ${PLUGIN_OCDM_IMPLEMENTATION}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

if("${CMAKE_FIND_ROOT_PATH}" STREQUAL "")
   # Desktop case: not cross compiling
   target_link_libraries(${MODULE_NAME} PRIVATE "${CMAKE_INSTALL_PREFIX}/lib/libocdm.so")
else()
   # Cross compiling: buildroot
   target_link_libraries(${MODULE_NAME} PRIVATE ${PLUGINS_LIBRARIES} ${OCDM_LIBRARIES})
endif()

# Library installation section
string(TOLOWER ${NAMESPACE} STORAGENAME)
install(TARGETS ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGENAME}/plugins)

write_config(${PLUGIN_NAME})
