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

set(PLUGIN_NAME Analytics)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 4)

add_compile_definitions(ANALYTICS_MAJOR_VERSION=${VERSION_MAJOR})
add_compile_definitions(ANALYTICS_MINOR_VERSION=${VERSION_MINOR})
add_compile_definitions(ANALYTICS_PATCH_VERSION=${VERSION_PATCH})

set(MODULE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(PLUGIN_ANALYTICS_STARTUPORDER "" CACHE STRING "To configure startup order of Analytics plugin")
set(PLUGIN_ANALYTICS_AUTOSTART "false" CACHE STRING "Automatically start Analytics plugin")
set(PLUGIN_ANALYTICS_EVENTS_MAP "" CACHE STRING "Optional path to events mapping file")
set(PLUGIN_ANALYTICS_LOGGER_NAME "${PLUGIN_NAME}" CACHE STRING "Logger name")
set(PLUGIN_ANALYTICS_LOGGER_VERSION "${MODULE_VERSION}" CACHE STRING "Logger version")
set(PLUGIN_ANALYTICS_BACKEND_LIBRARY_NAME "" CACHE STRING "Analytics backend library name")

message("Setup ${MODULE_NAME} v${MODULE_VERSION}")

set(PLUGIN_ANALYTICS_INTERFACES_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/${NAMESPACE}/interfaces/analytics/")
set(PLUGIN_ANALYTICS_LIBLOADER_DFL_DIR "${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins/")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
        Analytics.cpp
        Implementation/AnalyticsImplementation.cpp
        Module.cpp)

target_include_directories(${MODULE_NAME} PRIVATE Implementation)
target_include_directories(${MODULE_NAME} PRIVATE Implementation/SystemTime)
target_include_directories(${MODULE_NAME} PRIVATE Implementation/LocalStore)
target_include_directories(${MODULE_NAME} PRIVATE Implementation/Backend)
target_include_directories(${MODULE_NAME} PRIVATE Implementation/Interfaces)
target_include_directories(${MODULE_NAME} PRIVATE ../)
target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

file(GLOB PLUGIN_ANALYTICS_INTERFACES_HEADERS Implementation/Interfaces/I*.h)
message(STATUS "Installing Analytics interfaces: ${PLUGIN_ANALYTICS_INTERFACES_HEADERS} into ${PLUGIN_ANALYTICS_INTERFACES_INSTALL_DIR}")
install(FILES ${PLUGIN_ANALYTICS_INTERFACES_HEADERS}
        DESTINATION ${PLUGIN_ANALYTICS_INTERFACES_INSTALL_DIR})

add_subdirectory(Implementation/SystemTime)
add_subdirectory(Implementation/LocalStore)
add_subdirectory(Implementation/Backend)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})
target_compile_definitions(${MODULE_NAME} PRIVATE LIBLOADER_DFL_DIR="${PLUGIN_ANALYTICS_LIBLOADER_DFL_DIR}")

target_link_libraries(${MODULE_NAME}
        PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${MODULE_NAME}Backends
        ${MODULE_NAME}SystemTime
        ${MODULE_NAME}LocalStore)

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

    add_subdirectory(Tests/AnalyticsBackendMock)
endif (RDK_SERVICES_L2_TEST)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
