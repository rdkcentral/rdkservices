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

set(PLUGIN_NAME DisplayInfo)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_DISPLAYINFO_AUTOSTART "true" CACHE BOOL "Automatically start DisplayInfo plugin")
set(PLUGIN_DISPLAYINFO_MODE "Off" CACHE STRING "Set DisplayInfo plugin mode")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(BCM_HOST QUIET)
find_package(NEXUS QUIET)
find_package(NXCLIENT QUIET)
find_package(DRM)
find_path (STAGING_INCDIR glib-2.0)
include_directories(include)
include_directories(${STAGING_INCDIR})
include_directories(${STAGING_INCDIR}/libdrm)

add_library(${MODULE_NAME} SHARED
    DisplayInfo.cpp
    Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})

target_link_libraries(${MODULE_NAME}
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

if (PLUGIN_DISPLAYINFO_BCM_VERSION_MAJOR)
    target_compile_definitions(${MODULE_NAME}
        PRIVATE
            DISPLAYINFO_BCM_VERSION_MAJOR=${PLUGIN_DISPLAYINFO_BCM_VERSION_MAJOR})
endif()

if (USE_DEVICESETTINGS)
    find_package(DS REQUIRED)
    find_package(IARMBus REQUIRED)
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS}
        ${DS_INCLUDE_DIRS}
        ../helpers)
    target_link_libraries(${MODULE_NAME} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${IARMBUS_LIBRARIES}
        ${DS_LIBRARIES})
    target_sources(${MODULE_NAME}
        PRIVATE
            DeviceSettings/PlatformImplementation.cpp)
elseif (NXCLIENT_FOUND AND NEXUS_FOUND)
    target_sources(${MODULE_NAME}
        PRIVATE
            Nexus/PlatformImplementation.cpp)
    target_link_libraries(${MODULE_NAME}
        PRIVATE
            NEXUS::NEXUS
            NXCLIENT::NXCLIENT)
elseif (BCM_HOST_FOUND)
    target_sources(${MODULE_NAME}
        PRIVATE
            RPI/PlatformImplementation.cpp)
    target_link_libraries(${MODULE_NAME}
        PRIVATE
            BCM_HOST::BCM_HOST)
else ()
    message(FATAL_ERROR "There is no graphic backend for display info plugin")
endif ()

if (BUILD_BROADCOM)
    target_sources(${MODULE_NAME}
        PRIVATE
            DeviceSettings/Broadcom/SoC_abstraction.cpp
    )
elseif (BUILD_REALTEK)
    target_sources(${MODULE_NAME}
        PRIVATE
            DeviceSettings/Realtek/SoC_abstraction.cpp
            DeviceSettings/Realtek/kms.c
    )
elseif (BUILD_AMLOGIC)
    target_sources(${MODULE_NAME}
        PRIVATE
	DeviceSettings/Amlogic/SoC_abstraction.cpp
	DeviceSettings/Amlogic/kms.c
    )
endif ()

install(TARGETS ${MODULE_NAME}
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
