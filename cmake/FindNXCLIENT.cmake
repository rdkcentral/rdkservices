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

find_path(LIBNXCLIENT_INCLUDE nexus_config.h
        PATH_SUFFIXES refsw)

find_library(LIBNXCLIENT_LIBRARY nxclient)

if(EXISTS "${LIBNXCLIENT_LIBRARY}")
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(NXCLIENT DEFAULT_MSG LIBNXCLIENT_INCLUDE LIBNXCLIENT_LIBRARY)
    mark_as_advanced(LIBNXCLIENT_LIBRARY)

    if(NXCLIENT_FOUND AND NOT TARGET NXCLIENT::NXCLIENT)
        add_library(NXCLIENT::NXCLIENT UNKNOWN IMPORTED)
        set_target_properties(NXCLIENT::NXCLIENT PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${LIBNXCLIENT_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIBNXCLIENT_INCLUDE}"
                )
    endif()
else()
    if(NXCLIENT_FIND_REQUIRED)
        message(FATAL_ERROR "LIBNXCLIENT_LIBRARY not available")
    elseif(NOT NXCLIENT_FIND_QUIETLY)
        message(STATUS "LIBNXCLIENT_LIBRARY not available")
    endif()
endif()
