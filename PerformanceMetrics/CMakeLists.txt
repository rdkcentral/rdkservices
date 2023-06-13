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

project(PerformanceMetrics)

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

project_version(1.0.0)

set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

message("Setup ${MODULE_NAME} v${PROJECT_VERSION}")

set(PLUGIN_PERFORMANCEMETRICS_AUTOSTART "true" CACHE STRING "Automatically start Performance Metrics plugin")
set(PLUGIN_PERFORMANCEMETRICS_LOGGER_IMPLEMENTATION "TRACING" CACHE STRING "Defines what implementation to use for logging the Performance Metrics")

# Plugins built from this repository that can be autmatically enabled or enabled manually when built externally
set(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER "${PLUGIN_WEBKITBROWSER}" CACHE BOOL "Enable monitor for the Performance Metrics plugin")
set(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER_APPS "${PLUGIN_WEBKITBROWSER_APPS}" CACHE BOOL "Enable monitor for the Performance Metrics Apps plugin")
set(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER_RESIDENT_APP "${PLUGIN_WEBKITBROWSER_RESIDENT_APP}" CACHE BOOL "Enable monitor for the Performance Metrics Resident App plugin")
set(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER_UX "${PLUGIN_WEBKITBROWSER_UX}" CACHE BOOL "Enable monitor for the Performance Metrics UX plugin")
set(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER_YOUTUBE "${PLUGIN_WEBKITBROWSER_YOUTUBE}" CACHE BOOL "Enable monitor for the Performance Metrics Youtube plugin")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED 
    PerformanceMetrics.cpp
    Module.cpp)

if (PLUGIN_PERFORMANCEMETRICS_LOGGER_IMPLEMENTATION STREQUAL "TRACING")
    message(STATUS "Outputting PerformanceMetrics to Tracing")
    target_sources(${MODULE_NAME}
        PRIVATE
            TraceOutput.cpp)
elseif (PLUGIN_PERFORMANCEMETRICS_LOGGER_IMPLEMENTATION STREQUAL "SYSLOG")
    message(STATUS "Outputting PerformanceMetrics to Syslog")
    target_sources(${MODULE_NAME}
        PRIVATE
            SyslogOutput.cpp)
else()
    message(FATAL_ERROR "There is no output implementation specified for the Performance Metrics plugin")
endif()

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

install(TARGETS ${MODULE_NAME} 
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

if(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER)
    write_config( PLUGINS PerfMetricsWebKitBrowser )
endif()

if(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER_APPS)
    write_config( PLUGINS PerfMetricsApps )
endif()

if(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER_RESIDENT_APP)
    write_config( PLUGINS PerfMetricsResidentApp )
endif()

if(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER_UX)
    write_config( PLUGINS PerfMetricsUX )
endif()

if(PLUGIN_PERFORMANCEMETRICS_WEBKITBROWSER_YOUTUBE)
    write_config( PLUGINS PerfMetricsYouTube )
endif()


