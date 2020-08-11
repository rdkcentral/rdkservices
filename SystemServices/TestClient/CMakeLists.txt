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

set(PLUGIN_NAME systemServiceTestClient)
find_package(WPEFramework REQUIRED)

if (WPEFRAMEWORK_FOUND)
    message("WPEFRAMEWORK_FOUND = ${WPEFRAMEWORK_FOUND}")
    message("< ${WPEFRAMEWORK_DEFINITIONS} | ${WPEFRAMEWORK_INCLUDE_DIRS} |
        ${WPEFRAMEWORK_LIBRARIES} | ${WPEFRAMEWORK_LIBRARY_WPEFrameworkPlugins} |
        ${WPEFRAMEWORK_LIBRARY_WPEFrameworkCore} | 
        ${WPEFRAMEWORK_LIBRARY_WPEFrameworkTracing} | 
        ${WPEFRAMEWORK_LIBRARY_WPEFrameworkProtocols} >")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR} PRIVATE
        ${WPEFRAMEWORK_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${WPEFRAMEWORK_LIBRARIES})
else()
    message(FATAL_ERROR "WPEFrameworkProtocols not found")
endif()

add_executable(${PLUGIN_NAME} systemServiceTestClient.cpp)

set_target_properties(${PLUGIN_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    )

install(TARGETS ${PLUGIN_NAME} DESTINATION bin)

