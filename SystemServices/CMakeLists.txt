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

set(PLUGIN_NAME SystemServices)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)

if(BUILD_TESTS)
    add_subdirectory(TestClient)
endif()

add_library(${MODULE_NAME} SHARED
        SystemServices.cpp
        Module.cpp
        cTimer.cpp
        ../helpers/cSettings.cpp
        ../helpers/powerstate.cpp
        thermonitor.cpp
        SystemServicesHelper.cpp
        ../helpers/utils.cpp
        uploadlogs.cpp
        platformcaps/platformcaps.cpp
        platformcaps/platformcapsdata.cpp
        platformcaps/platformcapsdatarpc.cpp
        )

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(IARMBus)
if (IARMBus_FOUND)
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES})
else (IARMBus_FOUND)
    message ("Module IARMBus required.")
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES})
endif(IARMBus_FOUND)

find_package(DS)
if (DS_FOUND)
	find_package(IARMBus)
    add_definitions(-DDS_FOUND)
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_include_directories(${MODULE_NAME} PRIVATE ${DS_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES} ${DS_LIBRARIES})
else (DS_FOUND)
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)
endif(DS_FOUND)

find_package(CURL)
if (CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIRS})
	target_link_libraries(${MODULE_NAME} PRIVATE ${CURL_LIBRARIES})
else (CURL_FOUND)
	message ("Curl/libcurl required.")
endif (CURL_FOUND)

find_package(RFC)
if (RFC_FOUND)
	target_include_directories(${MODULE_NAME} PRIVATE ${RFC_INCLUDE_DIRS})
	target_link_libraries(${MODULE_NAME} PRIVATE ${RFC_LIBRARIES})
else (RFC_FOUND)
	message ("RFC lib required.")
endif (RFC_FOUND)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers)
target_include_directories(${MODULE_NAME} PRIVATE ./)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})

