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

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake/")

include(CmakeHelperFunctions)

# Library installation section
string(TOLOWER ${NAMESPACE} STORAGE_DIRECTORY)

if(BUILD_REFERENCE)
    add_definitions(-DBUILD_REFERENCE=${BUILD_REFERENCE})
endif()

set(PLUGIN_NAME WebKitBrowser)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

option(PLUGIN_WEBKITBROWSER_YOUTUBE "Include YouTube in seperate plugin." OFF)
option(PLUGIN_WEBKITBROWSER_APPS "Include Apps instance in seperate plugin." OFF)
option(PLUGIN_WEBKITBROWSER_UX "Include UX in seperate plugin." OFF)
option(PLUGIN_WEBKITBROWSER_RESIDENT_APP "Include Resident App in seperate plugin." OFF)
option(PLUGIN_WEBKITBROWSER_SEARCH_AND_DISCOVERY_APP "Include Search&Discovery App in seperate plugin." OFF)
option(PLUGIN_WEBKITBROWSER_HTML_APP "Include HTML App in seperate plugin." OFF)
option(PLUGIN_WEBKITBROWSER_LIGHTNING_APP "Include Lightning App in seperate plugin." OFF)

option(PLUGIN_WEBKITBROWSER_ENABLE_JIT "Enable the use of JIT javascript optimalization" ON)
option(PLUGIN_WEBKITBROWSER_ENABLE_DFG "Enable the use of DFG javascript optimalization" ON)

set(PLUGIN_WEBKITBROWSER_AUTOSTART false CACHE STRING "Automatically start WebKitBrowser plugin")
set(PLUGIN_WEBKITBROWSER_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_WEBKITBROWSER_TRANSPARENT false CACHE STRING "Set transparency")
set(PLUGIN_WEBKITBROWSER_STARTURL "about:blank" CACHE STRING "Initial URL for WebKitBrowser plugin")
set(PLUGIN_WEBKITBROWSER_USERAGENT "" CACHE STRING "User agent string")
set(PLUGIN_WEBKITBROWSER_MEMORYPROFILE "512m" CACHE STRING "Memory Profile")
set(PLUGIN_WEBKITBROWSER_MEMORYPRESSURE "databaseprocess:50m,networkprocess:100m,webprocess:300m,rpcprocess:50m" CACHE STRING "Memory Pressure")
set(PLUGIN_WEBKITBROWSER_MEDIA_CONTENT_TYPES_REQUIRING_HARDWARE_SUPPORT "video/*" CACHE STRING "Media content types requiring hardware support")
set(PLUGIN_WEBKITBROWSER_MEDIADISKCACHE false CACHE STRING "Media Disk Cache")
set(PLUGIN_WEBKITBROWSER_MSEBUFFERS "audio:2m,video:15m,text:1m" CACHE STRING "MSE Buffers for WebKit")
set(PLUGIN_WEBKITBROWSER_DISKCACHE "0" CACHE STRING "Disk Cache")
set(PLUGIN_WEBKITBROWSER_XHRCACHE true CACHE STRING "XHR Cache")
set(PLUGIN_WEBKITBROWSER_LOCALSTORAGE "" CACHE STRING "HTML5 local storage path")
set(PLUGIN_WEBKITBROWSER_COOKIESTORAGE "" CACHE STRING "Browser cookie storage path")
set(PLUGIN_WEBKITBROWSER_WINDOWCLOSE false CACHE STRING "Allow window close")
set(PLUGIN_WEBKITBROWSER_WEBGL true CACHE STRING "Enable WebGL")
set(PLUGIN_WEBKITBROWSER_RESOLUTION "720p" CACHE STRING "Browser resolution")
set(PLUGIN_WEBKITBROWSER_THREADEDPAINTING "1" CACHE STRING "Threads for the Threaded Painting")
set(PLUGIN_WEBKITBROWSER_WEBINSPECTOR_ADDRESS 0.0.0.0:9224 CACHE STRING "IP:Port for WebInspector of WebKitBrowser")
set(PLUGIN_WEBKITBROWSER_LOCALSTORAGE_ENABLE true CACHE STRING "Enable LocalStorage of WebKitBrowser")
set(PLUGIN_WEBKITBROWSER_THUNDER_DECRYPTOR_PREFERENCE true CACHE STRING "Enable Thunder decryptor preference in WebKit")
set(PLUGIN_WEBKITBROWSER_PERSISTENTPATHPOSTFIX "" CACHE STRING "Specify callsign persistent path postfix")

set(PLUGIN_YOUTUBE_AUTOSTART false CACHE STRING "Automatically start Youtube plugin")
set(PLUGIN_YOUTUBE_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_YOUTUBE_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string YouTube")
set(PLUGIN_YOUTUBE_WEBINSPECTOR_ADDRESS 0.0.0.0:9999 CACHE STRING "IP:Port for WebInspector of YouTube")
set(PLUGIN_YOUTUBE_LOCALSTORAGE_ENABLE true CACHE STRING "Enable LocalStorage of YouTube App")

set(PLUGIN_UX_AUTOSTART false CACHE STRING "Automatically start UX plugin")
set(PLUGIN_UX_AUTOSTART "false" CACHE STRING "Automatically start UX plugin")
set(PLUGIN_UX_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string for UX")
set(PLUGIN_UX_WEBINSPECTOR_ADDRESS 0.0.0.0:10000 CACHE STRING "IP:Port for WebInspector of UX")
set(PLUGIN_UX_LOCALSTORAGE_ENABLE true CACHE STRING "Enable LocalStorage of UX")

set(PLUGIN_APPS_AUTOSTART false CACHE STRING "Automatically start Apps plugin")
set(PLUGIN_APPS_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_APPS_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string for Apps")
set(PLUGIN_APPS_WEBINSPECTOR_ADDRESS 0.0.0.0:10001 CACHE STRING "IP:Port for WebInspector of Apps")
set(PLUGIN_APPS_LOCALSTORAGE_ENABLE true CACHE STRING "Enable LocalStorage of Apps")

set(PLUGIN_RESIDENT_APP_AUTOSTART false CACHE STRING "Automatically start Resident App plugin")
set(PLUGIN_RESIDENT_APP_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_RESIDENT_APP_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string for Resident App")
set(PLUGIN_RESIDENT_APP_WEBINSPECTOR_ADDRESS :::10000 CACHE STRING "IP:Port for WebInspector of Resident App")
set(PLUGIN_RESIDENT_APP_STARTURL "about:blank" CACHE STRING "Initial URL for Resident App plugin")
set(PLUGIN_RESIDENT_APP_LOCALSTORAGE_ENABLE true CACHE STRING "Enable LocalStorage of Resident App")
set(PLUGIN_RESIDENT_APP_PERSISTENTPATHPOSTFIX "" CACHE STRING "Specify callsign persistent path postfix")
set(PLUGIN_RESIDENT_APP_MEMORYPRESSURE ${PLUGIN_WEBKITBROWSER_MEMORYPRESSURE} CACHE STRING "Resident App Memory Pressure")

set(PLUGIN_SEARCH_AND_DISCOVERY_APP_AUTOSTART false CACHE STRING "Automatically start Search&Discovery App plugin")
set(PLUGIN_SEARCH_AND_DISCOVERY_APP_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_SEARCH_AND_DISCOVERY_APP_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string for Search&Discovery App")
set(PLUGIN_SEARCH_AND_DISCOVERY_APP_WEBINSPECTOR_ADDRESS :::10003 CACHE STRING "IP:Port for WebInspector of Search&Discovery App")
set(PLUGIN_SEARCH_AND_DISCOVERY_APP_LOCALSTORAGE_ENABLE false CACHE STRING "Enable LocalStorage of Search&Discovery App")
set(PLUGIN_SEARCH_AND_DISCOVERY_APP_PERSISTENTPATHPOSTFIX "" CACHE STRING "Specify callsign persistent path postfix")
set(PLUGIN_SEARCH_AND_DISCOVERY_APP_MEMORYPRESSURE ${PLUGIN_WEBKITBROWSER_MEMORYPRESSURE} CACHE STRING "Search&Discovery Memory Pressure")

set(PLUGIN_HTML_APP_AUTOSTART false CACHE STRING "Automatically start Htmp App plugin")
set(PLUGIN_HTML_APP_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_HTML_APP_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string for Html App")
set(PLUGIN_HTML_APP_WEBINSPECTOR_ADDRESS :::10001 CACHE STRING "IP:Port for WebInspector of Html App")
set(PLUGIN_HTML_APP_LOCALSTORAGE_ENABLE false CACHE STRING "Enable LocalStorage of Html App")
set(PLUGIN_HTML_APP_PERSISTENTPATHPOSTFIX "" CACHE STRING "Specify callsign persistent path postfix")
set(PLUGIN_HTML_APP_MEMORYPRESSURE ${PLUGIN_WEBKITBROWSER_MEMORYPRESSURE} CACHE STRING "Html App Memory Pressure")

set(PLUGIN_LIGHTNING_APP_AUTOSTART false CACHE STRING "Automatically start Lightninig App plugin")
set(PLUGIN_LIGHTNING_APP_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_LIGHTNING_APP_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string for Lightning App")
set(PLUGIN_LIGHTNING_APP_WEBINSPECTOR_ADDRESS :::10002 CACHE STRING "IP:Port for WebInspector of Lightning App")
set(PLUGIN_LIGHTNING_APP_LOCALSTORAGE_ENABLE false CACHE STRING "Enable LocalStorage of Lightning App")
set(PLUGIN_LIGHTNING_APP_PERSISTENTPATHPOSTFIX "" CACHE STRING "Specify callsign persistent path postfix")
set(PLUGIN_LIGHTNING_APP_MEMORYPRESSURE ${PLUGIN_WEBKITBROWSER_MEMORYPRESSURE} CACHE STRING "Lightning App Memory Pressure")

set(PLUGIN_AMAZON_AUTOSTART false CACHE STRING "Automatically start Amazon plugin")
set(PLUGIN_AMAZON_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_AMAZON_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string for Amazon App")
set(PLUGIN_AMAZON_WEBINSPECTOR_ADDRESS 0.0.0.0:9999 CACHE STRING "IP:Port for WebInspector of Amazon")
set(PLUGIN_AMAZON_LOCALSTORAGE_ENABLE true CACHE STRING "Enable LocalStorage of Amazon")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(GLIB REQUIRED)
find_package(WPEWebKit REQUIRED)
find_package(WPEBackend REQUIRED)
if(DEFINED WEBKIT_GLIB_API)
find_package(LibSoup REQUIRED)
endif()

set(SOURCE_LIST
    Module.cpp
    WebKitBrowser.cpp
    WebKitBrowserJsonRpc.cpp
    WebKitImplementation.cpp
    Tags.cpp
)

if(NOT DEFINED WEBKIT_GLIB_API)
    set(SOURCE_LIST
        ${SOURCE_LIST}
        InjectedBundle/Utils.cpp
)
endif()

add_library(${MODULE_NAME} SHARED
    ${SOURCE_LIST}
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME}
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${WPE_BACKEND_LIBRARIES}
        ${LIBSOUP_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${WPE_WEBKIT_LIBRARIES}
        ${SECURITY_TOKEN_LIBRARY})

target_include_directories(${MODULE_NAME}
    PRIVATE
        ${WPE_BACKEND_INCLUDE_DIRS}
        ${WPE_WEBKIT_INCLUDE_DIRS}
        ${WPE_WEBKIT_INCLUDE_DIRS}/WPE
        ${GLIB_INCLUDE_DIRS}
        ${LIBSOUP_INCLUDE_DIRS}
)

if(WPE_WEBKIT_DEPRECATED_API)
    target_compile_definitions(${MODULE_NAME} PRIVATE WPE_WEBKIT_DEPRECATED_API=${WPE_WEBKIT_DEPRECATED_API})
endif()

if(WEBKIT_GLIB_API)
    target_compile_definitions(${MODULE_NAME} PRIVATE WEBKIT_GLIB_API=${WEBKIT_GLIB_API})
endif()

install(
    TARGETS ${MODULE_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins
    COMPONENT ${MODULE_NAME})

if(WEBKIT_GLIB_API)
    add_subdirectory(Extension)
else()
    # also build injected bundle
    add_subdirectory(InjectedBundle)
endif()

# resolution handling
if(PLUGIN_WEBKITBROWSER_RESOLUTION EQUAL "720p")
    set(PLUGIN_WEBKITBROWSER_HEIGHT "720")
    set(PLUGIN_WEBKITBROWSER_WIDTH "1280")
elseif(PLUGIN_WEBKITBROWSER_RESOLUTION EQUAL "1080p")
    set(PLUGIN_WEBKITBROWSER_HEIGHT "1080")
    set(PLUGIN_WEBKITBROWSER_WIDTH "1920")
elseif(PLUGIN_WEBKITBROWSER_RESOLUTION EQUAL "2160p")
    set(PLUGIN_WEBKITBROWSER_HEIGHT "2160")
    set(PLUGIN_WEBKITBROWSER_WIDTH "3840")
elseif(PLUGIN_WEBKITBROWSER_RESOLUTION EQUAL "auto")
    set(PLUGIN_WEBKITBROWSER_HEIGHT "")
    set(PLUGIN_WEBKITBROWSER_WIDTH "")
endif()

write_config( ${PLUGIN_NAME} )

# youtube configuration
if(PLUGIN_WEBKITBROWSER_YOUTUBE)
    write_config( YouTube )
endif()

# amazon configuration
if(PLUGIN_AMAZON_HYBRID)
    write_config( Amazon )
endif()

# Applications instance configuration
if(PLUGIN_WEBKITBROWSER_APPS)
    write_config( Apps )
endif()

# UX configuration
if(PLUGIN_WEBKITBROWSER_UX)
    write_config( UX )
endif()

# Search&Discovery configuration
if(PLUGIN_WEBKITBROWSER_SEARCH_AND_DISCOVERY_APP)
    write_config( SearchAndDiscoveryApp )
endif()

# Html App configuration
if(PLUGIN_WEBKITBROWSER_HTML_APP)
    write_config( HtmlApp )
endif()

# Lightning App configuration
if(PLUGIN_WEBKITBROWSER_LIGHTNING_APP)
    write_config( LightningApp )
endif()

# Resident App configuration
if(PLUGIN_WEBKITBROWSER_RESIDENT_APP)
    write_config( ResidentApp )
endif()
