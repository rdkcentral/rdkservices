cmake_minimum_required(VERSION 3.3)

project(NetworkManager)
find_package(WPEFramework)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

add_compile_definitions(NETWORKMANAGER_MAJOR_VERSION=${VERSION_MAJOR})
add_compile_definitions(NETWORKMANAGER_MINOR_VERSION=${VERSION_MINOR})
add_compile_definitions(NETWORKMANAGER_PATCH_VERSION=${VERSION_PATCH})

set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


include(CmakeHelperFunctions)

string(TOLOWER ${NAMESPACE} STORAGE_DIRECTORY)
get_directory_property(SEVICES_DEFINES COMPILE_DEFINITIONS)

add_subdirectory(interface)
add_subdirectory(service)

