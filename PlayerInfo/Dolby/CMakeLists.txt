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

find_package(${NAMESPACE}Definitions REQUIRED)

set(DOLBY_IMPLEMENTATION "NONE" CACHE STRING "Dolby C API implementation type." )

add_library(PlayerInfoDolby STATIC "")

target_include_directories(PlayerInfoDolby
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include)

if(${DOLBY_IMPLEMENTATION} STREQUAL "AMLogic")

    find_library(AUDIO_CLIENT NAMES audio_client)

    if (DOLBY_IMPLEMENTATION_PATH)
        target_sources(PlayerInfoDolby
            PRIVATE
                ${DOLBY_IMPLEMENTATION_PATH}/Implementation.cpp)
    else()
        include(GetExternalCode)
        set(DOLBY_IMPLEMENTATION_VERSION "master" CACHE STRING "dolby implementation version: commit_id_or_tag_or_branch_name")
        set(DOLBY_IMPLEMENTATION_REPOSITORY "https://github.com/WebPlatformForEmbedded/Dolby-AmLogic" CACHE STRING "dolby implementation repository")
        GetExternalCode(
            GIT_REPOSITORY ${DOLBY_IMPLEMENTATION_REPOSITORY}
            GIT_VERSION ${DOLBY_IMPLEMENTATION_VERSION}
            SOURCE_DIR "AMLogic"
        )
        target_sources(PlayerInfoDolby
            PRIVATE
                AMLogic/Implementation.cpp)
    endif()

    target_link_libraries(PlayerInfoDolby
        PRIVATE
            ${AUDIO_CLIENT}
            ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

else()
    message(SEND_ERROR "No DOLBY_IMPLEMENTATION picked. Turn off dolby support or pick one.")
endif()


