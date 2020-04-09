find_package(jsonrpc REQUIRED)

add_executable(CWTestThunderClient CWTestThunderClient.cpp)

set_target_properties(CWTestThunderClient PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        )


target_link_libraries(CWTestThunderClient
    PRIVATE
    jsonrpc::jsonrpc
        )

install(TARGETS CWTestThunderClient DESTINATION bin)
