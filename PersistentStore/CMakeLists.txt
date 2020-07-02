set(PLUGIN_NAME PersistentStore)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)

find_package(PkgConfig)

# enabling the secure extension requires a license: https://www.hwaci.com/cgi-bin/see-step1
if (USE_SQLITE_SEE)
    find_package(SqliteSee REQUIRED)
else()
    find_package(Sqlite REQUIRED)
endif()

find_package(GLIB REQUIRED)

# nonce generator
if (BUILD_WITH_PLABELS AND USE_SQLITE_SEE)
    find_package(DL REQUIRED)
    find_package(Plabels REQUIRED)
endif()

add_library(${MODULE_NAME} SHARED
        PersistentStore.cpp
        Module.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

add_definitions(${SQLITE_CFLAGS_OTHER} ${PLABELS_FLAGS})
link_directories(${SQLITE_LIBRARY_DIRS})

target_include_directories(${MODULE_NAME} PRIVATE ../helpers
        ${SQLITE_INCLUDE_DIRS}
        ${PLABELS_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS})

target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${SQLITE_LIBRARIES}
        ${PLABELS_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${DL_LIBRARIES})

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
