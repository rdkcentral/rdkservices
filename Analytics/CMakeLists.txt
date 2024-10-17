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
set(VERSION_PATCH 1)

add_compile_definitions(ANALYTICS_MAJOR_VERSION=${VERSION_MAJOR})
add_compile_definitions(ANALYTICS_MINOR_VERSION=${VERSION_MINOR})
add_compile_definitions(ANALYTICS_PATCH_VERSION=${VERSION_PATCH})

set(MODULE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

option(PLUGIN_ANALYTICS_SIFT_BACKEND "Enable Sift backend" OFF)

set(PLUGIN_ANALYTICS_STARTUPORDER "" CACHE STRING "To configure startup order of Analytics plugin")
set(PLUGIN_ANALYTICS_AUTOSTART "false" CACHE STRING "Automatically start Analytics plugin")
set(PLUGIN_ANALYTICS_DEVICE_OS_NAME "rdk" CACHE STRING "Device OS name")

set(PLUGIN_ANALYTICS_SIFT_BACKEND_ENABLED ${PLUGIN_ANALYTICS_SIFT_BACKEND} CACHE BOOL "Enable Sift backend configuration")
set(PLUGIN_ANALYTICS_SIFT_2_0_ENABLED "false" CACHE STRING "Enable Sift 2.0 schema")
set(PLUGIN_ANALYTICS_SIFT_COMMON_SCHEMA "entos/common/v1" CACHE STRING "Sift common schema")
set(PLUGIN_ANALYTICS_SIFT_ENV "prod" CACHE STRING "Sift environment")
set(PLUGIN_ANALYTICS_SIFT_PRODUCT_NAME "entos" CACHE STRING "Sift product name") #entos-immerse in Sift2.0
set(PLUGIN_ANALYTICS_SIFT_LOGGER_NAME "Analytics" CACHE STRING "Sift logger name")
set(PLUGIN_ANALYTICS_SIFT_LOGGER_VERSION "${MODULE_VERSION}" CACHE STRING "Sift logger version")
set(PLUGIN_ANALYTICS_SIFT_PLATFORM_DEFAULT "entos:rdk" CACHE STRING "Sift platform default value")
set(PLUGIN_ANALYTICS_SIFT_MAX_RANDOMISATION_WINDOW_TIME 300 CACHE STRING "Sift max randomisation window time of posting queued events")
set(PLUGIN_ANALYTICS_SIFT_MAX_EVENTS_IN_POST 10 CACHE STRING "Sift max events in post")
set(PLUGIN_ANALYTICS_SIFT_MAX_RETRIES 10 CACHE STRING "Sift max retries posting events")
set(PLUGIN_ANALYTICS_SIFT_MIN_RETRY_PERIOD 1 CACHE STRING "Sift min retry period seconds")
set(PLUGIN_ANALYTICS_SIFT_MAX_RETRY_PERIOD 30 CACHE STRING "Sift max retry period seconds")
set(PLUGIN_ANALYTICS_SIFT_EXPONENTIAL_PERIODIC_FACTOR 2 CACHE STRING "Sift exponential periodic factor")
set(PLUGIN_ANALYTICS_SIFT_STORE_PATH "/opt/persistent/sky/AnalyticsSiftStore" CACHE STRING "Sift store path")
set(PLUGIN_ANALYTICS_SIFT_STORE_EVENTS_LIMIT 1000 CACHE STRING "Sift store events limit")
set(PLUGIN_ANALYTICS_SIFT_URL "" CACHE STRING "Sift URL")

message("Setup ${MODULE_NAME} v${MODULE_VERSION}")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(DS)
find_package(IARMBus)

# -- Build IAnalytics interface --

find_package(WPEFramework REQUIRED)
find_package(${NAMESPACE}Core REQUIRED)
find_package(CompileSettingsDebug REQUIRED)
find_package(ProxyStubGenerator REQUIRED)
find_package(JsonGenerator REQUIRED)

# Interfaces we want to build
set(INTERFACES_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/IAnalytics.h"
)

# Invoke the code generators
if(NOT GENERATOR_SEARCH_PATH)
    set(GENERATOR_SEARCH_PATH ${CMAKE_SYSROOT}${CMAKE_INSTALL_PREFIX}/include/${NAMESPACE})
endif()

message("Using generator search path: ${GENERATOR_SEARCH_PATH}")

ProxyStubGenerator(
    INPUT ${INTERFACES_HEADERS}
    OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
    INCLUDE_PATH ${GENERATOR_SEARCH_PATH}
)

JsonGenerator(
    CODE
    INPUT ${INTERFACES_HEADERS}
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated"
    INCLUDE_PATH ${GENERATOR_SEARCH_PATH}
)

# Find the generated sources
file(GLOB PROXY_STUB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/generated/ProxyStubs*.cpp")
file(GLOB JSON_ENUM_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/generated/JsonEnum*.cpp")
file(GLOB JSON_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/generated/J*.h")

# Build the proxystub lib
add_library(AnalyticsProxyStubs SHARED
    ${PROXY_STUB_SOURCES}
    Module.cpp
)

target_link_libraries(AnalyticsProxyStubs
    PRIVATE
    ${NAMESPACE}Core::${NAMESPACE}Core
    CompileSettingsDebug::CompileSettingsDebug
)

target_include_directories(AnalyticsProxyStubs
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Build the definitions lib if applicable
if(JSON_ENUM_SOURCES)
    add_library(AnalyticsDefinitions SHARED
        ${JSON_ENUM_SOURCES}
    )

    target_link_libraries(AnalyticsDefinitions
        PRIVATE
        ${NAMESPACE}Core::${NAMESPACE}Core
        CompileSettingsDebug::CompileSettingsDebug
    )
endif()

# Install libs & headers
string(TOLOWER ${NAMESPACE} NAMESPACE_LIB)
install(TARGETS AnalyticsProxyStubs
    EXPORT AnalyticsProxyStubsTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${NAMESPACE_LIB}/proxystubs COMPONENT ${NAMESPACE}_Runtime
)

if(JSON_ENUM_SOURCES)
    install(TARGETS AnalyticsDefinitions
        EXPORT AnalyticsDefinitionsTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/ COMPONENT ${NAMESPACE}_Runtime
    )
endif()

install(FILES ${INTERFACES_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${NAMESPACE}/interfaces
)

install(FILES ${JSON_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${NAMESPACE}/interfaces/json
)

# -- end of IAnalytics interface build --

add_library(${MODULE_NAME} SHARED
        Analytics.cpp
        AnalyticsJsonRpc.cpp
        Implementation/AnalyticsImplementation.cpp
        Module.cpp)

target_include_directories(${MODULE_NAME} PRIVATE Implementation)
target_include_directories(${MODULE_NAME} PRIVATE Implementation/SystemTime)
target_include_directories(${MODULE_NAME} PRIVATE ../)
target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

add_subdirectory(Implementation/SystemTime)
add_subdirectory(Implementation/LocalStore)
add_subdirectory(Implementation/Backend)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})

target_link_libraries(${MODULE_NAME}
        PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${DS_LIBRARIES}
        ${IARMBUS_LIBRARIES}
        ${MODULE_NAME}Backends
        ${MODULE_NAME}SystemTime)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
