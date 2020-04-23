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

set(PLUGIN_NAME Spark)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(Spark REQUIRED)
find_package(Freetype2 REQUIRED)

add_library(${MODULE_NAME} SHARED 
    Spark.cpp
    SparkImplementation.cpp
    SparkJsonRpc.cpp
    Module.cpp)

if(PLUGIN_SPARK_NAME)
    target_compile_definitions(${MODULE_NAME} 
        PRIVATE 
            FRIENDLY_NAME="${PLUGIN_SPARK_NAME}")
endif()

if (PLATFORM_LINUX)
    target_compile_definitions(${MODULE_NAME}
        PRIVATE
            RT_PLATFORM_LINUX=ON)
endif()

if (PLATFORM_WPEFRAMEWORK)
    target_compile_definitions(${MODULE_NAME}
        PRIVATE
            PX_PLATFORM_WPEFRAMEWORK=ON)
endif()

if (PLATFORM_WAYLAND_EGL)
    target_compile_definitions(${MODULE_NAME}
        PRIVATE
            PX_PLATFORM_WAYLAND_EGL=ON)
endif()

if (PLATFORM_ESSOS)
    target_compile_definitions(${MODULE_NAME}
        PRIVATE
        PX_PLATFORM_WAYLAND_EGL=ON PX_PLATFORM_ESSOS=ON)
endif()

target_compile_definitions(${MODULE_NAME}
    PRIVATE
        ANIMATION_ROTATE_XYZ=ON
        PX_SCENE_VERSION="dev"
        ENABLE_PERMISSIONS_CHECK=ON
        ENABLE_RT_NODE=ON
        ENABLE_HTTP_CACHE=ON
        RUNINMAIN=ON)

target_include_directories(${MODULE_NAME} 
    PRIVATE 
        ${FREETYPE2_INCLUDE_DIRS}) 

target_link_libraries(${MODULE_NAME} 
    PRIVATE 
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        Spark::Spark)

# Library installation section
install(
    TARGETS ${MODULE_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

set(PLUGIN_SPARK_AUTOSTART false CACHE STRING "Automatically start Spark plugin")
set(PLUGIN_SPARK_STARTURL "browser.js" CACHE STRING "Initial URL for Spark plugin")
set(PLUGIN_SPARK_RESOLUTION "720p" CACHE STRING "Browser resolution")

# resolution handling
if(PLUGIN_SPARK_RESOLUTION EQUAL "720p")
    set(PLUGIN_SPARK_HEIGHT "720")
    set(PLUGIN_SPARK_WIDTH "1280")
elseif(PLUGIN_SPARK_RESOLUTION EQUAL "1080p")
    set(PLUGIN_SPARK_HEIGHT "1080")
    set(PLUGIN_SPARK_WIDTH "1920")
elseif(PLUGIN_SPARK_RESOLUTION EQUAL "2160p")
    set(PLUGIN_SPARK_HEIGHT "2160")
    set(PLUGIN_SPARK_WIDTH "3840")
endif()

write_config(${PLUGIN_NAME})
