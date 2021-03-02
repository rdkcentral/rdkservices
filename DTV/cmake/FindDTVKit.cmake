# If not stated otherwise in this file or this component's LICENSE file the
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

# - Try to find DTVKit libraries
# Once done this will define
#  DTVKIT_LIBRARIES - The libraries needed to use DTVKit

find_library(DTV_DVB_LIBRARY NAMES libdvbcore.a)
find_library(DTV_HW_LIBRARY NAMES libdtvkit_hw.a)
find_library(DTV_OS_LIBRARY NAMES libdtvkit_os.a)
find_library(JPEG_LIBRARY NAMES libjpeg.a)
find_library(PNG_LIBRARY NAMES libpng.a)
find_library(OSAL_LIBRARY NAMES librmfosal.a)
find_library(OSALUTILS_LIBRARY NAMES librmfosalutils.a)
find_library(RDKLOG_LIBRARY NAMES librdkloggers.a)
find_library(SQLITE_LIBRARY NAMES libsqlite3.a)

set(DTVKIT_LIBRARIES ${DTV_DVB_LIBRARY}
                     ${DTV_HW_LIBRARY}
                     ${DTV_OS_LIBRARY}
                     ${JPEG_LIBRARY}
                     ${PNG_LIBRARY}
                     ${RDKLOG_LIBRARY}
                     ${OSAL_LIBRARY}
                     ${OSALUTILS_LIBRARY}
                     ${SQLITE_LIBRARY})

message(STATUS "DTVKit libs: ${DTVKIT_LIBRARIES}")

mark_as_advanced(DTVKIT_LIBRARIES)

