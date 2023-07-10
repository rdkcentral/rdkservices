# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright Synamedia, All rights reserved
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

set(PLUGIN_NAME UnifiedCASManagement)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Protocols REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(IARMBus)

find_package(LMPLAYER REQUIRED)

if (LMPLAYER_FOUND)
	add_library(${MODULE_NAME} SHARED
	        UnifiedCASManagement.cpp
	        Module.cpp
	        LibMediaPlayerImpl.cpp
	        )
else(LMPLAYER_FOUND)
    message ("MISSING A PLAYER IMPLEMENTATION.")
	add_library(${MODULE_NAME} SHARED
	        UnifiedCASManagement.cpp
	        Module.cpp
	        )
endif(LMPLAYER_FOUND)

if (IARMBus_FOUND)
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES})
else (IARMBus_FOUND)
    message ("Module IARMBus required.")
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES})
endif(IARMBus_FOUND)

add_definitions( -DRT_PLATFORM_LINUX=1 )

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

if (LMPLAYER_FOUND)
    add_definitions(-DLMPLAYER_FOUND)
    target_include_directories(${MODULE_NAME} PRIVATE ${LMPLAYER_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${LMPLAYER_LIBRARIES})

else(LMPLAYER_FOUND)
    message ("MISSING A PLAYER IMPLEMENTATION.")
endif(LMPLAYER_FOUND)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
