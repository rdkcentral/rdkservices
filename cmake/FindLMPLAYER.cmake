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
#  LMPLAYER_FOUND - System has audiocapturemgr
#  LMPLAYER_INCLUDE_DIRS - The audiocapturemgr include directories
#  LMPLAYER_LIBRARIES - The libraries needed to use audiocapturemgr
#  LMPLAYER_FLAGS - The flags needed to use audiocapturemgr
#

find_package(PkgConfig)

find_library(LMPLAYER_LIBRARIES NAMES mediaplayer)
find_path(LMPLAYER_INCLUDE_DIRS NAMES libmediaplayer.h)
#find_library(LMPLAYER_LIBRARIES NAMES ds)
#find_path(LMPLAYER_INCLUDE_DIRS NAMES dsTypes.h PATH_SUFFIXES rdk/ds-hal)
message(STATUS "LMPLAYER_LIBRARIES is ${LMPLAYER_LIBRARIES}")
message(STATUS "LMPLAYER_INCLUDE_DIRS is ${LMPLAYER_INCLUDE_DIRS}")

set(LMPLAYER_LIBRARIES ${LMPLAYER_LIBRARIES} CACHE PATH "Path to libmediaplayer library")
set(LMPLAYER_INCLUDE_DIRS ${LMPLAYER_INCLUDE_DIRS} )
set(LMPLAYER_INCLUDE_DIRS ${LMPLAYER_INCLUDE_DIRS} CACHE PATH "Path to libmediaplayer include")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LMPLAYER DEFAULT_MSG LMPLAYER_INCLUDE_DIRS LMPLAYER_LIBRARIES)

mark_as_advanced(
    LMPLAYER_FOUND
    LMPLAYER_INCLUDE_DIRS
    LMPLAYER_LIBRARIES
    LMPLAYER_LIBRARY_DIRS
    LMPLAYER_FLAGS)
