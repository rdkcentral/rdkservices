set(PLUGIN_NAME UsbAccess)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)

find_package(PkgConfig)

add_library(${MODULE_NAME} SHARED
        UsbAccess.cpp
        Module.cpp
        ../helpers/utils.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

find_package(IARMBus REQUIRED)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers ${IARMBUS_INCLUDE_DIRS})

target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES})

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
