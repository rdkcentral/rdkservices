# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
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


set(PLUGIN_NAME L2Tests)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(THUNDER_PORT 9998)
find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
    Module.cpp
    L2Tests.cpp
    L2TestsMock.cpp
    tests/SystemService_L2Test.cpp
    tests/UsbAccess_L2Test.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} 
							PRIVATE 
							MODULE_NAME=Plugin_${PLUGIN_NAME}
							THUNDER_PORT="${THUNDER_PORT}")

target_compile_options(${MODULE_NAME} PRIVATE -Wno-error)
target_link_libraries(${MODULE_NAME} PRIVATE gmock_main ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

target_include_directories(${MODULE_NAME} PRIVATE ../../../helpers)
target_include_directories(${MODULE_NAME} PRIVATE ./)
target_include_directories(${MODULE_NAME} PRIVATE ../../mocks)
 
install(TARGETS ${MODULE_NAME}
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins)
 
write_config(${PLUGIN_NAME})



