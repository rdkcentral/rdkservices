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

set(PLUGIN_NAME DataCapture)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_DATACAPTURE_STARTUPORDER "" CACHE STRING "To configure startup order of DataCapture plugin")
set(PLUGIN_DATACAPTURE_AUTOSTART false CACHE STRING "To automatically start DataCapture plugin.")

find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
        socket_adaptor.cpp
        DataCapture.cpp
        Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

if (NOT RDK_SERVICES_TEST)
    target_compile_options(${MODULE_NAME} PRIVATE -Wno-error)
endif ()

find_package(Curl)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

find_package(AC)
if (AC_FOUND)
    find_package(IARMBus)
    add_definitions(-DAC_FOUND)
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_include_directories(${MODULE_NAME} PRIVATE ${AC_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME}
            PRIVATE
            ${NAMESPACE}Plugins::${NAMESPACE}Plugins
            ${CURL_LIBRARY}
            ${IARMBUS_LIBRARIES}
            ${AC_LIBRARIES}
            )

else(AC_FOUND)
    target_link_libraries(${MODULE_NAME}
            PRIVATE
            ${NAMESPACE}Plugins::${NAMESPACE}Plugins
            ${CURL_LIBRARY}
            )
endif(AC_FOUND)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
