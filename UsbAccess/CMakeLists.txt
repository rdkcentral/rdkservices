set(PLUGIN_NAME UsbAccess)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_USBACCESS_STARTUPORDER "" CACHE STRING "To configure startup order of UsbAccess plugin")
set(PLUGIN_USBACCESS_AUTOSTART false CACHE STRING "To automatically start UsbAccess plugin.")

find_package(${NAMESPACE}Plugins REQUIRED)

find_package(PkgConfig)
find_package(Udev)
find_package(IARMBus)

add_library(${MODULE_NAME} SHARED
        UsbAccess.cpp
        Module.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

if (RDK_SERVICE_L2_TEST)
   find_library(TESTMOCKLIB_LIBRARIES NAMES TestMocklib)
   if (TESTMOCKLIB_LIBRARIES)
       message ("linking mock libraries ${TESTMOCKLIB_LIBRARIES} library")
       target_link_libraries(${MODULE_NAME} PRIVATE ${TESTMOCKLIB_LIBRARIES})
   else (TESTMOCKLIB_LIBRARIES)
       message ("Require ${TESTMOCKLIB_LIBRARIES} library")
   endif (TESTMOCKLIB_LIBRARIES)
endif (RDK_SERVICES_L2_TEST)

target_include_directories(${MODULE_NAME} PRIVATE
        ../helpers
        ${UDEV_INCLUDE_DIRS}
        ${IARMBUS_INCLUDE_DIRS})

link_directories(${UDEV_LIBRARY_DIRS})

target_link_libraries(${MODULE_NAME} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${UDEV_LIBRARIES}
        ${IARMBUS_LIBRARIES})

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
