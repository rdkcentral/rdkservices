project(NetworkManagerInterface)
find_package(WPEFramework REQUIRED)
find_package(${NAMESPACE}Core REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(ProxyStubGenerator REQUIRED)

set(ProxyStubGenerator_DIR ${CMAKE_SYSROOT}${CMAKE_INSTALL_PREFIX}/tools/cmake ${ProxyStubGenerator_DIR})

if(NOT GENERATOR_SEARCH_PATH)
    set(GENERATOR_SEARCH_PATH ${CMAKE_SYSROOT}${CMAKE_INSTALL_PREFIX}/include/${NAMESPACE})
endif()

ProxyStubGenerator(INPUT "${CMAKE_CURRENT_SOURCE_DIR}" OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/generated" INCLUDE_PATH ${GENERATOR_SEARCH_PATH})

file(GLOB INTERFACES_HEADERS I*.h)
list(APPEND INTERFACES_HEADERS Module.h)

file(GLOB PROXY_STUB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/generated/ProxyStubs*.cpp")

set(TARGET ${NAMESPACE}NetworkManagerInterface)
string(TOLOWER ${NAMESPACE} NAMESPACE_LIB)

add_library(${TARGET} SHARED ${PROXY_STUB_SOURCES} Module.cpp)

target_include_directories( ${TARGET} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(${TARGET} PRIVATE ${NAMESPACE}Core::${NAMESPACE}Core)
target_link_libraries(${TARGET} PRIVATE CompileSettingsDebug::CompileSettingsDebug)

set_target_properties(${TargetMarshalling} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        FRAMEWORK FALSE
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        )

install(
        TARGETS ${TARGET}  EXPORT ${TARGET}Targets  # for downstream dependencies
        ARCHIVE DESTINATION lib/${NAMESPACE_LIB}/proxystubs COMPONENT libs      # static lib
        LIBRARY DESTINATION lib/${NAMESPACE_LIB}/proxystubs COMPONENT libs      # shared lib
        RUNTIME DESTINATION bin COMPONENT libs      # binaries
        FRAMEWORK DESTINATION bin/${NAMESPACE_LIB}/proxystubs COMPONENT libs    # for mac
        PUBLIC_HEADER DESTINATION include/${NAMESPACE}/proxystubs COMPONENT devel   # headers for mac (note the different component -> different package)
        INCLUDES DESTINATION include/${NAMESPACE}/proxystubs      # headers
)

InstallCMakeConfig(TARGETS ${TARGET})
