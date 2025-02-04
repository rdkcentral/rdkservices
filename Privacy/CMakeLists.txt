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

set(PLUGIN_NAME Privacy)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

add_compile_definitions(PRIVACY_MAJOR_VERSION=${VERSION_MAJOR})
add_compile_definitions(PRIVACY_MINOR_VERSION=${VERSION_MINOR})
add_compile_definitions(PRIVACY_PATCH_VERSION=${VERSION_PATCH})

set(MODULE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(PLUGIN_PRIVACY_STARTUPORDER "" CACHE STRING "To configure startup order of Privacy plugin")
set(PLUGIN_PRIVACY_AUTOSTART "false" CACHE STRING "Automatically start Privacy plugin")
set(PLUGIN_PRIVACY_ESS_URL "https://example-ess.com" CACHE STRING "ESS URL")
set(PLUGIN_PRIVACY_ESS_SETTINGS_BASE_RETRY_BACKOFF_TIME_SEC 60 CACHE STRING "ESS settings base retry backoff time in seconds")
set(PLUGIN_PRIVACY_ESS_SETTINGS_RETRY_LIMIL 5 CACHE STRING "ESS settings retry limit")
set(PLUGIN_PRIVACY_ESS_SETTINGS_SYNC_AFTER_BOOT_MAX_DELAY_SEC 60 CACHE STRING "ESS settings sync after boot max delay in seconds")
set(PLUGIN_PRIVACY_ESS_SETTINGS_SYNC_AFTER_LINCHPIN_DELAY_SEC 2 CACHE STRING "ESS settings sync after linchpin delay in seconds")

message("Setup ${MODULE_NAME} v${MODULE_VERSION}")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
        Privacy.cpp
        Implementation/PrivacyImplementation.cpp
        Module.cpp)

target_include_directories(${MODULE_NAME} PRIVATE Implementation)
target_include_directories(${MODULE_NAME} PRIVATE ../)
target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})

target_link_libraries(${MODULE_NAME}
        PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

if (RDK_SERVICE_L2_TEST)
    target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})
    target_compile_options(${MODULE_NAME} PRIVATE -Wno-error)

    find_library(TESTMOCKLIB_LIBRARIES NAMES TestMocklib)
    if (TESTMOCKLIB_LIBRARIES)
        message ("linking mock libraries ${TESTMOCKLIB_LIBRARIES} library")
        target_link_libraries(${MODULE_NAME} PRIVATE ${TESTMOCKLIB_LIBRARIES})
    else (TESTMOCKLIB_LIBRARIES)
        message ("Require ${TESTMOCKLIB_LIBRARIES} library")
    endif (TESTMOCKLIB_LIBRARIES)
endif (RDK_SERVICES_L2_TEST)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
