##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2016 RDK Management
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
##########################################################################


# Broadcom specifics
message("Building for raspberrypi...")

add_definitions (-DUSE_SOUND_PLAYER)
add_definitions (-DPLATFORM_BROADCOM_REF)


#DeadCodeStrip(DCS)...forRNG150sizereduction
add_definitions (-DENABLE_DCS_BUILD_FLAGS)

add_definitions (-DUSE_IARM)
add_definitions (-DUSE_IARMBUS)
option(USE_IARM "USE_IARM" ON)
add_definitions (-DUSE_DEVICE_SETTINGS_SERVICE)
add_definitions (-DHAS_API_SYSTEM)
add_definitions (-DCROSS_COMPILED_FOR_DEVICE)
add_definitions (-DHAS_API_HOME_NETWORKING)
#add_definitions (-DHAS_API_QAM_METADATA)
add_definitions (-DHAS_API_POWERSTATE)
add_definitions (-DHAS_API_APPLICATION)
add_definitions (-DHAS_API_PING)
add_definitions (-DHAS_API_HDCP_COMPLIANCE)
add_definitions (-DHAS_API_DOWNLOADS)
    add_definitions (-DUSE_DS)
    add_definitions (-DENABLE_GET_SET_APIVERSION)
    add_definitions (-DUSE_UPNP_DISCOVERY)
    add_definitions (-DUSE_TR_69)
    add_definitions (-DDPLUGIN_STATEOBSERVER)

# Enabling image caching for all broadcom devices
add_definitions (-DHAS_API_DEVICEPROVISIONING)

if (BUILD_ENABLE_HDCP)
    message("Building with hdcp profile")
    add_definitions (-DBUILD_ENABLE_HDCP)
    add_definitions (-DENABLE_HDCP_PROFILE)
endif()

if (BUILD_ENABLE_LEGACY_HDCP_PROFILE)
    message("Building with hdcp profile 0 and 1")
    add_definitions (-DBUILD_ENABLE_LEGACY_HDCP_PROFILE)
    add_definitions (-DENABLE_HDCP_PROFILE_0_1)
endif()

if (BUILD_DISABLE_HOME_NETWORKING_V2)
    message("Building with hn2 disabled")
    add_definitions (-DBUILD_DISABLE_HOME_NETWORKING_V2)
    add_definitions (-DDISABLE_API_HOME_NETWORKING_2)
endif()

if (BUILD_DBUS)
    message("Building for DBUS")

    add_definitions (-DBUILD_DBUS)
    option(BUILD_DBUS "BUILD_DBUS" ON)
    add_definitions (-DIARM_USE_DBUS)
    option(IARM_USE_DBUS "IARM_USE_DBUS" ON)
endif()

if (BUILD_FOR_COMMUNITY)
    message("Building for COMMUNITY")
    add_definitions (-DBUILD_FOR_COMMUNITY)
endif()




