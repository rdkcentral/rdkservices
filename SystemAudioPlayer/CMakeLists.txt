set(PLUGIN_NAME SystemAudioPlayer)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_SYSTEMAUDIOPLAYER_AUTOSTART "true" CACHE STRING "Automatically start SystemAudioPlayer plugin")
set(PLUGIN_SYSTEMAUDIOPLAYER_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")

find_package(${NAMESPACE}Plugins REQUIRED)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(websocketpp REQUIRED)
find_package(OpenSSL REQUIRED)

add_subdirectory(test)

add_library(${MODULE_NAME} SHARED
        Module.cpp
        SystemAudioPlayer.cpp
        SystemAudioPlayerJsonRpc.cpp
        ProxyStubs_SystemAudioPlayer.cpp
        SystemAudioPlayerImplementation.cpp
        impl/AudioPlayer.cpp
        impl/BufferQueue.cpp
        impl/SecuredWebSocketClient.cpp
        impl/UnsecuredWebSocketClient.cpp
        impl/logger.cpp
        ../helpers/WebSockets/WSEndpoint.cpp
        ../helpers/WebSockets/JsonRpc/Request.cpp
        ../helpers/WebSockets/JsonRpc/Response.cpp
        ../helpers/WebSockets/JsonRpc/Notification.cpp
        )
set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMERBASE REQUIRED gstreamer-app-1.0)

find_package(GSTREAMER REQUIRED)

find_package(Curl)

set(AUDIO_CLIENT_LIB "")
if (BUILD_AMLOGIC)
    set(AUDIO_CLIENT_LIB "audio_client")
    target_sources(${MODULE_NAME}
        PRIVATE
            impl/amlogic/SoC_abstraction.cpp
    )
elseif (BUILD_REALTEK)
    set(AUDIO_CLIENT_LIB "RtkHALMisc")
    target_sources(${MODULE_NAME}
        PRIVATE
            impl/realtek/SoC_abstraction.cpp
    )
endif()

target_include_directories(${MODULE_NAME} PRIVATE ../helpers)
target_include_directories(${MODULE_NAME} PRIVATE ../helpers/WebSockets)
target_include_directories(${MODULE_NAME} PRIVATE ${GSTREAMER_INCLUDES})
target_include_directories(${MODULE_NAME} PRIVATE ${GSTREAMERBASE_INCLUDE_DIRS})

target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)
target_link_libraries(${MODULE_NAME} PRIVATE ${CURL_LIBRARY})
target_link_libraries(${MODULE_NAME} PRIVATE ${GSTREAMER_LIBRARIES})
target_link_libraries(${MODULE_NAME} PRIVATE ${GSTREAMERBASE_LIBRARIES})
target_link_libraries(${MODULE_NAME} PRIVATE ${AUDIO_CLIENT_LIB})
target_link_libraries(${MODULE_NAME} PRIVATE trower-base64)
target_link_libraries(${MODULE_NAME} PRIVATE ${Boost_SYSTEM_LIBRARY})
target_link_libraries(${MODULE_NAME} PRIVATE OpenSSL::SSL)
target_link_libraries(${MODULE_NAME} PRIVATE OpenSSL::Crypto)
target_link_libraries(${MODULE_NAME} PRIVATE ${Boost_FILESYSTEM_LIBRARY})

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config( SystemAudioPlayer )
