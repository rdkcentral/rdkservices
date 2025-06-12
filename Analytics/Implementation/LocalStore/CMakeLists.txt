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
set(TARGET_LIB ${NAMESPACE}${PLUGIN_NAME}LocalStore)

add_library(${TARGET_LIB} STATIC)

target_sources(${TARGET_LIB} PRIVATE LocalStore.cpp
        DatabaseConnection.cpp)
    
find_package(Sqlite)
if (SQLITE_FOUND)
    include_directories(${SQLITE_INCLUDE_DIRS})
    target_link_libraries(${TARGET_LIB} PRIVATE ${SQLITE_LIBRARIES})
else (SQLITE_FOUND)
    message ("Sqlite3 required.")
endif (SQLITE_FOUND)

target_include_directories(${TARGET_LIB} PUBLIC "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${TARGET_LIB} PRIVATE "${CMAKE_SOURCE_DIR}/Analytics")
target_include_directories(${TARGET_LIB} PRIVATE "${CMAKE_SOURCE_DIR}/Analytics/Implementation/Interfaces")
target_include_directories(${TARGET_LIB} PRIVATE "${CMAKE_SOURCE_DIR}/helpers")
set_property(TARGET ${TARGET_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(${TARGET_LIB} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_link_libraries(${TARGET_LIB} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)
