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

if (NOT PC_WPE_WEBKIT_FOUND)
pkg_check_modules(PC_WPE_WEBKIT wpe-webkit-1.1)
endif()

if (NOT PC_WPE_WEBKIT_FOUND)
pkg_check_modules(PC_WPE_WEBKIT wpe-webkit-1.0)
endif()

if (NOT PC_WPE_WEBKIT_FOUND)
pkg_check_modules(PC_WPE_WEBKIT wpe-webkit-0.1)
endif()

# Support old, non-versioned pc
if (NOT PC_WPE_WEBKIT_FOUND)
pkg_check_modules(PC_WPE_WEBKIT wpe-webkit)
endif()

if (PC_WPE_WEBKIT_FOUND)
    set(WPE_WEBKIT_FOUND TRUE)
    set(WPE_WEBKIT_VERSION ${PC_WPE_WEBKIT_VERSION})

    list(GET PC_WPE_WEBKIT_LIBRARIES 0 WPE_WEBKIT_LIBRARY_SHORT)
    list(REMOVE_AT PC_WPE_WEBKIT_LIBRARIES 0)
    find_library(WPE_WEBKIT_LIBRARY ${WPE_WEBKIT_LIBRARY_SHORT}
        HINTS ${PC_WPE_WEBKIT_LIBDIR} ${PC_WPE_WEBKIT_LIBRARY_DIRS}
    )
    
    set(WPE_WEBKIT_CFLAGS ${PC_WPE_WEBKIT_CFLAGS})
    list(APPEND WPE_WEBKIT_CFLAGS ${PC_WPE_WEBKIT_CFLAGS_OTHER})

    set(WPE_WEBKIT_NAMES ${PC_WPE_WEBKIT_LIBRARIES})
    foreach (_library ${WPE_WEBKIT_NAMES})
        find_library(WPE_WEBKIT_LIBRARIES_${_library} ${_library}
	        HINTS ${PC_WPE_WEBKIT_LIBDIR} ${PC_WPE_WEBKIT_LIBRARY_DIRS}
        )
        list(APPEND WPE_WEBKIT_LINK_LIBRARIES ${WPE_WEBKIT_LIBRARIES_${_library}})
    endforeach ()

    set(WPE_WEBKIT_INCLUDE_DIRS ${PC_WPE_WEBKIT_INCLUDE_DIRS})

    file(WRITE ${CMAKE_BINARY_DIR}/test_atomic.cpp
            "#include <atomic>\n"
            "int main() { std::atomic<int64_t> i(0); i++; return 0; }\n")
    try_compile(ATOMIC_BUILD_SUCCEEDED ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/test_atomic.cpp)
    if (NOT ATOMIC_BUILD_SUCCEEDED)
        list(APPEND WPE_WEBKIT_LINK_LIBRARIES atomic)
    endif ()
    file(REMOVE ${CMAKE_BINARY_DIR}/test_atomic.cpp)

    pkg_check_modules(PC_WPE_WEBKIT_DEPRECATED_API wpe-webkit-deprecated-0.1)
    if(PC_WPE_WEBKIT_DEPRECATED_API_FOUND)
        find_path( PC_WPE_WEBKIT_DEPRECATED_API_INCLUDE_DIR
            NAMES WebKit.h
            PATH_SUFFIX WPE
            HINTS ${PC_WPE_WEBKIT_DEPRECATED_API_INCLUDEDIR} ${PC_WPE_WEBKIT_DEPRECATED_API_INCLUDE_DIRS})
        list(APPEND WPE_WEBKIT_INCLUDE_DIRS "${PC_WPE_WEBKIT_DEPRECATED_API_INCLUDE_DIR}" "${PC_WPE_WEBKIT_DEPRECATED_API_INCLUDE_DIR}/WPE")
    endif()

    list(REMOVE_DUPLICATES WPE_WEBKIT_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES WPE_WEBKIT_LINK_LIBRARIES)
endif ()

if(WPE_WEBKIT_VERSION STREQUAL "0.0.20170728")
    set(WPE_WEBKIT_DEPRECATED_API TRUE CACHE INTERNAL "" FORCE)
else()
    set(WPE_WEBKIT_DEPRECATED_API FALSE CACHE INTERNAL "" FORCE)
endif()

if(WPE_WEBKIT_VERSION VERSION_GREATER 2.28.0)
    set(WEBKIT_GLIB_API TRUE CACHE INTERNAL "" FORCE)
else()
    set(WEBKIT_GLIB_API FALSE CACHE INTERNAL "" FORCE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WPEWebKit 
    VERSION_VAR WPE_WEBKIT_VERSION
    REQUIRED_VARS WPE_WEBKIT_FOUND WPE_WEBKIT_LIBRARY WPE_WEBKIT_LINK_LIBRARIES WPE_WEBKIT_INCLUDE_DIRS)
mark_as_advanced(WPE_WEBKIT_LINK_LIBRARIES WPE_WEBKIT_CFLAGS WPE_WEBKIT_INCLUDE_DIRS)

if(WPEWebKit_FOUND AND NOT TARGET WPEWebKit::WPEWebKit)
    add_library(WPEWebKit::WPEWebKit SHARED IMPORTED)
    set_target_properties(WPEWebKit::WPEWebKit PROPERTIES
            IMPORTED_LOCATION "${WPE_WEBKIT_LIBRARY}"
            INTERFACE_LINK_LIBRARIES "${WPE_WEBKIT_LINK_LIBRARIES}"
            INTERFACE_COMPILE_OPTIONS "${WPE_WEBKIT_CFLAGS}"
            INTERFACE_INCLUDE_DIRECTORIES "${WPE_WEBKIT_INCLUDE_DIRS}"
    )
endif()
