set(PLUGIN_NAME remoteControlTestClient)
find_package(${NAMESPACE}Protocols REQUIRED)

add_executable(${PLUGIN_NAME} remoteControlTestClient.cpp)

set_target_properties(${PLUGIN_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    )

target_link_libraries(${PLUGIN_NAME}
    PRIVATE
    ${NAMESPACE}Protocols::${NAMESPACE}Protocols
    )

install(TARGETS ${PLUGIN_NAME} DESTINATION bin)
