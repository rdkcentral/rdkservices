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

set(PLUGIN_NAME Packager)
set(MODULE_NAME WPEFramework${PLUGIN_NAME})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(BUILD_WITH_PACKAGER_EX "BUILD_WITH_PACKAGER_EX" OFF)

######################################################################################################
######################################################################################################
##
## Support for PACKAGER_EX additions
##

if (BUILD_WITH_PACKAGER_EX)

    add_definitions(-DINCLUDE_PACKAGER_EX)

    message("Building with support for PACKAGER_EX additions")

    find_path(LIBUUID_INCLUDE_DIR uuid.h PATH_SUFFIXES uuid)
    find_library(LIBUUID_LIBRARY libuuid.so)
    include_directories(${LIBUUID_INCLUDE_DIR})

    find_package(${NAMESPACE}Plugins REQUIRED)
    find_package(libprovision QUIET)
    find_package(LibOPKG REQUIRED)
    find_package(GLIB REQUIRED)

    find_package(CURL)
    find_package(CompileSettingsDebug CONFIG REQUIRED)

    # enabling the secure extension requires a license: https://www.hwaci.com/cgi-bin/see-step1
    if (USE_SQLITE_SEE)
        find_package(SqliteSee REQUIRED)
    else()
        find_package(Sqlite REQUIRED)
    endif()

    add_library(${MODULE_NAME} SHARED
        Module.cpp
        Packager.cpp
        PackagerImplementation.cpp
        PackagerExUtils.cpp
        PackagerExImplementation.cpp)

    if (CURL_FOUND)
        include_directories(${CURL_INCLUDE_DIRS})
        target_link_libraries(${MODULE_NAME} PRIVATE ${CURL_LIBRARIES})
    else (CURL_FOUND)
        message ("Curl/libcurl required.")
    endif (CURL_FOUND)

    add_definitions(${SQLITE_CFLAGS_OTHER} ${PLABELS_FLAGS})
    link_directories(${SQLITE_LIBRARY_DIRS})


    target_include_directories(${MODULE_NAME} PRIVATE ../helpers
            ${SQLITE_INCLUDE_DIRS}
            ${PLABELS_INCLUDE_DIRS}
            ${GLIB_INCLUDE_DIRS})

    if (libprovision_FOUND)
        target_link_libraries(${MODULE_NAME}
            PRIVATE
                CompileSettingsDebug::CompileSettingsDebug
                ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                LibOPKG::LibOPKG
                ${LIBUUID_LIBRARY}
                ${SQLITE_LIBRARIES}
                ${GLIB_LIBRARIES}
                libprovision::libprovision

                )
    else (libprovision_FOUND)
        target_include_directories(${MODULE_NAME}
            PRIVATE
                ${LIBOPKG_INCLUDE_DIRS}
                )

        target_link_libraries(${MODULE_NAME}
            PRIVATE
                CompileSettingsDebug::CompileSettingsDebug
                ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                ${LIBOPKG_LIBRARIES}
                ${LIBUUID_LIBRARY}
                ${SQLITE_LIBRARIES}
                ${GLIB_LIBRARIES}
                )
    endif (libprovision_FOUND)

    string(TOLOWER ${NAMESPACE} STORAGENAME)
    install(TARGETS ${MODULE_NAME} 
        DESTINATION lib/${STORAGENAME}/plugins)

    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/opkg.conf.in"
            "${CMAKE_CURRENT_BINARY_DIR}/opkg.conf"
            @ONLY)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/opkg.conf
            DESTINATION "/usr/share/${NAMESPACE}/${PLUGIN_NAME}")

    write_config(${PLUGIN_NAME})

######################################################################################################
######################################################################################################
##
## Support for legacy PACKAGER
##

else (BUILD_WITH_PACKAGER_EX)

    message("Building with support for legacy PACKAGER")

    set(PLUGIN_NAME Packager)
    set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

    find_path(LIBUUID_INCLUDE_DIR uuid.h PATH_SUFFIXES uuid)
    find_library(LIBUUID_LIBRARY libuuid.so)
    include_directories(${LIBUUID_INCLUDE_DIR})

    find_package(${NAMESPACE}Plugins REQUIRED)
    find_package(libprovision QUIET)
    find_package(LibOPKG REQUIRED)
    find_package(GLIB REQUIRED)

    find_package(CURL)
    find_package(CompileSettingsDebug CONFIG REQUIRED)

    # enabling the secure extension requires a license: https://www.hwaci.com/cgi-bin/see-step1
    if (USE_SQLITE_SEE)
        find_package(SqliteSee REQUIRED)
    else()
        find_package(Sqlite REQUIRED)
    endif()


    add_library(${MODULE_NAME} SHARED
        Module.cpp
        Packager.cpp
        PackagerImplementation.cpp
        PackagerExUtils.cpp
        PackagerExImplementation.cpp)

    if (CURL_FOUND)
        include_directories(${CURL_INCLUDE_DIRS})
        target_link_libraries(${MODULE_NAME} PRIVATE ${CURL_LIBRARIES})
    else (CURL_FOUND)
        message ("Curl/libcurl required.")
    endif (CURL_FOUND)

    add_definitions(${SQLITE_CFLAGS_OTHER} ${PLABELS_FLAGS})
    link_directories(${SQLITE_LIBRARY_DIRS})


    target_include_directories(${MODULE_NAME} PRIVATE ../helpers
            ${SQLITE_INCLUDE_DIRS}
            ${PLABELS_INCLUDE_DIRS}
            ${GLIB_INCLUDE_DIRS})

    if (libprovision_FOUND)
        target_link_libraries(${MODULE_NAME}
            PRIVATE
                CompileSettingsDebug::CompileSettingsDebug
                ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                LibOPKG::LibOPKG
                ${LIBUUID_LIBRARY}
                ${SQLITE_LIBRARIES}
                ${GLIB_LIBRARIES}
                libprovision::libprovision

                )
    else (libprovision_FOUND)
        target_include_directories(${MODULE_NAME}
            PRIVATE
                ${LIBOPKG_INCLUDE_DIRS}
                )

        target_link_libraries(${MODULE_NAME}
            PRIVATE
                CompileSettingsDebug::CompileSettingsDebug
                ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                ${LIBOPKG_LIBRARIES}
                ${LIBUUID_LIBRARY}
                ${SQLITE_LIBRARIES}
                ${GLIB_LIBRARIES}
                )
    endif (libprovision_FOUND)

    string(TOLOWER ${NAMESPACE} STORAGENAME)
    install(TARGETS ${MODULE_NAME} 
        DESTINATION lib/${STORAGENAME}/plugins)

    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/opkg.conf.in"
            "${CMAKE_CURRENT_BINARY_DIR}/opkg.conf"
            @ONLY)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/opkg.conf
            DESTINATION "/usr/share/${NAMESPACE}/${PLUGIN_NAME}")

    write_config(${PLUGIN_NAME})

endif (BUILD_WITH_PACKAGER_EX)

######################################################################################################
######################################################################################################