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

set(PLUGIN_NAME IOTControl)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_NETWORK_STARTUPORDER "" CACHE STRING "To configure startup order of IOTControl plugin")

find_package(${NAMESPACE}Plugins REQUIRED)
#find_package(Avahi)

add_library(${MODULE_NAME} SHARED
        IOTControl.cpp
        IOTConnector.cpp
        AvahiClient.cpp
        Module.cpp)

find_path(RTMESSAGE_API_HEADER NAMES rtMessage.h PATH_SUFFIXES rtmessage)

if (NOT ${RTMESSAGE_API_HEADER} STREQUAL "RBUS_API_HEADER-NOTFOUND")
        message("Found rtMessage.h ${RTMESSAGE_API_HEADER}")
        add_definitions (-DHAS_RBUS)
        set(RTMESSAGE_API_HEADERS ${RTMESSAGE_API_HEADER})
find_library(RTMESSAGE_LIBRARY NAMES rtMessage)
endif()

set(RTMESSAGE_LIBRARIES "")
if (NOT ${RTMESSAGE_LIBRARY} STREQUAL "RBUS_LIBRARY-NOTFOUND")
        set(RTMESSAGE_LIBRARIES ${RTMESSAGE_LIBRARY})
endif()

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")        
set_source_files_properties(
	IOTControl.cpp
        PROPERTIES COMPILE_FLAGS "-fexceptions")

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers ${RTMESSAGE_API_HEADERS})
target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${RTMESSAGE_LIBRARIES} ${AVAHI_CLIENT_LIBRARIES} ${AVAHI_COMMON_LIBRARIES})

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
