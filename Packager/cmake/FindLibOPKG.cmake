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

# - Try to find OPKG library
# Once done this will define
#  LIBOPKG_FOUND - System has LIBOPKG
#  LIBOPKG_INCLUDE_DIRS - The LIBOPKG include directories
#  LIBOPKG_LIBRARIES - The libraries needed to use LIBOPKG
#

find_package(PkgConfig)

find_library(LIBOPKG_LIBRARIES NAMES opkg)

find_path(LIBOPKG_INCLUDE_DIRS NAMES opkg.h PATH_SUFFIXES libopkg)
find_path(LIBOPKG_INCLUDE_DIRS NAMES opkg_cmd.h PATH_SUFFIXES libopkg)
find_path(LIBOPKG_INCLUDE_DIRS NAMES opkg_download.h PATH_SUFFIXES libopkg)

mark_as_advanced(LIBOPKG_FOUND LIBOPKG_INCLUDE_DIRS LIBOPKG_LIBRARIES)
