set(PLUGIN_NAME OCIContainer)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_OCICONTAINER_STARTUPORDER "" CACHE STRING "To configure startup order of OCIContainer plugin")

find_package(PkgConfig)
find_package(${NAMESPACE}Plugins REQUIRED)
find_package(Dobby CONFIG)

# Temporary fix to get defines in Dobby. Will be removed later.
add_definitions( -DRDK )

pkg_search_module(OMI_CLIENT "omiclientlib")

if(NOT OMI_CLIENT_FOUND)
    set(OMI_CLIENT_INCLUDE_DIRS stubs)
    message("Using stubs for omiclientlib")
endif()

add_library(${MODULE_NAME} SHARED
        OCIContainer.cpp
        Module.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# We need libsystemd since we're using their dbus library (sd-bus)
# There's probably a better way of doing this...
find_path(SYSTEMD_INCLUDE_DIRS
    NAMES systemd/sd-bus.h
)

find_library(SYSTEMD_LIBRARIES
    NAMES systemd
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LIBSYSTEMD
    SYSTEMD_LIBRARIES SYSTEMD_INCLUDE_DIRS
)


target_include_directories(${MODULE_NAME}
        PRIVATE
        ../helpers
        ${OMI_CLIENT_INCLUDE_DIRS}
)


target_link_libraries(${MODULE_NAME}
        PRIVATE

        ${DOBBY_LIBRARIES}
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${SYSTEMD_LIBRARIES}
        ${OMI_CLIENT_LIBRARIES}
)
# ${NAMESPACE}Protocols::${NAMESPACE}Protocols
install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
