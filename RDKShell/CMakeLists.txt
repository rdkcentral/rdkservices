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

set(PLUGIN_NAME RDKShell)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_RDKSHELL_AUTOSTART true CACHE STRING "Automatically start RDKShell plugin")
set(PLUGIN_RDKSHELL_EXTRA_LIBRARIES "")
set(PLUGIN_RDKSHELL_STARTUPORDER "" CACHE STRING "Automatically start RDKShell plugin")

option(PLUGIN_RDKSHELL_READ_MAC_ON_STARTUP "PLUGIN_RDKSHELL_READ_MAC_ON_STARTUP" OFF)
option(PLUGIN_HIBERNATESUPPORT "Include hibernate support in the build." OFF)
option(PLUGIN_HIBERNATE_NATIVE_APPS_ON_SUSPENDED "Try to hibernate native apps on suspended." OFF)

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(IARMBus)

set (RDKSHELL_SOURCES)
list(APPEND RDKSHELL_SOURCES RDKShell.cpp)
list(APPEND RDKSHELL_SOURCES Module.cpp)

if (RIALTO_FEATURE)
  add_definitions("-DENABLE_RIALTO_FEATURE")
  list(APPEND RDKSHELL_SOURCES RialtoConnector.cpp)
endif(RIALTO_FEATURE)

add_library(${MODULE_NAME} SHARED ${RDKSHELL_SOURCES})
if(RIALTO_FEATURE)
  target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins libRialtoServerManager.so)
endif(RIALTO_FEATURE)
set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

if (PLUGIN_RDKSHELL_READ_MAC_ON_STARTUP)
  add_definitions("-DRDKSHELL_READ_MAC_ON_STARTUP")
  set(PLUGIN_RDKSHELL_EXTRA_LIBRARIES "-lFactory-hal")
endif (PLUGIN_RDKSHELL_READ_MAC_ON_STARTUP)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})

if(PLUGIN_HIBERNATESUPPORT)
        target_compile_definitions(${MODULE_NAME} PRIVATE HIBERNATE_SUPPORT_ENABLED=1)
endif()

if(PLUGIN_HIBERNATE_NATIVE_APPS_ON_SUSPENDED)
        target_compile_definitions(${MODULE_NAME} PRIVATE HIBERNATE_NATIVE_APPS_ON_SUSPENDED=1)
endif()

target_include_directories(${MODULE_NAME} PRIVATE ../helpers ${IARMBUS_INCLUDE_DIRS} )

set_source_files_properties(RDKShell.cpp PROPERTIES COMPILE_FLAGS "-fexceptions")

set(RDKSHELL_INCLUDES $ENV{RDKSHELL_INCLUDES})
separate_arguments(RDKSHELL_INCLUDES)
include_directories(BEFORE ${RDKSHELL_INCLUDES})
if (NOT RDK_SERVICES_TEST)
        target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins -lrdkshell ${PLUGIN_RDKSHELL_EXTRA_LIBRARIES})
else(RDK_SERVICES_TEST)
        target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${PLUGIN_RDKSHELL_EXTRA_LIBRARIES})
endif(RDK_SERVICES_TEST)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
