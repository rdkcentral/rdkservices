# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright Synamedia, All rights reserved
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

set(PLUGIN_NAME UnifiedCASManagement)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
if (USE_THUNDER_R4)
    find_package(${NAMESPACE}COM REQUIRED)
else ()
    find_package(${NAMESPACE}Protocols REQUIRED)
endif (USE_THUNDER_R4)
find_package(${NAMESPACE}Definitions REQUIRED)

add_library(${MODULE_NAME} SHARED
        UnifiedCASManagement.cpp
        UnifiedCASManagementJsonRpc.cpp
        RTPlayer.cpp
        ConnectionManager.cpp
        Module.cpp
        )

add_definitions( -DRT_PLATFORM_LINUX=1 )
set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers)


#find_package(rtRemote)
#find_package(pxCore)

find_path(PXCORE_INCLUDE_DIRS "pxObject.h" PATH_SUFFIXES "pxcore")
target_include_directories(${MODULE_NAME} PRIVATE ${PXCORE_INCLUDE_DIRS})

target_link_libraries(${MODULE_NAME} PUBLIC  ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${NAMESPACE}Definitions::${NAMESPACE}Definitions rtRemote rtCore)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
