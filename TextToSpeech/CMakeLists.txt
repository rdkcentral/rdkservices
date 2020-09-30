set(PLUGIN_NAME TextToSpeech)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)

add_subdirectory(test)

add_library(${MODULE_NAME} SHARED
        TextToSpeech.cpp
        Module.cpp
        impl/TTSManager.cpp
        impl/TTSSpeaker.cpp
        impl/logger.cpp
        )
set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(GSTREAMER REQUIRED)

find_package(Curl)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers ${GSTREAMER_INCLUDES})
target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${CURL_LIBRARY} ${GSTREAMER_LIBRARIES})

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config( TextToSpeech )
