set(PLUGIN_NAME remoteControlTestClient)
if (USE_THUNDER_R4)
    find_package(${NAMESPACE}COM REQUIRED)
else ()
    find_package(${NAMESPACE}Protocols REQUIRED)
endif (USE_THUNDER_R4)

add_executable(${PLUGIN_NAME} remoteControlTestClient.cpp)

set_target_properties(${PLUGIN_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    )

if (USE_THUNDER_R4)
target_link_libraries(${PLUGIN_NAME} PRIVATE ${NAMESPACE}COM::${NAMESPACE}COM  ${NAMESPACE}WebSocket::${NAMESPACE}WebSocket)
else ()
target_link_libraries(${PLUGIN_NAME}
    PRIVATE
    ${NAMESPACE}Protocols::${NAMESPACE}Protocols
    )
endif (USE_THUNDER_R4)


install(TARGETS ${PLUGIN_NAME} DESTINATION bin)
