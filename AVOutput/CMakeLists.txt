# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2024 Sky UK
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

cmake_minimum_required (VERSION 2.6)

Project(AVOUTPUT-PLUGINS)

set(CMAKE_BUILD_TYPE "Debug")

set(PLUGIN_NAME AVOutput)

set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_AVOUTPUT_AUTOSTART "true" CACHE STRING "Automatically start AVOutput plugin")

find_package(WPEFramework)
find_package(${NAMESPACE}Plugins REQUIRED)

set (CMAKE_CXX_STANDARD 11)

find_path (STAGING_INCDIR glib-2.0)
include_directories(${STAGING_INCDIR})
include_directories(${STAGING_INCDIR}/glib-2.0)
include_directories(${STAGING_INCDIR}/../lib/glib-2.0/include)
include_directories(${STAGING_INCDIR}/rdk/iarmbus)
include_directories(${STAGING_INCDIR}/rdk/iarmmgrs-hal)
include_directories(${STAGING_INCDIR}/rdk/tv-hal)
include_directories(${STAGING_INCDIR}/rdk/ds-hal)
include_directories(${STAGING_INCDIR}/rdk/ds-rpc)
include_directories(${STAGING_INCDIR}/rdk/ds)

if (AVOUTPUT_TV)
add_library(${MODULE_NAME} SHARED
	AVOutputBase.cpp
	AVOutputTV.cpp
        AVOutputTVPrivateAPIs.cpp	
        AVOutput.cpp
        Module.cpp)
target_link_libraries(${MODULE_NAME} "-lglib-2.0 -lpthread -lIARMBus -ltvsettings-hal -ltr181api -lds")
else()
add_library(${MODULE_NAME} SHARED
	AVOutputBase.cpp
	AVOutputSTB.cpp
        AVOutput.cpp
        Module.cpp)
target_link_libraries(${MODULE_NAME} "-lglib-2.0 -lpthread -lIARMBus -lds")
endif()

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(DS)

target_include_directories(${MODULE_NAME} PRIVATE ${DS_INCLUDE_DIRS})
target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins)


install(TARGETS ${MODULE_NAME} 
    DESTINATION lib/wpeframework/plugins)

write_config(${PLUGIN_NAME})
