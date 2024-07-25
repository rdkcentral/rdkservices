# If not stated otherwise in this file or this component's LICENSE file the
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

cmake_minimum_required(VERSION 3.14)

project(grpcl2test)

set(CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME}
        StubTest.cpp
)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)
target_link_libraries(${PROJECT_NAME} PRIVATE gmock_main)

find_package(Protobuf REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Protobuf_LIBRARIES})

add_custom_target(protoc
        ${Protobuf_PROTOC_EXECUTABLE} --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR}/../secure_storage ${CMAKE_CURRENT_SOURCE_DIR}/../secure_storage/secure_storage.proto
)
add_dependencies(${PROJECT_NAME} protoc)

find_package(gRPC CONFIG)
if (gRPC_FOUND)
    set(GRPC_LIBS gRPC::grpc++)
    set(GRPC_CPP_PLUGIN $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
else ()
    set(GRPC_LIBS grpc++)
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)
endif ()
target_link_libraries(${PROJECT_NAME} PRIVATE ${GRPC_LIBS})

add_custom_target(protoc-gen-grpc
        ${Protobuf_PROTOC_EXECUTABLE} --grpc_out ${CMAKE_CURRENT_BINARY_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} -I ${CMAKE_CURRENT_SOURCE_DIR}/../secure_storage ${CMAKE_CURRENT_SOURCE_DIR}/../secure_storage/secure_storage.proto
)
add_dependencies(${PROJECT_NAME} protoc-gen-grpc)

set(PROTO_SRCS secure_storage.pb.cc secure_storage.grpc.pb.cc)
target_sources(${PROJECT_NAME} PRIVATE ${PROTO_SRCS})
set_property(SOURCE ${PROTO_SRCS} PROPERTY GENERATED 1)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
