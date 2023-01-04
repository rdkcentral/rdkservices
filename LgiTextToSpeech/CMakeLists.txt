set(PLUGIN_NAME TextToSpeech)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_TEXTTOSPEECH_AUTOSTART "true" CACHE STRING "Automatically start TestToSpeech plugin")
set(PLUGIN_TEXTTOSPEECH_STARTUPORDER "" CACHE STRING "Automatically start TextToSpeech plugin")
set(PLUGIN_TEXTTOSPEECH_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(PkgConfig)
pkg_search_module(GIO REQUIRED "gio-2.0 gio-unix-2.0")

add_library(${MODULE_NAME} SHARED
        Module.cpp
        TextToSpeech.cpp
        TextToSpeechJsonRpc.cpp
        ProxyStubs_TextToSpeech.cpp
        TextToSpeechImplementation.cpp
        impl/TTSManager.cpp
        impl/logger.cpp
        )
set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

target_include_directories(${MODULE_NAME} PRIVATE ${GIO_INCLUDE_DIRS} ../helpers)
target_link_libraries(${MODULE_NAME} PRIVATE
    ${NAMESPACE}Plugins::${NAMESPACE}Plugins ttsclient)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config( TextToSpeech )
