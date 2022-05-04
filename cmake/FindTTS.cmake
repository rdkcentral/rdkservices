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
#  TTS_INCLUDE_DIRS - The TTS include directories
#  TTS_LIBRARIES - The libraries needed to use TTS
#

find_package(PkgConfig)
find_library(TTS_LIBRARIES NAMES TTSClient)
find_path(TTS_INCLUDE_DIRS NAMES TTSCommon.h)
find_path(TTSC_INCLUDE_DIRS NAMES TTSClient.h)

set(TTS_LIBRARIES ${TTS_LIBRARIES} CACHE PATH "Path to TTSClient")
set(TTS_INCLUDE_DIRS ${TTS_INCLUDE_DIRS} CACHE PATH "Path to TTS include")
set(TTSC_INCLUDE_DIRS ${TTSC_INCLUDE_DIRS} CACHE PATH "Path to TTSClient include")



include(FindPackageHandleStandardArgs)

mark_as_advanced(
    TTS_INCLUDE_DIRS
    TTSC_INCLUDE_DIRS
    TTS_LIBRARIES)
