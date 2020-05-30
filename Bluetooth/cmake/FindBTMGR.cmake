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

# - Try to find Bluetooth Manager Library
# Once done this will define
#  BTMGR_FOUND - System has DS
#  BTMGR_INCLUDE_DIRS - The DS include directories
#  BTMGR_LIBRARIES - The libraries needed to use DS
#  BTMGR_FLAGS - The flags needed to use DS
#

find_package(PkgConfig)

find_library(BTMGR_LIBRARIES NAMES BTMgr)
find_path(BTMGR_INCLUDE_DIRS NAMES btmgr.h PATH_SUFFIXES rdk/btmgr)

#set(BTMGR_LIBRARIES ${BTMGR_LIBRARIES}
set(BTMGR_LIBRARIES ${BTMGR_LIBRARIES} CACHE PATH "Path to BTMGR library")
#set(BTMGR_INCLUDE_DIRS ${BTMGR_INCLUDE_DIRS}
set(BTMGR_INCLUDE_DIRS ${BTMGR_INCLUDE_DIRS} CACHE PATH "Path to BTMGR include")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BTMGR DEFAULT_MSG BTMGR_INCLUDE_DIRS BTMGR_LIBRARIES)

mark_as_advanced(
        BTMGR_FOUND
        BTMGR_INCLUDE_DIRS
        BTMGR_LIBRARIES
        BTMGR_LIBRARY_DIRS
        BTMGR_FLAGS)
