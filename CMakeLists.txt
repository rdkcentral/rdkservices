cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

# All packages that did not deliver a CMake Find script (and some deprecated scripts that need to be removed)
# are located in the cmake directory. Include it in the search.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

include(services.cmake)

option(PLUGIN_DISPLAYSETTINGS "Include DisplaySettings plugin" ON)
option(PLUGIN_TTSSETTINGS "Include TTSSettings plugin" ON)
option(PLUGIN_TTSRESOURCE "Include TTSResource Plugin plugin" ON)
option(PLUGIN_LOGGING_PREFERENCES "Include LoggingPreferences plugin" ON)
option(PLUGIN_USER_PREFERENCES "Include UserPreferences plugin" ON)
option(PLUGIN_PROXIES "Include Proxies plugin" ON)
option(PLUGIN_HOMENETWORKING "Include HomeNetworking plugin" ON)


# Library installation section
string(TOLOWER ${NAMESPACE} STORAGE_DIRECTORY)

# for writing pc and config files
include(CmakeHelperFunctions)

get_directory_property(SEVICES_DEFINES COMPILE_DEFINITIONS)

if(PLUGIN_DISPLAYSETTINGS)
    add_subdirectory(DisplaySettings)
endif()

if(PLUGIN_VREXMANAGER)
    add_subdirectory(VrexManager)
endif()

if (NOT OEM_VENDOR_SAMSUNG) # Disabling these service until image oversize issue is fixed on Samsung Xg2v2
    if(PLUGIN_HOMENETWORKING)
        add_subdirectory(HomeNetworking)
    endif()

    if(PLUGIN_LOGGING_PREFERENCES)
        add_subdirectory(LoggingPreferences)
    endif()

    if(PLUGIN_USER_PREFERENCES)
        add_subdirectory(UserPreferences)
    endif()

    if(PLUGIN_PROXIES)
        add_subdirectory(Proxies)
    endif()

    if(SEVICES_DEFINES MATCHES "HAS_FRONT_PANEL")
        add_subdirectory(FrontPanel)
    endif()

    if(HAS_API_COPILOT)
        add_subdirectory(CoPilot)
    endif()
    
    if(HAS_API_STORAGE_MANAGER)
        add_subdirectory(StorageManager)
    endif()

    if(DEVICE_PROVISIONING)
        add_subdirectory(DeviceProvisioning)
    endif()

    if(HAS_API_FRAME_RATE)
        add_subdirectory(FrameRate)
    endif()

    if(HAS_ACTIVITY_MONITOR)
        add_subdirectory(ActivityMonitor)
    endif()

    if(WAREHOUSE_API)
        add_subdirectory(Warehouse)
    endif()

    # if(HAS_API_HDMI_CEC)
    #   add_subdirectory(HdmiCec)
    # endif()

    if (HAS_API_DEVICEDIAGNOSTICS)
        add_subdirectory(DeviceDiagnostics)
    endif()

    if (ENABLE_HDCP_PROFILE)
        add_subdirectory(HdcpProfile)
    endif()

    if (HAS_API_HDMI_INPUT)
        add_subdirectory(HdmiInput)
    endif()

    if (HAS_TIMER)
        add_subdirectory(Timer)
    endif()

    if(SLEEP_TIMER)
        add_subdirectory(SleepTimer)
    endif()

    if(PLUGIN_TTSSETTINGS)
        add_subdirectory(TTSSettings)
    endif()

    if(PLUGIN_TTSRESOURCE)
        add_subdirectory(TTSResource)
    endif()

    if(HAS_STATE_OBSERVER)
        add_subdirectory(StateObserver)
    endif()

endif()

if(WPEFRAMEWORK_CREATE_IPKG_TARGETS)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_GROUPING IGNORE)

    set(CPACK_DEBIAN_PACKAGE_NAME "${WPEFRAMEWORK_PLUGINS_OPKG_NAME}")
    set(CPACK_DEBIAN_PACKAGE_VERSION "${WPEFRAMEWORK_PLUGINS_OPKG_VERSION}")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${WPEFRAMEWORK_PLUGINS_OPKG_ARCHITECTURE}")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${WPEFRAMEWORK_PLUGINS_OPKG_MAINTAINER}")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${WPEFRAMEWORK_PLUGINS_OPKG_DESCRIPTION}")
    set(CPACK_PACKAGE_FILE_NAME "${WPEFRAMEWORK_PLUGINS_OPKG_FILE_NAME}")

    # list of components from which packages will be generated
    set(CPACK_COMPONENTS_ALL
            ${NAMESPACE}WebKitBrowser
            WPEInjectedBundle
            )

    include(CPack)
endif()
