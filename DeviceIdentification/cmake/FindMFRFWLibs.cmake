# This component contains software that is Copyright (c) 2018 RDK Management.
# The component is licensed to you under the Apache License, Version 2.0 (the "License").
# You may not use the component except in compliance with the License.
#
# The component may include material which is licensed under other licenses / copyrights as
# listed below. Your use of this material within the component is also subject to the terms and
# conditions of these licenses. The LICENSE file contains the text of all the licenses which apply
# within this component.
#
# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@webkit.org>
# Licensed under the BSD-2 license
#
# Based on pxCore, Copyright 2015-2018 John Robinson
# Licensed under the Apache License, Version 2.0
#
# Copyright (C) 2020 RDK Management
# Licensed under the BSD-2 license

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
