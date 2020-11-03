# - Try to find gstreamer-video-1.0.
# Once done, this will define
#
#  GSTREAMER_VIDEO_FOUND - the gstreamer-video-1.0 is available
#  GStreamerVideo::GStreamerVideo - The gstreamer-video-1.0 library and all its dependecies
#
# Copyright (C) 2019 Metrological B.V
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

find_package(PkgConfig)
pkg_check_modules(PC_GSTREAMER_VIDEO gstreamer-video-1.0)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PC_GSTREAMER_VIDEO DEFAULT_MSG PC_GSTREAMER_VIDEO_FOUND)

mark_as_advanced(PC_GSTREAMER_VIDEO_INCLUDE_DIRS PC_GSTREAMER_VIDEO_LIBRARIES PC_GSTREAMER_VIDEO_LIBRARY_DIRS)

if(${PC_GSTREAMER_VIDEO_FOUND})
    find_library(GSTREAMER_VIDEO_LIBRARY gstvideo-1.0
        HINTS ${PC_GSTREAMER_VIDEO_LIBRARY_DIRS}
    )

    set(GSTREAMER_VIDEO_LIBRARIES ${PC_GSTREAMER_VIDEO_LIBRARIES})
    set(GSTREAMER_VIDEO_INCLUDES ${PC_GSTREAMER_VIDEO_INCLUDE_DIRS})
    set(GSTREAMER_VIDEO_FOUND ${PC_GSTREAMER_VIDEO_FOUND})

    if(NOT TARGET GStreamerVideo::GStreamerVideo)
        add_library(GStreamerVideo::GStreamerVideo UNKNOWN IMPORTED)

        set_target_properties(GStreamerVideo::GStreamerVideo
                PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${GSTREAMER_VIDEO_LIBRARY}"
                INTERFACE_COMPILE_DEFINITIONS "GSTREAMER_VIDEO"
                INTERFACE_COMPILE_OPTIONS "${PC_GSTREAMER_VIDEO_CFLAGS_OTHER}"
                INTERFACE_INCLUDE_DIRECTORIES "${PC_GSTREAMER_VIDEO_INCLUDE_DIRS}"
                INTERFACE_LINK_LIBRARIES "${PC_GSTREAMER_VIDEO_LIBRARIES}"
                )
    endif()
endif()
