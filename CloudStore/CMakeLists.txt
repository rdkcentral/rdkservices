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

cmake_minimum_required(VERSION 3.14)

set(PLUGIN_NAME CloudStore)
find_package(WPEFramework NAMES WPEFramework Thunder)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(CMAKE_CXX_STANDARD 11)

set(PLUGIN_CLOUDSTORE_MODE "Off" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")
set(PLUGIN_CLOUDSTORE_URI "" CACHE STRING "Endpoint")
set(PLUGIN_CLOUDSTORE_STARTUPORDER "" CACHE STRING "To configure startup order of the plugin")

add_library(${MODULE_NAME} SHARED
        CloudStore.cpp
        Module.cpp
)

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
target_link_libraries(${MODULE_NAME} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
)

find_library(RFC_LIBRARIES NAMES rfcapi)
if (RFC_LIBRARIES)
    find_path(RFC_INCLUDE_DIRS NAMES rfcapi.h REQUIRED)
    target_include_directories(${MODULE_NAME} PRIVATE ${RFC_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${RFC_LIBRARIES})
    target_compile_definitions(${MODULE_NAME} PRIVATE WITH_RFC)
endif ()

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

set(PLUGIN_IMPLEMENTATION ${MODULE_NAME}Implementation)
add_library(${PLUGIN_IMPLEMENTATION} SHARED
        Module.cpp
        CloudStoreImplementation.cpp
)

target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
)

find_library(IARMBUS_LIBRARIES NAMES IARMBus)
if (IARMBUS_LIBRARIES)
    find_path(IARMBUS_INCLUDE_DIRS NAMES libIBus.h PATH_SUFFIXES rdk/iarmbus REQUIRED)
    find_path(IARMSYS_INCLUDE_DIRS NAMES sysMgr.h PATH_SUFFIXES rdk/iarmmgrs/sysmgr REQUIRED)
    target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${IARMBUS_INCLUDE_DIRS} ${IARMSYS_INCLUDE_DIRS})
    target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${IARMBUS_LIBRARIES})
    target_compile_definitions(${PLUGIN_IMPLEMENTATION} PRIVATE WITH_SYSMGR)
endif ()

find_package(Protobuf REQUIRED)
target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${Protobuf_LIBRARIES})

add_custom_target(protoc
        ${Protobuf_PROTOC_EXECUTABLE} --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR}/grpc/secure_storage ${CMAKE_CURRENT_SOURCE_DIR}/grpc/secure_storage/secure_storage.proto
)
add_dependencies(${PLUGIN_IMPLEMENTATION} protoc)

find_package(gRPC CONFIG)
if (gRPC_FOUND)
    set(GRPC_LIBS gRPC::grpc++)
else ()
    set(GRPC_LIBS grpc++)
endif ()
target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${GRPC_LIBS})
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)

add_custom_target(protoc-gen-grpc
        ${Protobuf_PROTOC_EXECUTABLE} --grpc_out ${CMAKE_CURRENT_BINARY_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} -I ${CMAKE_CURRENT_SOURCE_DIR}/grpc/secure_storage ${CMAKE_CURRENT_SOURCE_DIR}/grpc/secure_storage/secure_storage.proto
)
add_dependencies(${PLUGIN_IMPLEMENTATION} protoc-gen-grpc)

set(PROTO_SRCS secure_storage.pb.cc secure_storage.grpc.pb.cc)
target_sources(${PLUGIN_IMPLEMENTATION} PRIVATE ${PROTO_SRCS})
set_property(SOURCE ${PROTO_SRCS} PROPERTY GENERATED 1)
target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS ${PLUGIN_IMPLEMENTATION}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
