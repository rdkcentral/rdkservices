set(PLUGIN_NAME Proxies)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(GLIB REQUIRED)

add_library(${MODULE_NAME} SHARED
        Proxies.cpp
        Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

find_library(GLIB_LIBRARY NAMES glib-2.0 )

target_include_directories(${MODULE_NAME}
    PRIVATE
        ../helpers
        ${GLIB_INCLUDE_DIRS}
)

target_link_libraries(${MODULE_NAME}
    PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${GLIB_LIBRARIES}
)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
