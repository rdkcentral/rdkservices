set(PLUGIN_NAME WifiManager)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)

add_library(${MODULE_NAME} SHARED
        WifiManager.cpp
        Module.cpp
        impl/WifiManagerWPS.cpp
        impl/WifiManagerSignalThreshold.cpp
        impl/WifiManagerState.cpp
        impl/WifiManagerConnect.cpp
        impl/WifiManagerScan.cpp
        impl/WifiManagerEvents.cpp
        ../helpers/utils.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS} ../helpers)
target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES})


install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
