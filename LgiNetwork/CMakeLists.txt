# If not stated otherwise in this file or this component's license file the
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

set(PLUGIN_NAME Network)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_NETWORK_STARTUPORDER "" CACHE STRING "To configure startup order of Network plugin")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(LibGlib REQUIRED)
find_package(LibGio REQUIRED)

find_path(LIBGIOUNIX_INCLUDE_DIR
          NAMES gio/gunixfdlist.h
          PATH_SUFFIXES gio-unix-2.0 )

message("LIBGIOUNIX_INCLUDE_DIR       = ${LIBGIOUNIX_INCLUDE_DIR}")
set(LIBGIO_INCLUDE_DIRS ${LIBGIO_INCLUDE_DIR} ${LIBGIOUNIX_INCLUDE_DIR})

add_library(${MODULE_NAME} SHARED
        Network.cpp
        NetUtils.cpp
        NetUtilsNetlink.cpp
        NetworkTraceroute.cpp
        PingNotifier.cpp
        Module.cpp
        dbus/lginetwork_client.cpp
        dbus/lginetwork_dbus_api.c
        )

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

add_definitions (-DNET_DEFINED_INTERFACES_ONLY)

include_directories(${LIBGLIB_INCLUDE_DIRS})
include_directories(${LIBGIO_INCLUDE_DIRS})
target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

target_link_libraries(${MODULE_NAME}
        PRIVATE
                ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                ${NAMESPACE}Definitions::${NAMESPACE}Definitions
                ${LIBGIO_LIBRARIES}
                ${LIBGLIB_LIBRARIES})

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
