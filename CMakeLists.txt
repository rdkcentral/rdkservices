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

option(PLUGIN_PACKAGER "Include Packager plugin" OFF)
option(PLUGIN_DISPLAYSETTINGS "Include DisplaySettings plugin" ON)
option(PLUGIN_LOGGING_PREFERENCES "Include LoggingPreferences plugin" ON)
option(PLUGIN_USER_PREFERENCES "Include UserPreferences plugin" ON)
option(PLUGIN_PROXIES "Include Proxies plugin" OFF)
#option(PLUGIN_DATA_CAPTURE "Include DataCapture plugin" ON)
option(PLUGIN_AVINPUT "Include AVInput plugin" OFF)
option(PLUGIN_SYSTEMSERVICES "Include SystemServices plugin" ON)
option(PLUGIN_HOMENETWORKING "Include HomeNetworking plugin" OFF)
option(PLUGIN_REMOTEACTIONMAPPING "Include RemoteActionMapping plugin" OFF)
option(PLUGIN_PLUGANDPLAY "Include PlugAndPlay plugin" OFF)
option(PLUGIN_VREXMANAGER "Include VrexManager plugin" OFF)
option(PLUGIN_PLATFORMPLAYER "Include PlatformPlayer plugin" OFF)
option(PLUGIN_LINEARSEGMENTEDADVERTISING "Include LinearSegmentedAdvertising plugin" OFF)
option(PLUGIN_DEEP_LINK "Include DeepLink plugin" OFF)
option(PLUGIN_CONTROLSERVICE "Include ControlService plugin" ON)
option(PLUGIN_VIDEOAPPLICATIONEVENTS "Include video application events plugin" OFF)
option(PLUGIN_INACTIVITYNOTIFIER "Include InactivityNotifier plugin" OFF)
option(PLUGIN_RDKSHELL "Include RDKShell plugin" OFF)
option(PLUGIN_TEXTTOSPEECH "Include TextToSpeech plugin" OFF)
option(PLUGIN_OCICONTAINER "Include OCIContainer plugin" OFF)

# Library installation section
string(TOLOWER ${NAMESPACE} STORAGE_DIRECTORY)

# for writing pc and config files
include(CmakeHelperFunctions)

get_directory_property(SEVICES_DEFINES COMPILE_DEFINITIONS)

if(PLUGIN_PACKAGER)
    add_subdirectory(Packager)
endif()

if(PLUGIN_BLUETOOTH)
    add_subdirectory(Bluetooth)
endif()

if(PLUGIN_DISPLAYSETTINGS)
    add_subdirectory(DisplaySettings)
endif()

if(PLUGIN_REMOTEACTIONMAPPING)
    add_subdirectory(RemoteActionMapping)
endif()

if(PLUGIN_CONTROLSERVICE)
    add_subdirectory(ControlService)
endif()

    if(PLUGIN_SYSTEMSERVICES)
        add_subdirectory(SystemServices)
    endif()

    if(PLUGIN_HOMENETWORKING)
        add_subdirectory(HomeNetworking)
    endif()

    if(PLUGIN_NETWORK)
        add_subdirectory(Network)
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

    if(PLUGIN_AVINPUT)
        add_subdirectory(AVInput)
    endif()

    if(PLUGIN_DATA_CAPTURE)
        add_subdirectory(DataCapture)
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

    # if(BLUETOOTH_SETTINGS)
    #    add_subdirectory(BluetoothSettings)
    # endif()

    if(HAS_API_FRAME_RATE)
        add_subdirectory(FrameRate)
    endif()

    if(HAS_API_SCREEN_CAPTURE)
        add_subdirectory(ScreenCapture)
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

    if(HAS_API_HDMI_CEC)
       add_subdirectory(HdmiCec)
    endif()

    if(HAS_API_HDMI_CEC_2)
       add_subdirectory(HdmiCec_2)
    endif()

    if(HAS_API_HDMI_CEC_SINK)
       add_subdirectory(HdmiCecSink)
    endif()


    if(ENABLE_LOCATION_SYNC)
        add_subdirectory(LocationSync)
    endif()

    if (ENABLE_DEVICE_IDENTIFICATION)
        add_subdirectory(DeviceIdentification)
    endif()

    if (ENABLE_DEVICE_INFO)
        add_subdirectory(DeviceInfo)
    endif()

    if (ENABLE_DISPLAY_INFO)
        add_subdirectory(DisplayInfo)
    endif()

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

    if(ENABLE_MESSENGER)
        add_subdirectory(Messenger)
    endif()

    if(ENABLE_MONITOR)
        add_subdirectory(Monitor)
    endif()

    if(ENABLE_OPEN_CDMI)
        add_subdirectory(OpenCDMi)
    endif()

    if(ENABLE_PLAYER_INFO)
        add_subdirectory(PlayerInfo)
    endif()

    if(ENABLE_SECURITY_AGENT)
        add_subdirectory(SecurityAgent)
    endif()

    if(HAS_AUTHSERVICE)
        add_subdirectory(AuthService)
    endif()

    if(HAS_API_CONTINUE_WATCHING)
        add_subdirectory(ContinueWatching)
    endif()

    #if(HAS_API_APPLICATION)
    #    add_subdirectory(Application)
    #endif()

    if(HAS_STATE_OBSERVER)
        add_subdirectory(StateObserver)
    endif()

    if(PLUGIN_WIFIMANAGER)
        add_subdirectory(WifiManager)
    endif()

    if(ENABLE_TRACE_CONTROL)
        add_subdirectory(TraceControl)
    endif()

    if(PLUGIN_LOSTANDFOUND)
        add_subdirectory(LostAndFound)
    endif()

    if(PLUGIN_PLATFORMPLAYER)
        add_subdirectory(PlatformPlayer)
    endif()

    if(PLUGIN_LINEARSEGMENTEDADVERTISING)
        add_subdirectory(LinearSegmentedAdvertising)
    endif()

    if(PLUGIN_DEEP_LINK)
        add_subdirectory(DeepLink)
    endif()

    if(HAS_API_XCASTSERVICE)
        add_subdirectory(XCast)
    endif()

    if(PLUGIN_VIDEOAPPLICATIONEVENTS)
        add_subdirectory(VideoApplicationEvents)
    endif()

    if(PLUGIN_INACTIVITYNOTIFIER)
        add_subdirectory(InactivityNotifier)
    endif()

    if(PLUGIN_RDKSHELL)
        add_subdirectory(RDKShell)
    endif()

    if(ENABLE_WEBKIT_BROWSER)
        add_subdirectory(WebKitBrowser)
    endif()

    if(PLUGIN_TEXTTOSPEECH)
        add_subdirectory(TextToSpeech)
    endif()

if(PLUGIN_PERSISTENT_STORE)
    add_subdirectory(PersistentStore)
endif()

if(PLUGIN_OCICONTAINER)
    add_subdirectory(OCIContainer)
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
