# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# - Try to find WPEWebKit
# Once done this will define
#  WPE_WEBKIT_FOUND - System has WPEFrameworkPlugins
#  WPE_WEBKIT_INCLUDE_DIRS - The WPEFrameworkPlugins include directories
#  WPE_WEBKIT_LIBRARIES - The libraries needed to use WPEFrameworkPlugins
#
# Be extremely careful! WPEFRAMEWORK_PLUGINS_INCLUDE_DIRS and WPE_WEBKIT_LIBRARIES is already defined in
# WPEFramework/Source/plugins!!
# So here we purposely left one underscore away

find_package(PkgConfig)
pkg_check_modules(PC_WPE_WEBKIT wpe-webkit-deprecated-0.1)

if(PC_WPE_WEBKIT_FOUND)
    if(WPE_WEBKIT_FIND_VERSION AND PC_WPE_WEBKIT_VERSION)
        if ("${WPE_WEBKIT_FIND_VERSION}" VERSION_GREATER "${PC_WPE_WEBKIT_VERSION}")
            message(WARNING "Incorrect version, found ${PC_WPE_WEBKIT_VERSION}, need at least ${WPEFRAMEWORK_FIND_VERSION}, please install correct version ${WPE_WEBKIT_FIND_VERSION}")
            set(WPE_WEBKIT_FOUND_TEXT "Found incorrect version")
            unset(PC_WPE_WEBKIT_FOUND)
        endif()
    endif()
else()
    pkg_search_module(PC_WPE_WEBKIT wpe-webkit-deprecated-0.1 wpe-webkit-1.0)
    if(NOT PC_WPE_WEBKIT_FOUND)
        set(WPE_WEBKIT_FOUND_TEXT "Not found")
    endif()
endif()

if(PC_WPE_WEBKIT_FOUND)
    if("${PC_WPE_WEBKIT_VERSION}" STREQUAL "0.0.20170728")
        set(WPE_WEBKIT_DEPRECATED_API TRUE)
    endif()
    if("${PC_WPE_WEBKIT_VERSION}" VERSION_GREATER "2.28")
        set(WEBKIT_GLIB_API TRUE)
    endif()
    find_path(
        WPE_WEBKIT_INCLUDE_DIRS
        NAMES WPE/WebKit.h wpe/webkit.h
        HINTS ${PC_WPE_WEBKIT_INCLUDEDIR} ${PC_WPE_WEBKIT_INCLUDE_DIRS})

    set(WPE_WEBKIT_LIBRARIES )
    foreach(LIB ${PC_WPE_WEBKIT_LIBRARIES})
        find_library(WPE_WEBKIT_LIBRARY_${LIB} NAMES ${LIB}
            HINTS ${PC_WPE_WEBKIT_LIBRARY_DIRS} ${PC_WPE_WEBKIT_LIBDIR})
        list(APPEND WPE_WEBKIT_LIBRARIES ${WPE_WEBKIT_LIBRARY_${LIB}})
    endforeach()

    if("${WPE_WEBKIT_INCLUDE_DIRS}" STREQUAL "" OR "${WPE_WEBKIT_LIBRARIES}" STREQUAL "")
        set(WPE_WEBKIT_FOUND_TEXT "Not found")
    else()
        set(WPE_WEBKIT_FOUND_TEXT "Found")
    endif()
else()
    set(WPE_WEBKIT_FOUND_TEXT "Not found")
endif()

file(WRITE ${CMAKE_BINARY_DIR}/test_atomic.cpp
        "#include <atomic>\n"
        "int main() { std::atomic<int64_t> i(0); i++; return 0; }\n")
try_compile(ATOMIC_BUILD_SUCCEEDED ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/test_atomic.cpp)
if (NOT ATOMIC_BUILD_SUCCEEDED)
    list(APPEND WPE_WEBKIT_LIBRARIES atomic)
endif ()
file(REMOVE ${CMAKE_BINARY_DIR}/test_atomic.cpp)

message(STATUS "WPEWebKit      : ${WPE_WEBKIT_FOUND_TEXT}")
message(STATUS "  version      : ${PC_WPE_WEBKIT_VERSION}")
message(STATUS "  cflags       : ${PC_WPE_WEBKIT_CFLAGS}")
message(STATUS "  cflags other : ${PC_WPE_WEBKIT_CFLAGS_OTHER}")
message(STATUS "  include dirs : ${PC_WPE_WEBKIT_INCLUDE_DIRS} ${PC_WPE_WEBKIT_INCLUDEDIR}")
message(STATUS "  lib dirs     : ${PC_WPE_WEBKIT_LIBRARY_DIRS} ${PC_WPE_WEBKIT_LIBDIR}")
message(STATUS "  include dirs : ${WPE_WEBKIT_INCLUDE_DIRS}")
message(STATUS "  libs         : ${WPE_WEBKIT_LIBRARIES}")

set(WPE_WEBKIT_DEFINITIONS ${PC_WPE_WEBKIT_PLUGINS_CFLAGS_OTHER})
set(WPE_WEBKIT_INCLUDE_DIR ${WPE_WEBKIT_INCLUDE_DIRS})
set(WPE_WEBKIT_LIBRARY ${WPE_WEBKIT_LIBRARIES})
set(WPE_WEBKIT_LIBRARY_DIRS ${PC_WPE_WEBKIT_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WPE_WEBKIT DEFAULT_MSG
    WPE_WEBKIT_LIBRARIES  WPE_WEBKIT_INCLUDE_DIRS)

if(WPE_WEBKIT_FOUND)
else()
    message(WARNING "Could not find WPEFrameworkPlugins")
endif()

mark_as_advanced(WPE_WEBKIT_DEFINITIONS WPE_WEBKIT_INCLUDE_DIRS WPE_WEBKIT_LIBRARIES)
