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

set(PLUGIN_NAME InjectedBundle)
set(MODULE_NAME WPE${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(securityagent QUIET)

option(PLUGIN_AMAZON_HAWAII "Enable the Amazon Player interface (Hawaii) in javascript." OFF)
option(PLUGIN_SECURITYAGENT "Enable the Security Agent Features interface in javascript." OFF)
option(PLUGIN_WEBKITBROWSER_AAMP_JSBINDINGS "Enable AAMP JS bindings." OFF)
option(PLUGIN_WEBKITBROWSER_BADGER_BRIDGE "Enable $badger support." OFF)

add_library(${MODULE_NAME} SHARED 
    main.cpp
    WhiteListedOriginDomainsList.cpp
    Utils.cpp
    JavaScriptFunction.cpp
    NotifyWPEFramework.cpp
    Milestone.cpp
    ClassDefinition.cpp
    Tags.cpp
    RequestHeaders.cpp
)

if(PLUGIN_AMAZON_HAWAII)
    target_sources(${MODULE_NAME} PRIVATE HawaiiMessage.cpp)
    target_compile_definitions(${MODULE_NAME} PRIVATE SECURITY_AGENT_INTERFACE)
endif()

if(securityagent_FOUND)
    target_sources(${MODULE_NAME} PRIVATE SecurityAgent.cpp)
    target_link_libraries(${MODULE_NAME}
        PRIVATE 
            securityagent::securityagent)
endif()

if(PLUGIN_WEBKITBROWSER_AAMP_JSBINDINGS)
    find_package(AampJSBindings REQUIRED)
    target_sources(${MODULE_NAME} PRIVATE AAMPJSBindings.cpp)
    target_compile_definitions(${MODULE_NAME} PRIVATE ENABLE_AAMP_JSBINDINGS)
    target_link_libraries(${MODULE_NAME}
        PRIVATE
        ${AAMP_JSBINDINGS_LIBRARIES})
endif()

if(PLUGIN_WEBKITBROWSER_BADGER_BRIDGE)
    target_sources(${MODULE_NAME} PRIVATE BridgeObject.cpp)
    target_compile_definitions(${MODULE_NAME} PRIVATE ENABLE_BADGER_BRIDGE)
endif()

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME}
    PRIVATE 
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${GLIB_LIBRARIES}
        ${WPE_WEBKIT_LIBRARIES})

target_include_directories(${MODULE_NAME} 
    PRIVATE 
        ${WPE_WEBKIT_INCLUDE_DIRS}
        ${WPE_WEBKIT_INCLUDE_DIRS}/WPE
        ${GLIB_INCLUDE_DIRS})

install(TARGETS ${MODULE_NAME} 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${NAMESPACE}/WebKitBrowser
    COMPONENT ${MODULE_NAME})

if(PLUGIN_WEBKITBROWSER_AMAZON)
    install(TARGETS ${MODULE_NAME} 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${NAMESPACE}/Amazon
        COMPONENT ${MODULE_NAME})
endif()
