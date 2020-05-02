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

# - Try to find IARMBus
# Once done this will define
#  IARMBUS_FOUND - System has IARMBus
#  IARMBUS_INCLUDE_DIRS - The IARMBus include directories
#  IARMBUS_LIBRARIES - The libraries needed to use IARMBus
#  IARMBUS_FLAGS - The flags needed to use IARMBus
#

find_package(PkgConfig)

find_library(CEC_LIBRARIES NAMES RCEC)
find_library(CEC_IARM_LIBRARIES NAMES RCECIARMBusHal)
find_library(OSAL_LIBRARIES NAMES RCECOSHal)

find_path(CEC_INCLUDE_DIRS NAMES ccec/Connection.hpp PATH_SUFFIXES ccec/include)
find_path(OSAL_INCLUDE_DIRS NAMES osal/Mutex.hpp PATH_SUFFIXES osal/include)
find_path(CEC_HOST_INCLUDE_DIRS NAMES ccec/host/RDK.hpp PATH_SUFFIXES host/include)
find_path(CEC_IARM_INCLUDE_DIRS NAMES ccec/drivers/iarmbus/CecIARMBusMgr.h PATH_SUFFIXES ccec/drivers/include)

set(CEC_LIBRARIES "-Wl,--no-as-needed" ${CEC_LIBRARIES} ${CEC_IARM_LIBRARIES} ${OSAL_LIBRARIES} "-Wl,--as-needed")

set(CEC_LIBRARIES ${CEC_LIBRARIES} CACHE PATH "Path to CEC library")

set(CEC_INCLUDE_DIRS ${CEC_INCLUDE_DIRS} ${OSAL_INCLUDE_DIRS} ${CEC_HOST_INCLUDE_DIRS} ${CEC_IARM_INCLUDE_DIRS})
set(CEC_INCLUDE_DIRS ${CEC_INCLUDE_DIRS} CACHE PATH "Path to CEC include")

include(FindPackageHandleStandardArgs)
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(IARMBUS DEFAULT_MSG IARMBUS_INCLUDE_DIRS IARMBUS_LIBRARIES)

mark_as_advanced(
    CEC_FOUND
    CEC_INCLUDE_DIRS
    CEC_LIBRARIES
    CEC_LIBRARY_DIRS
    CEC_FLAGS)
