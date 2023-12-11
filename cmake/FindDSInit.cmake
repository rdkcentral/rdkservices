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

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_DSINIT QUIET dsinitmanager)

FIND_LIBRARY(DSINIT_LIBRARIES
    NAMES dsinitmanager
)

message(${DSINIT_LIBRARIES})

FIND_PATH(DSINIT_INCLUDE_DIR
    NAMES dsinitmanager/DSManagerPlugin.h
)
message(${DSINIT_INCLUDE_DIR})

SET(DSINIT_INCLUDE_DIRS ${DSINIT_INCLUDE_DIR})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DSINIT REQUIRED_VARS DSINIT_INCLUDE_DIRS DSINIT_LIBRARIES)
