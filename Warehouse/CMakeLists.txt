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

set(PLUGIN_NAME Warehouse)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_WAREHOUSE_STARTUPORDER "" CACHE STRING "To configure startup order of Warehouse plugin")
set(PLUGIN_WAREHOUSE_AUTOSTART false CACHE STRING "To automatically start Warehouse plugin.")

find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
        Warehouse.cpp
        Module.cpp
        ../helpers/frontpanel.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})
set_source_files_properties(Warehouse.cpp ../helpers/frontpanel.cpp PROPERTIES COMPILE_FLAGS "-fexceptions")

find_package(DS)
if (DS_FOUND)
    find_package(IARMBus)
    add_definitions(-DDS_FOUND)
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_include_directories(${MODULE_NAME} PRIVATE ${DS_INCLUDE_DIRS})
    target_include_directories(${MODULE_NAME} PRIVATE ../helpers)
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES} ${DS_LIBRARIES})
#    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${DS_LIBRARIES})
else (DS_FOUND)
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)
endif(DS_FOUND)

#find_package(RFC)

#find_package(IARMBus)

#target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS} ../helpers)
#target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES})

if (NOT RDK_SERVICES_L1_TEST)
    target_compile_options(${MODULE_NAME} PRIVATE -Wno-error)
endif ()

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
