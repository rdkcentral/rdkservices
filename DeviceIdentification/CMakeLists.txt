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

set(PLUGIN_NAME DeviceIdentification)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

option(CODE_FROM_EXTERNAL_REPOSITORY "Include code from External Repository" ON)
option(PLUGIN_DEVICEIDENTIFICATION_USE_MFR "Get device identification details using MFR library" OFF)

find_package(NEXUS QUIET)
find_package(BCM_HOST QUIET)
find_package(MFRFWLibs QUIET)

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
    DeviceIdentification.cpp
    DeviceIdentificationJsonRpc.cpp
    Module.cpp)

if (PLUGIN_DEVICEIDENTIFICATION_USE_MFR AND MFRFWLIBS_FOUND)
    target_sources(${MODULE_NAME}
        PRIVATE
            Implementation/MFR/MFR.cpp)
    target_link_libraries(${MODULE_NAME}
        PRIVATE
            mfrfwlibs::mfrfwlibs)
elseif (NEXUS_FOUND)
    find_package(NXCLIENT REQUIRED)
    target_link_libraries(${MODULE_NAME} 
        PRIVATE
            NEXUS::NEXUS
            NXCLIENT::NXCLIENT)
    if (DEVICEIDENTIFICATION_IMPLEMENTATION_PATH)
        target_sources(${MODULE_NAME}
            PRIVATE
                ${DEVICEIDENTIFICATION_IMPLEMENTATION_PATH}/Nexus.cpp)
    elseif (CODE_FROM_EXTERNAL_REPOSITORY)
        include(GetExternalCode)
        set(DEVICEIDENTIFICATION_IMPLEMENTATION_VERSION "master" CACHE STRING "DeviceIdentification implementation version")
        GetExternalCode(
            GIT_REPOSITORY "https://code.rdkcentral.com/r/soc/broadcom/components/rdkcentral/thundernanoservices/DeviceIdentification"
            GIT_TAG ${DEVICEIDENTIFICATION_IMPLEMENTATION_VERSION}
            SOURCE_DIR "Implementation/NexusExt"
        )
        target_sources(${MODULE_NAME}
            PRIVATE
                Implementation/NexusExt/Nexus.cpp)
    else()
        target_sources(${MODULE_NAME}
            PRIVATE
                Implementation/Nexus/Nexus.cpp)
    endif()
elseif (BCM_HOST_FOUND)
    target_sources(${MODULE_NAME}
        PRIVATE
            Implementation/RPI/RPI.cpp)
    target_link_libraries(${MODULE_NAME}
        PRIVATE
            BCM_HOST::BCM_HOST)
elseif (BUILD_AMLOGIC)
    target_sources(${MODULE_NAME}
        PRIVATE
            Implementation/Amlogic/Amlogic.cpp)
elseif (BUILD_REALTEK)
	target_sources(${MODULE_NAME}
        PRIVATE
            Implementation/Realtek/Realtek.cpp)
else ()
    message(FATAL_ERROR "There is no platform backend for device identifier plugin")
endif()

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

target_include_directories(${MODULE_NAME} 
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

install(TARGETS ${MODULE_NAME} 
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
