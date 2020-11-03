# - Try to find Broadcom Aamp streamer.
# Once done this will define
#  AAMP_FOUND     - System has a Aamp streamer
#  AAMP::AAMP - The Aamp streamer library
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
