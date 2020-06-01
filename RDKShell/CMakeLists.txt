set(PLUGIN_NAME RDKShell)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
        RDKShell.cpp
        Module.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

set(RDKSHELL_INCLUDES $ENV{RDKSHELL_INCLUDES})
separate_arguments(RDKSHELL_INCLUDES)
include_directories(BEFORE ${RDKSHELL_INCLUDES})
target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins -lrdkshell)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
