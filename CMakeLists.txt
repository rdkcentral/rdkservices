###
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2023 RDK Management
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
###

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

# All packages that did not deliver a CMake Find script (and some deprecated scripts that need to be removed)
# are located in the cmake directory. Include it in the search.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

if(RDK_SERVICE_L2_TEST)
    include(l2tests.cmake)
endif()

option(COMCAST_CONFIG "Comcast services configuration" ON)
if(COMCAST_CONFIG)
    include(services.cmake)
endif()

option(PLUGIN_OCICONTAINER "Include OCIContainer plugin" OFF)

if(RDK_SERVICES_L1_TEST)
    include(l1tests.cmake)
endif()

# Library installation section
string(TOLOWER ${NAMESPACE} STORAGE_DIRECTORY)

# for writing pc and config files
include(CmakeHelperFunctions)

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

if(ENABLE_COMMUNITY_DEVICE_TYPE)
   add_definitions(-DENABLE_COMMUNITY_DEVICE_TYPE)
endif()


if(PLUGIN_CONTROLSERVICE)
    add_subdirectory(ControlService)
endif()

if(PLUGIN_SYSTEMMODE)
    add_subdirectory(SystemMode)
endif()

if(PLUGIN_USERSETTINGS)
    add_subdirectory(UserSettings)
endif()

if(PLUGIN_VOICECONTROL)
    add_subdirectory(VoiceControl)
endif()

if(PLUGIN_REMOTECONTROL)
	add_subdirectory(RemoteControl)
endif()

if(PLUGIN_SYSTEMSERVICES)
    add_subdirectory(SystemServices)
endif()

if(RDK_SERVICE_L2_TEST)
    add_subdirectory(Tests/L2Tests/L2TestsPlugin)
    add_subdirectory(Tests/L2Tests)
endif()

if(PLUGIN_MAINTENANCEMANAGER)
    add_subdirectory(MaintenanceManager)
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

if(PLUGIN_LOGGINGPREFERENCES)
    add_subdirectory(LoggingPreferences)
endif()

if(PLUGIN_USERPREFERENCES)
    add_subdirectory(UserPreferences)
endif()

if(PLUGIN_PROXIES)
    add_subdirectory(Proxies)
endif()

if(PLUGIN_AVINPUT)
    add_subdirectory(AVInput)
endif()

if(PLUGIN_AVOUTPUT)
    add_subdirectory(AVOutput)
endif()

if(PLUGIN_DATACAPTURE)
    add_subdirectory(DataCapture)
endif()

if(PLUGIN_FRONTPANEL OR HAS_FRONT_PANEL)
    add_subdirectory(FrontPanel)
endif()

if(PLUGIN_COPILOT)
    add_subdirectory(CoPilot)
endif()

if(PLUGIN_STORAGE_MANAGER)
    add_subdirectory(StorageManager)
endif()

if(PLUGIN_DEVICE_PROVISIONING)
    add_subdirectory(DeviceProvisioning)
endif()


# if(BLUETOOTH_SETTINGS)
#    add_subdirectory(BluetoothSettings)
# endif()

if(PLUGIN_FRAMERATE)
    add_subdirectory(FrameRate)
endif()

if(PLUGIN_SCREENCAPTURE)
    add_subdirectory(ScreenCapture)
endif()

if(PLUGIN_WEBSOCKETPROXY)
    add_subdirectory(WebSocketProxy)
endif()

if(PLUGIN_ACTIVITYMONITOR)
    add_subdirectory(ActivityMonitor)
endif()

if(PLUGIN_TELEMETRY)
    add_subdirectory(Telemetry)
endif()

if(PLUGIN_WAREHOUSE)
    add_subdirectory(Warehouse)
endif()

if(PLUGIN_HDMICEC)
    add_subdirectory(HdmiCec)
endif()

if(PLUGIN_HDMICEC2)
    add_subdirectory(HdmiCec_2)
endif()

if(PLUGIN_HDMICECSOURCE)
    add_subdirectory(HdmiCecSource)
endif()

if(PLUGIN_HDMICECSINK)
    add_subdirectory(HdmiCecSink)
endif()

if(PLUGIN_LOCATIONSYNC)
    add_subdirectory(LocationSync)
endif()

if(PLUGIN_DEVICEIDENTIFICATION)
    add_subdirectory(DeviceIdentification)
endif()

if(PLUGIN_DEVICEINFO)
    add_subdirectory(DeviceInfo)
endif()

if(PLUGIN_DISPLAYINFO)
    add_subdirectory(DisplayInfo)
endif()

if(PLUGIN_SOUNDPLAYER)
    add_subdirectory(SoundPlayer)
endif()

if(PLUGIN_DEVICEDIAGNOSTICS)
    add_subdirectory(DeviceDiagnostics)
endif()

if(PLUGIN_HDCPPROFILE)
    add_subdirectory(HdcpProfile)
endif()

if(PLUGIN_HDMIINPUT)
    add_subdirectory(HdmiInput)
endif()

if(PLUGIN_COMPOSITEINPUT)
	add_subdirectory(CompositeInput)
endif()

if(PLUGIN_TIMER)
    add_subdirectory(Timer)
endif()

if(PLUGIN_SLEEPTIMER)
    add_subdirectory(SleepTimer)
endif()

if(PLUGIN_MESSENGER)
    add_subdirectory(Messenger)
endif()

if(PLUGIN_MONITOR)
    add_subdirectory(Monitor)
endif()

if(PLUGIN_OPENCDMI)
    add_subdirectory(OpenCDMi)
endif()

if(PLUGIN_MOTION_DETECTION)
    add_subdirectory(MotionDetection)
endif()

if(PLUGIN_PLAYERINFO)
    add_subdirectory(PlayerInfo)
endif()

if(PLUGIN_SECURITYAGENT)
    add_subdirectory(SecurityAgent)
endif()

if(HAS_AUTHSERVICE)
    add_subdirectory(AuthService)
endif()

if(PLUGIN_CONTINUEWATCHING)
    add_subdirectory(ContinueWatching)
endif()

#if(HAS_API_APPLICATION)
#    add_subdirectory(Application)
#endif()

if(PLUGIN_STATEOBSERVER)
    add_subdirectory(StateObserver)
endif()

if(PLUGIN_WIFIMANAGER)
    add_subdirectory(WifiManager)
endif()

if(PLUGIN_TRACECONTROL)
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

if(PLUGIN_DEEPLINK)
    add_subdirectory(DeepLink)
endif()

if(PLUGIN_XCAST)
    add_subdirectory(XCast)
endif()

if(PLUGIN_MIRACAST)
	add_subdirectory(Miracast)
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

if(PLUGIN_WEBKITBROWSER)
    add_subdirectory(WebKitBrowser)
endif()

if(PLUGIN_TEXTTOSPEECH)
    add_subdirectory(TextToSpeech)
endif()

if(PLUGIN_SYSTEMAUDIOPLAYER)
    add_subdirectory(SystemAudioPlayer)
endif()

if(PLUGIN_PERSISTENTSTORE)
    add_subdirectory(PersistentStore)
endif()

if(PLUGIN_OCICONTAINER)
    add_subdirectory(OCIContainer)
endif()

if(PLUGIN_USBACCESS)
    add_subdirectory(UsbAccess)
endif()

if(PLUGIN_DTV)
    add_subdirectory(DTV)
endif()

if(PLUGIN_FIREBOLTMEDIAPLAYER)
    add_subdirectory(FireboltMediaPlayer)
endif()

if(PLUGIN_WEBBRIDGE)
    add_subdirectory(WebBridge)
endif()

if(PLUGIN_LINEARPLAYBACKCONTROL)
    add_subdirectory(LinearPlaybackControl)
endif()

if(PLUGIN_RUSTADAPTER)
    add_subdirectory(RustAdapter)
endif()

if(RDK_SERVICES_L1_TEST)
    add_subdirectory(Tests/L1Tests)
endif()

if(PLUGIN_PERFORMANCEMETRICS)
    add_subdirectory(PerformanceMetrics)
endif()

if(PLUGIN_CRYPTOGRAPHY)
    add_subdirectory(Cryptography)
endif()

if(PLUGIN_UNIFIEDCASMANAGEMENT)
    add_subdirectory(UnifiedCASManagement)
endif()

if(PLUGIN_MEDIAENGINERMF)
    add_subdirectory(MediaEngineRMF)
endif()

if(PLUGIN_LEDCONTROL)
    add_subdirectory(LEDControl)
endif()

if(PLUGIN_MESSAGECONTROL)
    add_subdirectory(MessageControl)
endif()

if (PLUGIN_RESOURCEMANAGER)
    add_subdirectory(ResourceManager)
endif()

if(PLUGIN_CLOUDSTORE)
    add_subdirectory(CloudStore)
endif()

if(PLUGIN_SHAREDSTORAGE)
    add_subdirectory(SharedStorage)
endif()

if (PLUGIN_ANALYTICS)
    add_subdirectory(Analytics)
endif()

if(PLUGIN_MIGRATIONPREPARER)
	add_subdirectory(MigrationPreparer)
endif()

if(PLUGIN_CONTENTPROTECTION)
    add_subdirectory(ContentProtection)
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
