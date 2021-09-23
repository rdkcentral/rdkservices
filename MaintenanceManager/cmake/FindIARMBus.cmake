# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2021 RDK Management
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

# - Try to find IARMBus
# Once done this will define
#  IARMBUS_FOUND - System has IARMBus
#  IARMBUS_INCLUDE_DIRS - The IARMBus include directories
#  IARMBUS_LIBRARIES - The libraries needed to use IARMBus
#  IARMBUS_FLAGS - The flags needed to use IARMBus
#

find_package(PkgConfig)

find_library(IARMBUS_LIBRARIES NAMES IARMBus)
find_path(IARMBUS_INCLUDE_DIRS NAMES libIARM.h PATH_SUFFIXES rdk/iarmbus)
find_path(IARMIR_INCLUDE_DIRS NAMES irMgr.h PATH_SUFFIXES rdk/iarmmgrs-hal)
find_path(IARMSYS_INCLUDE_DIRS NAMES sysMgr.h PATH_SUFFIXES rdk/iarmmgrs-hal)
find_path(IARMPWR_INCLUDE_DIRS NAMES pwrMgr.h PATH_SUFFIXES rdk/iarmmgrs-hal)


set(IARMBUS_LIBRARIES ${IARMBUS_LIBRARIES} CACHE PATH "Path to IARMBus library")
set(IARMBUS_INCLUDE_DIRS ${IARMBUS_INCLUDE_DIRS} ${IARMIR_INCLUDE_DIRS} ${IARMSYS_INCLUDE_DIRS} ${IARMPWR_INCLUDE_DIRS} )
set(IARMBUS_INCLUDE_DIRS ${IARMBUS_INCLUDE_DIRS} ${IARMIR_INCLUDE_DIRS} ${IARMSYS_INCLUDE_DIRS} ${IARMPWR_INCLUDE_DIRS}  CACHE PATH "Path to IARMBus include")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(IARMBUS DEFAULT_MSG IARMBUS_INCLUDE_DIRS IARMBUS_LIBRARIES)

mark_as_advanced(
    IARMBUS_FOUND
    IARMBUS_INCLUDE_DIRS
    IARMBUS_LIBRARIES
    IARMBUS_LIBRARY_DIRS
    IARMBUS_FLAGS)
