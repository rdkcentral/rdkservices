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

set(PLAYER_NAME AampMediaPlayer)
message("Building ${PLAYER_NAME}....")

find_package(Aamp REQUIRED)
find_package(GLIB REQUIRED)
find_package(GStreamer REQUIRED)
find_package(GStreamerVideo REQUIRED)
find_package(${NAMESPACE}Core REQUIRED)

set(LIB_NAME ${PLAYER_NAME})

add_library(${LIB_NAME} STATIC
    AampMediaPlayer.cpp
    AampMediaStream.cpp
    AampEventListener.cpp)

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../
        ${GLIB_INCLUDE_DIRS}
        ${IARMBUS_INCLUDE_DIRS}
        ../../../helpers/)

target_link_libraries(${LIB_NAME}
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        AAMP::AAMP
        GStreamer::GStreamer
        GStreamerVideo::GStreamerVideo
        ${NAMESPACE}Core::${NAMESPACE}Core
        ${GLIB_GIO_LIBRARIES}
        ${GLIB_LIBRARIES})
        