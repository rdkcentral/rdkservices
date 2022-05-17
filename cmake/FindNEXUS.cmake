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

find_path(LIBNEXUS_INCLUDE nexus_config.h
        PATH_SUFFIXES refsw)

find_library(LIBNEXUS_LIBRARY nexus)

if(EXISTS "${LIBNEXUS_LIBRARY}")
    find_library(LIBB_OS_LIBRARY b_os)
    find_library(LIBNEXUS_CLIENT_LIBRARY nexus_client)
    find_library(LIBNXCLIENT_LIBRARY nxclient)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(NEXUS DEFAULT_MSG LIBNEXUS_INCLUDE LIBNEXUS_LIBRARY)
    mark_as_advanced(LIBNEXUS_INCLUDE LIBNEXUS_LIBRARY)

    if(NEXUS_FOUND AND NOT TARGET NEXUS::NEXUS)
        add_library(NEXUS::NEXUS UNKNOWN IMPORTED)
        set_target_properties(NEXUS::NEXUS PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                INTERFACE_INCLUDE_DIRECTORIES "${LIBNEXUS_INCLUDE}"
                    )

        if(NOT EXISTS "${LIBNEXUS_CLIENT_LIBRARY}")
            message(STATUS "Nexus in Proxy mode")
            set_target_properties(NEXUS::NEXUS PROPERTIES
                    IMPORTED_LOCATION "${LIBNEXUS_LIBRARY}"
                    )
        else()
            message(STATUS "Nexus in Client mode")
            set_target_properties(NEXUS::NEXUS PROPERTIES
                    IMPORTED_LOCATION "${LIBNEXUS_CLIENT_LIBRARY}"
                    )
        endif()

        if(NOT EXISTS "${LIBNXCLIENT_LIBRARY}")
            set_target_properties(NEXUS::NEXUS PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS NO_NXCLIENT
                    )
        endif()

        if(EXISTS "${LIBB_OS_LIBRARY}")
            set_target_properties(NEXUS::NEXUS PROPERTIES
                    IMPORTED_LINK_INTERFACE_LIBRARIES "${LIBB_OS_LIBRARY}"
                    )
        endif()
    endif()
    set_target_properties(NEXUS::NEXUS PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS "PLATFORM_BRCM"
            )
else()
    if(NEXUS_FIND_REQUIRED)
        message(FATAL_ERROR "LIBNEXUS_LIBRARY not available")
    elseif(NOT NEXUS_FIND_QUIETLY)
        message(STATUS "LIBNEXUS_LIBRARY not available")
    endif()
endif()
