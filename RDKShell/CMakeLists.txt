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

option(PLUGIN_RDKSHELL_READ_MAC_ON_STARTUP "PLUGIN_RDKSHELL_READ_MAC_ON_STARTUP" OFF)

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(IARMBus)
    
add_library(${MODULE_NAME} SHARED
        RDKShell.cpp
        Module.cpp
        ../helpers/UtilsSecurityToken.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

if (PLUGIN_RDKSHELL_READ_MAC_ON_STARTUP)
  add_definitions("-DRDKSHELL_READ_MAC_ON_STARTUP")
  set(PLUGIN_RDKSHELL_EXTRA_LIBRARIES "-lFactory-hal")
endif (PLUGIN_RDKSHELL_READ_MAC_ON_STARTUP)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})

target_include_directories(${MODULE_NAME} PRIVATE ../helpers ${IARMBUS_INCLUDE_DIRS} )

set(RDKSHELL_INCLUDES $ENV{RDKSHELL_INCLUDES})
separate_arguments(RDKSHELL_INCLUDES)
include_directories(BEFORE ${RDKSHELL_INCLUDES})
target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${NAMESPACE}SecurityUtil -lrdkshell ${PLUGIN_RDKSHELL_EXTRA_LIBRARIES} trower-base64)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
