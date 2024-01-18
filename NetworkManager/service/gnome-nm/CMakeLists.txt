find_package(IARMBus)
find_package(${NAMESPACE}Core REQUIRED)
find_package(${NAMESPACE}Plugins REQUIRED)

add_library(
    NMWrapper
    SHARED
    NetworkManagerGnomeProxy.cpp)

target_link_libraries(NMWrapper
        PRIVATE
        ${NAMESPACE}Core::${NAMESPACE}Core
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
)


set_target_properties(NMWrapper PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_include_directories(NMWrapper PRIVATE ${IARMBUS_INCLUDE_DIRS})
target_link_libraries(NMWrapper PRIVATE ${IARMBUS_LIBRARIES})

set_target_properties(NMWrapper PROPERTIES SOVERSION "0" VERSION "${PROJECT_VERSION}")

install (TARGETS NMWrapper  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

