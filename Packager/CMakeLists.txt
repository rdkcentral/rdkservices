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

set(PLUGIN_NAME Packager)
set(MODULE_NAME WPEFramework${PLUGIN_NAME})

if (BUILD_RDK)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
else (BUILD_RDK)
    find_package(libprovision REQUIRED)
endif (BUILD_RDK)

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(LibOPKG REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
    Module.cpp
    Packager.cpp
    PackagerImplementation.cpp)

if (BUILD_RDK)
    target_include_directories(${MODULE_NAME}
        PRIVATE
            ${LIBOPKG_INCLUDE_DIRS}
            )

    target_link_libraries(${MODULE_NAME}
        PRIVATE
            CompileSettingsDebug::CompileSettingsDebug
            ${NAMESPACE}Plugins::${NAMESPACE}Plugins
            ${LIBOPKG_LIBRARIES}
            )
else (BUILD_RDK)
    target_link_libraries(${MODULE_NAME}
        PRIVATE
            CompileSettingsDebug::CompileSettingsDebug
            ${NAMESPACE}Plugins::${NAMESPACE}Plugins
            libprovision::libprovision
            LibOPKG::LibOPKG
            )
endif (BUILD_RDK)

string(TOLOWER ${NAMESPACE} STORAGENAME)
install(TARGETS ${MODULE_NAME} 
    DESTINATION lib/${STORAGENAME}/plugins)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/opkg.conf.in"
        "${CMAKE_CURRENT_BINARY_DIR}/opkg.conf"
        @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/opkg.conf
        DESTINATION "/usr/share/${NAMESPACE}/${PLUGIN_NAME}")

write_config(${PLUGIN_NAME})
