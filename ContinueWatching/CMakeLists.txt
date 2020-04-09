set(PLUGIN_NAME ContinueWatching)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_CONTINUEWATCHING_AUTOSTART true CACHE STRING "Automatically start ContinueWatching plugin")

find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
        ContinueWatching.cpp
        Module.cpp)

#add_subdirectory (test)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins crypto sec_api rfcapi trower-base64) 
target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
