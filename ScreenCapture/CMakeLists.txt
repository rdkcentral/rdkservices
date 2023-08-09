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

set(PLUGIN_NAME ScreenCapture)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_SCREENCAPTURE_STARTUPORDER "" CACHE STRING "To configure startup order of ScreenCapture plugin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(LibDRM QUIET)
find_path (STAGING_INCDIR glib-2.0)
include_directories(${STAGING_INCDIR}/libdrm)

add_library(${MODULE_NAME} SHARED
        ScreenCapture.cpp
        Module.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})

if (BUILD_REALTEK AND LIBDRM_FOUND)
    message("Found libdrm")
    add_definitions (-DUSE_DRM_SCREENCAPTURE)

    target_sources(${MODULE_NAME}
        PRIVATE
        Implementation/Realtek/kms.c
		Implementation/Realtek/Realtek.cpp)
    target_link_libraries(${MODULE_NAME}
        PRIVATE
            drm)
endif()

target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins -lpng -lcurl)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
