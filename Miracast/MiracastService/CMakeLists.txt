# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
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

set(PLUGIN_NAME MiracastService)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_MIRACAST_STARTUPORDER "" CACHE STRING "To configure startup order of MiracastService plugin")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(IARMBus)
find_package(GLIB REQUIRED)

find_package(WPEFrameworkSecurityUtil)
if(NOT WPEFRAMEWORK_SECURITYUTIL_FOUND)
    add_definitions(-DSECURITY_TOKEN_ENABLED=0)
endif()

find_library(GLIB_LIBRARY NAMES glib-2.0 )

add_library(${MODULE_NAME} SHARED Module.cpp MiracastService.cpp ../common/MiracastCommon.cpp ../common/MiracastLogger.cpp MiracastController.cpp ../P2P/MiracastP2P.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)
set_source_files_properties(MiracastService.cpp PROPERTIES COMPILE_FLAGS "-fexceptions")

target_include_directories(${MODULE_NAME} PRIVATE ./ ../include ../../helpers ${IARMBUS_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})

#target_compile_options(${MODULE_NAME} PRIVATE -Wno-error -fpermissive -Os -pipe -g -feliminate-unused-debug-types -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed)
#target_compile_options(${MODULE_NAME} PRIVATE -DENABLE_MIRACAST_SERVICE_TEST_NOTIFIER)

target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)
target_link_libraries(${MODULE_NAME} PRIVATE ${IARMBUS_LIBRARIES})
target_link_libraries(${MODULE_NAME} PRIVATE ${GLIB_LIBRARIES})
target_link_libraries(${MODULE_NAME} PRIVATE -lpthread)

if(WPEFRAMEWORK_SECURITYUTIL_FOUND)
    target_link_libraries(${MODULE_NAME} PRIVATE -lwpa_client)
endif()

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
