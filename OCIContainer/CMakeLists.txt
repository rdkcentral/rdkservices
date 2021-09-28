set(PLUGIN_NAME OCIContainer)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Dobby REQUIRED CONFIG)

# Temporary fix to get defines in Dobby. Will be removed later.
add_definitions( -DRDK )

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
        )

target_link_libraries(${MODULE_NAME}
        PRIVATE

        # Dobby libraries
        DobbyClientLib
        IpcService
        AppInfraCommon
        AppInfraLogging
        JsonCpp::JsonCpp

        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${SYSTEMD_LIBRARIES}
)


find_library(OMIPROXY omiclientlib)
if(OMIPROXY)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I=/usr/include/glib-2.0 -I=/usr/lib/glib-2.0/include")
  target_link_libraries (${MODULE_NAME} PRIVATE omiclientlib)

else()

  target_include_directories(${MODULE_NAME} PRIVATE stubs)

endif (OMIPROXY)

# ${NAMESPACE}Protocols::${NAMESPACE}Protocols
install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
