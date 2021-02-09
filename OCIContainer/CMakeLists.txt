# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2021 RDK Management
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


set(PLUGIN_NAME OCIContainer)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Dobby REQUIRED CONFIG)

# Temporary fix to get defines in Dobby. Will be removed later.
add_definitions( -DRDK )

# Set default Dobby dbus service to org.rdk.dobby and add defines
set( DOBBY_SERVICE "org.rdk.dobby" CACHE STRING "Dobby dbus service name")
if(NOT DOBBY_SERVICE STREQUAL "org.rdk.dobby")
    add_definitions( -DDOBBY_SERVICE_OVERRIDE="${DOBBY_SERVICE}")
endif()

# Set default Dobby dbus object path to /org/rdk/dobby and add defines
set( DOBBY_OBJECT "/org/rdk/dobby" CACHE STRING "Dobby dbus object path")
if(NOT DOBBY_OBJECT STREQUAL "/org/rdk/dobby")
    add_definitions( -DDOBBY_OBJECT_OVERRIDE="${DOBBY_OBJECT}")
endif()

add_library(${MODULE_NAME} SHARED
        OCIContainer.cpp
        Module.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# We need libsystemd since we're using their dbus library (sd-bus)
# There's probably a better way of doing this...
find_path(SYSTEMD_INCLUDE_DIRS
    NAMES systemd/sd-bus.h
)

find_library(SYSTEMD_LIBRARIES
    NAMES systemd
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LIBSYSTEMD
    SYSTEMD_LIBRARIES SYSTEMD_INCLUDE_DIRS
)

target_include_directories(${MODULE_NAME}
        PRIVATE
        ../helpers

        )

target_link_libraries(${MODULE_NAME}
        PRIVATE

        # Dobby libraries
        DobbyClientLib
        IpcService
        AppInfraCommon
        AppInfraLogging
        JsonCpp::JsonCpp

        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${SYSTEMD_LIBRARIES}
)
# ${NAMESPACE}Protocols::${NAMESPACE}Protocols
install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})