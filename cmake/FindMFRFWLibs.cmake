# - Try to find MFRFWLIBS
# Once done this will define
#  MFRFWLIBS_FOUND - System has mfrfwlibs
#  MFRFWLIBS_INCLUDE_DIRS - The mfrfwlibs include directories
#  MFRFWLIBS_LIBRARIES - The libraries needed to use mfrfwlibs
#
# Copyright (C) 2019 Metrological.
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
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE

find_package(PkgConfig)
pkg_check_modules(MFRFWLIBS fwupgrade)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(mfrfwlibs DEFAULT_MSG MFRFWLIBS_LIBRARIES)

mark_as_advanced(MFRFWLIBS_INCLUDE_DIRS MFRFWLIBS_LIBRARIES)


find_library(MFRFWLIBS_LIBRARY NAMES ${MFRFWLIBS_LIBRARIES}
        HINTS ${MFRFWLIBS_LIBDIR} ${MFRFWLIBS_LIBRARY_DIRS}
        )

if(MFRFWLIBS_LIBRARY AND NOT TARGET mfrfwlibs::mfrfwlibs)
    add_library(mfrfwlibs::mfrfwlibs UNKNOWN IMPORTED)
    set_target_properties(mfrfwlibs::mfrfwlibs PROPERTIES
            IMPORTED_LOCATION "${MFRFWLIBS_LIBRARY}"
            INTERFACE_LINK_LIBRARIES "${MFRFWLIBS_LIBRARIES}"
            INTERFACE_COMPILE_OPTIONS "${MFRFWLIBS_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${MFRFWLIBS_INCLUDE_DIRS}"
            )
endif()
