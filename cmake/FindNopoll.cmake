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

# - Try to find RDK Storage Manager library
# Once done this will define
#  NOPOLL_FOUND - System has Nopoll
#  NOPOLL_LIBRARIES - The libraries needed to use Nopoll

find_package(PkgConfig)

find_path(NOPOLL_INCLUDE_DIRS NAMES nopoll.h PATH_SUFFIXES nopoll)
#find_path(NOPOLL_INCLUDE_DIRS NAMES libIARM.h PATH_SUFFIXES rdk/iarmbus)

find_library(NOPOLL_LIBRARIES NAMES nopoll)

mark_as_advanced(
    NOPOLL_FOUND
    NOPOLL_INCLUDE_DIRS
    NOPOLL_LIBRARIES)
