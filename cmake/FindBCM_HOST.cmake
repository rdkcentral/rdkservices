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

if(BCM_HOST_FIND_QUIETLY)
    set(_BCM_HOST_MODE QUIET)
elseif(BCM_HOST_FIND_REQUIRED)
    set(_BCM_HOST_MODE REQUIRED)
endif()

find_package(PkgConfig)
pkg_check_modules(PC_BCM_HOST ${_BCM_HOST_MODE} bcm_host)

if(${PC_BCM_HOST_FOUND})
    find_library(BCM_HOST_LIBRARY bcm_host
        HINTS ${PC_BCM_LIBDIR} ${PC_BCM_LIBRARY_DIRS}
    )
    set(BCM_LIBRARIES ${PC_BCM_HOST_LIBRARIES})

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(BCM_HOST DEFAULT_MSG PC_BCM_HOST_FOUND PC_BCM_HOST_INCLUDE_DIRS BCM_HOST_LIBRARY PC_BCM_HOST_LIBRARIES)
    mark_as_advanced(PC_BCM_HOST_INCLUDE_DIRS PC_BCM_HOST_LIBRARIES)

    if(BCM_HOST_FOUND AND NOT TARGET BCM_HOST::BCM_HOST)
        add_library(BCM_HOST::BCM_HOST UNKNOWN IMPORTED)

        set_target_properties(BCM_HOST::BCM_HOST
            PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${BCM_HOST_LIBRARY}"
                INTERFACE_COMPILE_DEFINITIONS "PLATFORM_RPI"
                INTERFACE_COMPILE_OPTIONS "${PC_BCM_HOST_CFLAGS_OTHER}"
                INTERFACE_INCLUDE_DIRECTORIES "${PC_BCM_HOST_INCLUDE_DIRS}"
                INTERFACE_LINK_LIBRARIES "${PC_BCM_HOST_LIBRARIES}"
                )
    endif()
endif()
