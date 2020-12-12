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

# - Try to find libopkg.so
# Once done this will define
#  LIBOPKG_FOUND - System has libopkg 
#  LIBOPKG_INCLUDE_DIRS - The libopkg include directories
#  LIBOPKG_LIBRARIES - The libraries needed to use libopkg 
#
find_package(PkgConfig)
pkg_check_modules(PC_LIBOPKG libopkg)

if(PC_LIBOPKG_FOUND)
    if(LIBOPKG_FIND_VERSION AND PC_LIBOPKG_VERSION)
        if ("${LIBOPKG_FIND_VERSION}" VERSION_GREATER "${PC_LIBOPKG_VERSION}")
            message(SEND_ERROR "Incorrect version, found ${PC_LIBOPKG_VERSION}, need at least ${WPEFRAMEWORK_FIND_VERSION}, please install correct version ${LIBOPKG_FIND_VERSION}")
            set(LIBOPKG_FOUND_TEXT "Found incorrect version")
            unset(PC_LIBOPKG_FOUND)
        endif()
    set(LIBOPKG_FOUND TRUE)
    endif()
else()
    set(LIBOPKG_FOUND_TEXT "Not found")
endif()

set(LIBOPKG_DEFINITIONS ${PC_LIBOPKG_CFLAGS_OTHER})
set(LIBOPKG_INCLUDE_DIR ${PC_LIBOPKG_INCLUDE_DIRS})
set(LIBOPKG_LIBRARY ${PC_LIBOPKG_LIBRARIES})
set(LIBOPKG_LIBRARY_DIRS ${PC_LIBOPKG_LIBRARY_DIRS} ${PC_LIBOPKG_LIBDIR})

find_library(OPKG_LIBRARY_LOCATION "${LIBOPKG_LIBRARY}")

mark_as_advanced(LIBOPKG_DEFINITIONS LIBOPKG_INCLUDE_DIRS LIBOPKG_LIBRARIES)

if(NOT TARGET LibOPKG::LibOPKG)
    add_library(LibOPKG::LibOPKG SHARED IMPORTED)
    set_target_properties(LibOPKG::LibOPKG PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${OPKG_LIBRARY_LOCATION}"
            INTERFACE_INCLUDE_DIRECTORIES "${LIBOPKG_INCLUDE_DIR}"
            INTERFACE_COMPILE_DEFINITIONS "${LIBOPKG_DEFINITIONS}"
            INTERFACE_COMPILE_OPTIONS "${PC_LIBOPKG_CFLAGS}"
            IMPORTED_LINK_INTERFACE_LIBRARIES "${LIBOPKG_LIBRARY}"
    )
endif()
