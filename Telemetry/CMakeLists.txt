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

set(PLUGIN_NAME Telemetry)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_TELEMETRY_STARTUPORDER "" CACHE STRING "To configure startup order of Telemetry plugin")
set(PLUGIN_T2_PERSISTENT_FOLDER /opt/.t2reportprofiles/ CACHE STRING "Path to t2 persistent folder")
set(PLUGIN_DEFAULT_PROFILES_FILE /etc/t2profiles/default.json CACHE STRING "Path to default t2 profile")

find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
        Telemetry.cpp
        Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

if (RDK_SERVICE_L2_TEST)
   find_library(TESTMOCKLIB_LIBRARIES NAMES TestMocklib)
   if (TESTMOCKLIB_LIBRARIES)
       message ("linking mock libraries ${TESTMOCKLIB_LIBRARIES} library")
       target_link_libraries(${MODULE_NAME} PRIVATE ${TESTMOCKLIB_LIBRARIES})
   else (TESTMOCKLIB_LIBRARIES)
       message ("Require ${TESTMOCKLIB_LIBRARIES} library")
   endif (TESTMOCKLIB_LIBRARIES)
endif (RDK_SERVICES_L2_TEST)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(RFC)

find_path(RBUS_API_HEADER NAMES rbus.h PATH_SUFFIXES rbus)

if (NOT ${RBUS_API_HEADER} STREQUAL "RBUS_API_HEADER-NOTFOUND")
    message("Found rbus.h ${RBUS_API_HEADER}")
    add_definitions (-DHAS_RBUS)
    set(RBUS_API_HEADERS ${RBUS_API_HEADER})
find_library(RBUS_LIBRARY NAMES rbus)
endif()

set(RBUS_LIBRARIES "")
if (NOT ${RBUS_LIBRARY} STREQUAL "RBUS_LIBRARY-NOTFOUND")
    set(RBUS_LIBRARIES ${RBUS_LIBRARY})
endif()

find_package(IARMBus)

target_include_directories(${MODULE_NAME} PRIVATE ${RFC_INCLUDE_DIRS} ${RBUS_API_HEADERS} ${IARMBUS_INCLUDE_DIRS} ../helpers)
target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${RFC_LIBRARIES} ${RBUS_LIBRARIES} ${IARMBUS_LIBRARIES} )

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
