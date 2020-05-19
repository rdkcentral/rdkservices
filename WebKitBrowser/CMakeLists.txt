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

set(PLUGIN_WEBKITBROWSER_AUTOSTART false CACHE STRING "Automatically start WebKitBrowser plugin")
set(PLUGIN_WEBKITBROWSER_TRANSPARENT false CACHE STRING "Set transparency")
set(PLUGIN_WEBKITBROWSER_STARTURL "about:blank" CACHE STRING "Initial URL for WebKitBrowser plugin")
set(PLUGIN_WEBKITBROWSER_USERAGENT "Mozilla/5.0 (Macintosh, Intel Mac OS X 10_11_4) AppleWebKit/602.1.28+ (KHTML, like Gecko) Version/9.1 Safari/601.5.17" CACHE STRING "User agent string")
set(PLUGIN_WEBKITBROWSER_MEMORYPROFILE "512m" CACHE STRING "Memory Profile")
set(PLUGIN_WEBKITBROWSER_MEMORYPRESSURE "databaseprocess:50m,networkprocess:100m,webprocess:300m,rpcprocess:50m" CACHE STRING "Memory Pressure")
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
set(PLUGIN_YOUTUBE_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string YouTube")
set(PLUGIN_UX_AUTOSTART false CACHE STRING "Automatically start UX plugin")
set(PLUGIN_UX_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string for UX")
set(PLUGIN_APPS_AUTOSTART false CACHE STRING "Automatically start Apps plugin")
set(PLUGIN_APPS_USERAGENT ${PLUGIN_WEBKITBROWSER_USERAGENT} CACHE STRING "User agent string for Apps")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(GLIB REQUIRED)
find_package(WPEWebKit REQUIRED)
find_package(WPEBackend REQUIRED)

add_library(${MODULE_NAME} SHARED
    Module.cpp
    WebKitBrowser.cpp
    WebKitBrowserJsonRpc.cpp
    WebKitImplementation.cpp
    InjectedBundle/Tags.cpp
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
        ${GLIB_LIBRARIES}
        ${WPE_WEBKIT_LIBRARIES}
        ${SECURITY_TOKEN_LIBRARY})

target_include_directories(${MODULE_NAME}
    PRIVATE
        ${WPE_BACKEND_INCLUDE_DIRS}
        ${WPE_WEBKIT_INCLUDE_DIRS}
        ${WPE_WEBKIT_INCLUDE_DIRS}/WPE
        ${GLIB_INCLUDE_DIRS})

if(WPE_WEBKIT_DEPRECATED_API)
    target_compile_definitions(${MODULE_NAME} PRIVATE WPE_WEBKIT_DEPRECATED_API=${WPE_WEBKIT_DEPRECATED_API})
endif()

install(
    TARGETS ${MODULE_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins
    COMPONENT ${MODULE_NAME})

# also build injected bundle
add_subdirectory(InjectedBundle)

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
endif()

write_config( ${PLUGIN_NAME} )

# youtube configuration
if(PLUGIN_WEBKITBROWSER_YOUTUBE)
    write_config( YouTube )
endif()

# amazon configuration
if(PLUGIN_AMAZON_HAWAII)
    write_config( Amazon )
endif()

# Applications instance configuration
if(PLUGIN_WEBKITBROWSER_APPS)
    write_config( Apps )
endif()

# youtube configuration
if(PLUGIN_WEBKITBROWSER_UX)
    write_config( UX )
endif()
