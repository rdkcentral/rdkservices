set(PLUGIN_NAME SystemAudioPlayer)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)

add_subdirectory(test)

add_library(${MODULE_NAME} SHARED
        Module.cpp
        SystemAudioPlayer.cpp
        SystemAudioPlayerJsonRpc.cpp
        ProxyStubs_SystemAudioPlayer.cpp
        SystemAudioPlayerImplementation.cpp
        impl/AudioPlayer.cpp
        impl/BufferQueue.cpp
        impl/WebSocketClient.cpp
        impl/logger.cpp
        )
set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMERBASE REQUIRED gstreamer-app-1.0)
pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)

find_package(GSTREAMER REQUIRED)

find_package(Curl)

set(AUDIO_CLIENT_LIB "")
if (BUILD_AMLOGIC)
set(AUDIO_CLIENT_LIB "audio_client")
endif()

target_include_directories(${MODULE_NAME} PRIVATE ../helpers ${GSTREAMER_INCLUDES} ${GSTREAMERBASE_INCLUDE_DIRS} ${LIBWEBSOCKETS_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${CURL_LIBRARY} ${GSTREAMER_LIBRARIES} ${GSTREAMERBASE_LIBRARIES} ${AUDIO_CLIENT_LIB} ${LIBWEBSOCKETS_LIBRARIES} trower-base64)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config( SystemAudioPlayer )
