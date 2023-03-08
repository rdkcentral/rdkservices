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

set(PLUGIN_NAME MiracastService)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_MIRACAST_STARTUPORDER "" CACHE STRING "To configure startup order of MiracastService plugin")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(DS)
find_package(IARMBus)
find_package(GLIB REQUIRED)

find_library(GLIB_LIBRARY NAMES glib-2.0 )

add_library(${MODULE_NAME} SHARED MiracastService.cpp Module.cpp MiracastServiceImplementation.cpp MiracastServicePrivate.cpp MiracastServiceP2P.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})
target_compile_options(${MODULE_NAME} PRIVATE -Wno-error)
target_compile_options(${MODULE_NAME} PRIVATE -fpermissive -Os -pipe -g -feliminate-unused-debug-types -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed)

#LDFLAGS += -lwpa_client -lpthread -lglib-2.0 -lrdkloggers -lIARMBus

target_include_directories(${MODULE_NAME} PRIVATE ./ ../helpers ${IARMBUS_INCLUDE_DIRS} ${DS_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})

target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES} ${GLIB_LIBRARIES} -lwpa_client -lpthread -lrdkloggers)

#target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${NAMESPACE}SecurityUtil ${IARMBUS_LIBRARIES} ${DS_LIBRARIES} ${GLIB_LIBRARIES})

#add_definitions(-DDS_FOUND)

install(TARGETS ${MODULE_NAME}
	DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
