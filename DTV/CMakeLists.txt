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

set(PLUGIN_NAME DTV)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

# set(PLUGIN_DTV_AUTOSTART false CACHE STRING true)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(DTVKit REQUIRED)

find_library(LIBNXCLIENT_LIBRARY NAMES nxclient)
message(STATUS "BCM nxclient: ${LIBNXCLIENT_LIBRARY}")
if (LIBNXCLIENT_LIBRARY)
   find_library(PM_LIBRARY NAMES libpmlib.a)
   message(STATUS "BCM power manager: ${PM_LIBRARY}")

   set (TARGET_LIBRARIES ${NXCLIENT_LIBRARY} ${PM_LIBRARY})
else()
   set (TARGET_LIBRARIES)
endif()

add_library(${MODULE_NAME} SHARED 
    DTV.cpp
    DTVJsonRpc.cpp
    Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

find_path(DTVKIT_INCLUDE_DIR NAMES techtype.h HINTS /usr/include/dtvkit/inc)
message(STATUS "DTVKit include: ${DTVKIT_INCLUDE_DIR}")

include_directories(
   ${DTVKIT_INCLUDE_DIR}
   ${DTVKIT_INCLUDE_DIR}/../dvb/inc
   ${DTVKIT_INCLUDE_DIR}/../midware/stb/inc
   ${DTVKIT_INCLUDE_DIR}/../platform/inc)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        -Wl,--start-group ${DTVKIT_LIBRARIES} -Wl,--end-group
        -Wl,--start-group ${TARGET_LIBRARIES} -Wl,--end-group)

install(TARGETS ${MODULE_NAME} 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
