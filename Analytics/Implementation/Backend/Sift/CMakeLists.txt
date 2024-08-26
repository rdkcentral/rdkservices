set(TARGET_LIB ${NAMESPACE}${PLUGIN_NAME}SiftBackend)

add_library(${TARGET_LIB} STATIC)

target_sources(${TARGET_LIB} PRIVATE SiftBackend.cpp SiftConfig.cpp)
    
find_package(CURL)
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(${TARGET_LIB} PRIVATE ${CURL_LIBRARIES})
else (CURL_FOUND)
    message ("Curl/libcurl required.")
endif (CURL_FOUND)

target_include_directories(${TARGET_LIB} PUBLIC "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${TARGET_LIB} PRIVATE ../../../../helpers)
set_property(TARGET ${TARGET_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(${TARGET_LIB} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_link_libraries(${TARGET_LIB} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)