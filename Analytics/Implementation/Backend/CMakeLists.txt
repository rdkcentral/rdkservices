
set(TARGET_LIB ${NAMESPACE}${PLUGIN_NAME}Backends)

add_library(${TARGET_LIB} STATIC)

if (PLUGIN_ANALYTICS_SIFT_BACKEND)
    add_subdirectory(Sift)
    target_link_libraries(${TARGET_LIB} PRIVATE ${NAMESPACE}${PLUGIN_NAME}SiftBackend)
    target_compile_definitions(${TARGET_LIB} PRIVATE ANALYTICS_SIFT_BACKEND=1)
endif()

target_sources(${TARGET_LIB} PRIVATE AnalyticsBackend.cpp)
target_include_directories(${TARGET_LIB} PUBLIC "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TARGET ${TARGET_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(${TARGET_LIB} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_link_libraries(${TARGET_LIB} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

