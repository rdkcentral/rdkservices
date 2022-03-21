# - Try to find MFRFWLIBS
# Once done this will define
#  MFRFWLIBS_FOUND - System has MFRFWLibs
#  MFRFWLIBS_INCLUDE_DIRS - The MFRFWLibs include directories
#  MFRFWLIBS_LIBRARIES - The libraries needed to use MFRFWLibs
#
# Copyright 2020 RDK Management.
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

if(MFRFWLibs_FIND_QUIETLY)
    set(_MFRFWLIBS_MODE QUIET)
elseif(MFRFWLibs_FIND_REQUIRED)
    set(_MFRFWLIBS_MODE REQUIRED)
endif()

find_package(PkgConfig)
pkg_check_modules(MFRFWLIBS ${_MFRFWLIBS_MODE} fwupgrade)
find_library(MFRFWLIBS_LIBRARY NAMES ${MFRFWLIBS_LIBRARIES}
        HINTS ${MFRFWLIBS_LIBDIR} ${MFRFWLIBS_LIBRARY_DIRS}
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MFRFWLibs DEFAULT_MSG MFRFWLIBS_LIBRARY MFRFWLIBS_LIBRARIES)
mark_as_advanced(MFRFWLIBS_INCLUDE_DIRS MFRFWLIBS_LIBRARIES)

if(MFRFWLibs_FOUND AND NOT TARGET MFRFWLibs::MFRFWLibs)
    add_library(MFRFWLibs::MFRFWLibs UNKNOWN IMPORTED)
    set_target_properties(MFRFWLibs::MFRFWLibs PROPERTIES
            IMPORTED_LOCATION "${MFRFWLIBS_LIBRARY}"
            INTERFACE_LINK_LIBRARIES "${MFRFWLIBS_LIBRARIES}"
            INTERFACE_COMPILE_OPTIONS "${MFRFWLIBS_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${MFRFWLIBS_INCLUDE_DIRS}"
            )
endif()
