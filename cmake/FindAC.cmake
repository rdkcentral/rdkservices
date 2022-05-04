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

# - Try to find Audio Capture Mgr
# Once done this will define
#  AC_FOUND - System has audiocapturemgr
#  AC_INCLUDE_DIRS - The audiocapturemgr include directories
#  AC_LIBRARIES - The libraries needed to use audiocapturemgr
#  AC_FLAGS - The flags needed to use audiocapturemgr
#

find_package(PkgConfig)

find_library(AC_LIBRARIES NAMES audiocapturemgr)
find_path(AC_INCLUDE_DIRS NAMES audiocapturemgr_iarm.h PATH_SUFFIXES audiocapturemgr)

set(AC_LIBRARIES ${AC_LIBRARIES} CACHE PATH "Path to audiocapturemgr library")
set(AC_INCLUDE_DIRS ${AC_INCLUDE_DIRS} )
set(AC_INCLUDE_DIRS ${AC_INCLUDE_DIRS} CACHE PATH "Path to audiocapturemgr include")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(AC DEFAULT_MSG AC_INCLUDE_DIRS AC_LIBRARIES)

mark_as_advanced(
    AC_FOUND
    AC_INCLUDE_DIRS
    AC_LIBRARIES
    AC_LIBRARY_DIRS
    AC_FLAGS)
