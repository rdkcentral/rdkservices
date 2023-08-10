# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
# Copyright 2021 Liberty Global Service B.V.
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

set(PLUGIN_NAME LgiHdcpProfile)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
        LgiHdcpProfile.cpp
        ../HdcpProfile/HdcpProfile.cpp
        Module.cpp)

add_definitions(-DMODULE_NAME=${PLUGIN_NAME})

find_package(CURL)
if (CURL_FOUND)
        include_directories(${CURL_INCLUDE_DIRS})
        target_link_libraries(${MODULE_NAME} PRIVATE ${CURL_LIBRARIES})
else (CURL_FOUND)
        message ("Curl/libcurl required.")
endif (CURL_FOUND)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

find_package(DS)
find_package(IARMBus)

target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS} ../helpers ../HdcpProfile)
target_include_directories(${MODULE_NAME} PRIVATE ${DS_INCLUDE_DIRS})

target_link_libraries(${MODULE_NAME} PUBLIC ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES} ${DS_LIBRARIES} ${NAMESPACE}SecurityUtil)


install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
