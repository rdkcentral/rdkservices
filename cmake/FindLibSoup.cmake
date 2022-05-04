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

# - Try to find libsoup
# Once done this will define
#  LIBSOUP_FOUND - System has libsoup
#  LIBSOUP_INCLUDE_DIRS - The libsoup include directories
#  LIBSOUP_LIBRARIES - The libraries needed to use libsoup

find_package(PkgConfig)
pkg_check_modules(PC_LIBSOUP QUIET libsoup-2.4)

find_path(LIBSOUP_INCLUDE_DIRS
    NAMES libsoup/soup.h
    HINTS ${PC_LIBSOUP_INCLUDEDIR} ${PC_LIBSOUP_INCLUDE_DIRS}
)

find_library(LIBSOUP_LIBRARIES
    NAMES soup-2.4
    HINTS ${PC_LIBSOUP_LIBDIR} ${PC_LIBSOUP_LIBRARY_DIRS}
)

mark_as_advanced(LIBSOUP_INCLUDE_DIRS LIBSOUP_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBSOUP REQUIRED_VARS LIBSOUP_INCLUDE_DIRS LIBSOUP_LIBRARIES)
