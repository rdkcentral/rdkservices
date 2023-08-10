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

# - Try to find Display Settings library
# Once done this will define
#  DS_FOUND - System has DS
#  DS_INCLUDE_DIRS - The DS include directories
#  DS_LIBRARIES - The libraries needed to use DS
#  DS_FLAGS - The flags needed to use DS
#

find_package(PkgConfig)

find_library(DS_LIBRARIES NAMES ds)
find_library(DSHAL_LIBRARIES NAMES dshalcli)
find_path(DS_INCLUDE_DIRS NAMES manager.hpp PATH_SUFFIXES rdk/ds)
find_path(DSHAL_INCLUDE_DIRS NAMES dsTypes.h PATH_SUFFIXES rdk/ds-hal)
find_path(DSRPC_INCLUDE_DIRS NAMES dsMgr.h PATH_SUFFIXES rdk/ds-rpc)

set(DS_LIBRARIES ${DS_LIBRARIES} ${DSHAL_LIBRARIES})
set(DS_LIBRARIES ${DS_LIBRARIES} CACHE PATH "Path to DS library")
set(DS_INCLUDE_DIRS ${DS_INCLUDE_DIRS} ${DSHAL_INCLUDE_DIRS} ${DSRPC_INCLUDE_DIRS})
set(DS_INCLUDE_DIRS ${DS_INCLUDE_DIRS} CACHE PATH "Path to DS include")



include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DS DEFAULT_MSG DS_INCLUDE_DIRS DS_LIBRARIES)

mark_as_advanced(
    DS_FOUND
    DS_INCLUDE_DIRS
    DS_LIBRARIES
    DS_LIBRARY_DIRS
    DS_FLAGS)
