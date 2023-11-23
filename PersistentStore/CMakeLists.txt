# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2022 RDK Management
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

set(PLUGIN_NAME PersistentStore)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_PERSISTENTSTORE_PATH /opt/secure/persistent/rdkservicestore CACHE STRING "Path")
set(PLUGIN_PERSISTENTSTORE_KEY "" CACHE STRING "Encryption key")
set(PLUGIN_PERSISTENTSTORE_STARTUPORDER "" CACHE STRING "To configure startup order of PersistentStore plugin")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)

find_package(PkgConfig)

# enabling the secure extension requires a license: https://www.hwaci.com/cgi-bin/see-step1
if (USE_SQLITE_SEE)
    find_package(SqliteSee REQUIRED)

    # nonce generator
    if (BUILD_WITH_PLABELS)
        find_package(DL REQUIRED)
        find_package(Plabels REQUIRED)
        find_package(GLIB REQUIRED)

        set(PLABELS_SOURCE_FILES sqlite/see/SqliteSeeDbPlabels.cpp)
    else ()
        set(SQLITE_SEE_SOURCE_FILES sqlite/see/SqliteSeeDb.cpp)
    endif ()
else ()
    find_package(Sqlite REQUIRED)
endif ()

find_package(IARMBus)
if (IARMBus_FOUND)
    add_definitions(-DWITH_CLOCK_SYNC)
    set(IARMBUS_SOURCE_FILES sqlite/SqliteStore2WithClockSync.cpp)
endif ()

add_library(${MODULE_NAME} SHARED
        PersistentStore.cpp
        PersistentStoreJsonRpc.cpp
        sqlite/SqliteStore2.cpp
        sqlite/SqliteStoreCache.cpp
        sqlite/SqliteStoreInspector.cpp
        sqlite/SqliteDb.cpp
        ${SQLITE_SEE_SOURCE_FILES}
        ${PLABELS_SOURCE_FILES}
        ${IARMBUS_SOURCE_FILES}
        Module.cpp
        )

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )

add_definitions(${SQLITE_CFLAGS_OTHER} ${PLABELS_FLAGS})
link_directories(${SQLITE_LIBRARY_DIRS})

target_include_directories(${MODULE_NAME} PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${SQLITE_INCLUDE_DIRS}
        ${PLABELS_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
        ${IARMBUS_INCLUDE_DIRS}
        )

target_link_libraries(${MODULE_NAME} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${SQLITE_LIBRARIES}
        ${PLABELS_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${DL_LIBRARIES}
        ${IARMBUS_LIBRARIES}
        )

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
