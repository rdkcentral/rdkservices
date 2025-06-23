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


set(LIB_NAME AnalyticsBackendMock)
set(MODULE_NAME ${NAMESPACE}${LIB_NAME})


message("Setup ${MODULE_NAME} v${MODULE_VERSION}")

set(LIB_ANALYTICS_MOCK_SERVER_URL "127.0.0.1:12345" CACHE STRING "Sift max randomisation window time of posting queued events")


add_library(${MODULE_NAME} SHARED
        AnalyticsBackendMock.cpp)

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

find_package(CURL)
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${CURL_LIBRARIES})
else (CURL_FOUND)
    message ("Curl/libcurl required.")
endif (CURL_FOUND)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Lib_${LIB_NAME})

target_compile_definitions(${MODULE_NAME} PRIVATE 
        SERVER_URL="${LIB_ANALYTICS_MOCK_SERVER_URL}")

target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/helpers")
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Analytics/Implementation/Interfaces")
target_include_directories(${MODULE_NAME} PRIVATE "${WPEFRAMEWORK_INCLUDE_DIRS}")

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(${MODULE_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_link_libraries(${MODULE_NAME}
        PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)