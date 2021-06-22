# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_path(AAMP_INCLUDE priv_aamp.h
        PATHS /usr/include/)

find_library(AAMP_LIBRARY aamp)

if(EXISTS "${AAMP_LIBRARY}")
    include(FindPackageHandleStandardArgs)

    set(AAMP_FOUND TRUE)

    find_package_handle_standard_args(AAMP DEFAULT_MSG AAMP_FOUND AAMP_INCLUDE AAMP_LIBRARY)
    mark_as_advanced(AAMP_INCLUDE AAMP_LIBRARY)

    if(NOT TARGET AAMP::AAMP)
        add_library(AAMP::AAMP UNKNOWN IMPORTED)

        set_target_properties(AAMP::AAMP PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${AAMP_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${AAMP_INCLUDE}"
                )
    endif()
endif()
