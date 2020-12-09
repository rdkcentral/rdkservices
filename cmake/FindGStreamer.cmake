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

find_package(PkgConfig)
pkg_check_modules(PC_GSTREAMER gstreamer-1.0)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PC_GSTREAMER DEFAULT_MSG PC_GSTREAMER_FOUND)

mark_as_advanced(PC_GSTREAMER_INCLUDE_DIRS PC_GSTREAMER_LIBRARIES PC_GSTREAMER_LIBRARY_DIRS)

if(${PC_GSTREAMER_FOUND})
    find_library(GSTREAMER_LIBRARY gstreamer-1.0
        HINTS ${PC_GSTREAMER_LIBRARY_DIRS}
    )

    set(GSTREAMER_LIBRARIES ${PC_GSTREAMER_LIBRARIES})
    set(GSTREAMER_INCLUDES ${PC_GSTREAMER_INCLUDE_DIRS})
    set(GSTREAMER_FOUND ${PC_GSTREAMER_FOUND})

    if(NOT TARGET GStreamer::GStreamer)
        add_library(GStreamer::GStreamer UNKNOWN IMPORTED)

        set_target_properties(GStreamer::GStreamer
                PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${GSTREAMER_LIBRARY}"
                INTERFACE_COMPILE_DEFINITIONS "GSTREAMER"
                INTERFACE_COMPILE_OPTIONS "${PC_GSTREAMER_CFLAGS_OTHER}"
                INTERFACE_INCLUDE_DIRECTORIES "${PC_GSTREAMER_INCLUDE_DIRS}"
                INTERFACE_LINK_LIBRARIES "${PC_GSTREAMER_LIBRARIES}"
                )
    endif()
endif()
