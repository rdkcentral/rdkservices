set(PLUGIN_NAME UsbAccess)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)

find_package(PkgConfig)
find_package(Udev)
find_package(IARMBus)

add_library(${MODULE_NAME} SHARED
        UsbAccess.cpp
        Module.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_include_directories(${MODULE_NAME} PRIVATE
        ../helpers
        ${UDEV_INCLUDE_DIRS}
        ${IARMBUS_INCLUDE_DIRS})

link_directories(${UDEV_LIBRARY_DIRS})

target_link_libraries(${MODULE_NAME} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${UDEV_LIBRARIES}
        ${IARMBUS_LIBRARIES})

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
