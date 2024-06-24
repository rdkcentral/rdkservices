cmake_minimum_required(VERSION 3.3)

project(NetworkManager)
find_package(WPEFramework)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_PATCH 7)

add_compile_definitions(NETWORKMANAGER_MAJOR_VERSION=${VERSION_MAJOR})
add_compile_definitions(NETWORKMANAGER_MINOR_VERSION=${VERSION_MINOR})
add_compile_definitions(NETWORKMANAGER_PATCH_VERSION=${VERSION_PATCH})

set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


include(CmakeHelperFunctions)

string(TOLOWER ${NAMESPACE} STORAGE_DIRECTORY)
get_directory_property(SEVICES_DEFINES COMPILE_DEFINITIONS)

set(PLUGIN_NAME NetworkManager)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_LEGACY_DEPRECATED_NETWORK ${NAMESPACE}Network)
set(PLUGIN_LEGACY_DEPRECATED_WIFI    ${NAMESPACE}WiFiManager)

find_package(${NAMESPACE}Core REQUIRED)
find_package(${NAMESPACE}Plugins REQUIRED)
find_package(CURL)
if(ENABLE_GNOME_NETWORKMANAGER)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(LIBNM REQUIRED libnm)
else()
find_package(IARMBus REQUIRED)
endif ()


message("Setup ProxyStub for INetworkManager.h")
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(ProxyStubGenerator REQUIRED)

set(ProxyStubGenerator_DIR ${CMAKE_SYSROOT}${CMAKE_INSTALL_PREFIX}/tools/cmake ${ProxyStubGenerator_DIR})

if(NOT GENERATOR_SEARCH_PATH)
    set(GENERATOR_SEARCH_PATH ${CMAKE_SYSROOT}${CMAKE_INSTALL_PREFIX}/include/${NAMESPACE})
endif()

message("The Search path is, ${GENERATOR_SEARCH_PATH}")
ProxyStubGenerator(INPUT "${CMAKE_CURRENT_SOURCE_DIR}" OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/generated" INCLUDE_PATH ${GENERATOR_SEARCH_PATH})

file(GLOB PROXY_STUB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/generated/ProxyStubs*.cpp")

message("Setup ${PROJECT_NAME} v${PROJECT_VERSION}")

set(PLUGIN_NETWORKMANAGER_LOGLEVEL "5" CACHE STRING "To configure default loglevel NetworkManager plugin")


include_directories(${PROJECT_SOURCE_DIR})
# Build the main plugin that runs inside the WPEFramework daemon
add_library(${MODULE_NAME} SHARED
                            NetworkManager.cpp
                            NetworkManagerJsonRpc.cpp
                            NetworkManagerLogger.cpp
                            NetworkManagerImplementation.cpp
                            NetworkManagerConnectivity.cpp
                            NetworkManagerStunClient.cpp
                            WiFiSignalStrengthMonitor.cpp
                            Module.cpp
                            ${PROXY_STUB_SOURCES})

target_link_libraries(${MODULE_NAME}  PRIVATE
                                        ${NAMESPACE}Core::${NAMESPACE}Core
                                        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                                        )

set_target_properties(${MODULE_NAME} PROPERTIES
                                     CXX_STANDARD 11
                                     CXX_STANDARD_REQUIRED YES
                                     FRAMEWORK FALSE)

include_directories(${PROJECT_SOURCE_DIR}/interface)
include_directories(${PROJECT_SOURCE_DIR}/service)
include_directories(${PROJECT_SOURCE_DIR}/../helpers)

install(TARGETS ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)


# Build the implementation that runs out-of-process behind a COM-RPC interface

if(ENABLE_GNOME_NETWORKMANAGER)
    target_sources(${MODULE_NAME} PRIVATE NetworkManagerGnomeProxy.cpp NetworkManagerGnomeWIFI.cpp)
    target_include_directories(${MODULE_NAME} PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBNM_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${LIBNM_LIBRARIES})
else()
    target_sources(${MODULE_NAME} PRIVATE NetworkManagerRDKProxy.cpp)
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${IARMBUS_LIBRARIES})
endif()


target_link_libraries(${MODULE_NAME} PRIVATE ${CURL_LIBRARIES})
target_include_directories(${MODULE_NAME} PRIVATE ${CURL_INCLUDE_DIRS})


add_library(${PLUGIN_LEGACY_DEPRECATED_NETWORK} SHARED
        LegacyPlugin_NetworkAPIs.cpp
        NetworkManagerLogger.cpp
        Module.cpp
)

target_link_libraries(${PLUGIN_LEGACY_DEPRECATED_NETWORK}  PRIVATE
                                        ${NAMESPACE}Core::${NAMESPACE}Core
                                        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                                        )

set_target_properties(${PLUGIN_LEGACY_DEPRECATED_NETWORK} PROPERTIES
                                        CXX_STANDARD 11
                                        CXX_STANDARD_REQUIRED YES)

install(TARGETS ${PLUGIN_LEGACY_DEPRECATED_NETWORK} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

add_library(${PLUGIN_LEGACY_DEPRECATED_WIFI} SHARED
        LegacyPlugin_WiFiManagerAPIs.cpp
        NetworkManagerLogger.cpp
        Module.cpp
)

target_link_libraries(${PLUGIN_LEGACY_DEPRECATED_WIFI}  PRIVATE
                                        ${NAMESPACE}Core::${NAMESPACE}Core
                                        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
                                    )

set_target_properties(${PLUGIN_LEGACY_DEPRECATED_WIFI} PROPERTIES
                                        CXX_STANDARD 11
                                        CXX_STANDARD_REQUIRED YES)

install(TARGETS ${PLUGIN_LEGACY_DEPRECATED_WIFI} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

write_config()
write_config(PLUGINS LegacyPlugin_NetworkAPIs CLASSNAME Network LOCATOR lib${PLUGIN_LEGACY_DEPRECATED_NETWORK}.so)
write_config(PLUGINS LegacyPlugin_WiFiManagerAPIs CLASSNAME WiFiManager LOCATOR lib${PLUGIN_LEGACY_DEPRECATED_WIFI}.so)

