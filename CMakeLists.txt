# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
option(PLUGIN_SYSTEMSERVICES "Include SystemServices plugin" ON)
option(PLUGIN_HOMENETWORKING "Include HomeNetworking plugin" ON)
option(PLUGIN_PLUGANDPLAY "Include PlugAndPlay plugin" ON)
option(PLUGIN_VREXMANAGER "Include VrexManager plugin" ON)
option(PLUGIN_LINEARSEGMENTEDADVERTISING "Include LinearSegmentedAdvertising plugin" ON)
option(PLUGIN_DEEP_LINK "Include DeepLink plugin" ON)
option(PLUGIN_CONTROLSERVICE "Include ControlService plugin" ON)

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

if(PLUGIN_CONTROLSERVICE)
    add_subdirectory(ControlService)
endif()

if (NOT OEM_VENDOR_SAMSUNG) # Disabling these service until image oversize issue is fixed on Samsung Xg2v2
    if(PLUGIN_SYSTEMSERVICES)
        add_subdirectory(SystemServices)
    endif()

    if(PLUGIN_HOMENETWORKING)
        add_subdirectory(HomeNetworking)
    endif()

    if(PLUGIN_PLUGANDPLAY)
        add_subdirectory(PlugAndPlay)
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

    if(HAS_API_WEBSOCKET_PROXY)
        add_subdirectory(WebSocketProxy)
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

    if(HAS_API_SOUND)
        add_subdirectory(SoundPlayer)
    endif()

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

    if(HAS_API_CONTINUE_WATCHING)
        add_subdirectory(ContinueWatching)
    endif()

    if(HAS_API_APPLICATION)
        add_subdirectory(Application)
    endif()

    if(HAS_STATE_OBSERVER)
        add_subdirectory(StateObserver)
    endif()


    if(PLUGIN_WIFIMANAGER)
        add_subdirectory(WifiManager)
    endif()

    if(PLUGIN_LOSTANDFOUND)
        add_subdirectory(LostAndFound)
    endif()

    if(PLUGIN_LINEARSEGMENTEDADVERTISING)
        add_subdirectory(LinearSegmentedAdvertising)
    endif()

    if(PLUGIN_DEEP_LINK)
        add_subdirectory(DeepLink)
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
