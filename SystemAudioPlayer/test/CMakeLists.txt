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

set(PLUGIN_NAME SystemAudioPlayerAPITest)
if (USE_THUNDER_R4)
    find_package(${NAMESPACE}COM REQUIRED)
else ()
    find_package(${NAMESPACE}Protocols REQUIRED)
endif (USE_THUNDER_R4)
find_package(securityagent REQUIRED)

option(PLUGIN_SECURITY_AGENT "Enable the Security Agent Features interface in javascript." OFF)

add_executable(${PLUGIN_NAME} SAPThunderAPITest.cpp)

set_target_properties(${PLUGIN_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    )

set(LIBRARIES WPEFrameworkSecurityUtil)
target_include_directories(${PLUGIN_NAME} PRIVATE ../../helpers ../impl)

if (USE_THUNDER_R4)
target_link_libraries(${PLUGIN_NAME} PUBLIC ${LIBRARIES} PRIVATE ${NAMESPACE}COM::${NAMESPACE}COM ${NAMESPACE}WebSocket::${NAMESPACE}WebSocket)
else ()
target_link_libraries(${PLUGIN_NAME} PUBLIC ${LIBRARIES}
    PRIVATE
    ${NAMESPACE}Protocols::${NAMESPACE}Protocols
    )
endif (USE_THUNDER_R4)


install(TARGETS ${PLUGIN_NAME} DESTINATION bin)

