#
# ============================================================================
# RDK MANAGEMENT, LLC CONFIDENTIAL AND PROPRIETARY
# ============================================================================
# This file (and its contents) are the intellectual property of RDK Management, LLC.
# It may not be used, copied, distributed or otherwise  disclosed in whole or in
# part without the express written permission of RDK Management, LLC.
# ============================================================================
# Copyright (c) 2016 RDK Management, LLC. All rights reserved.
# ============================================================================
#


# Broadcom specifics
message("Building for raspberrypi...")

add_definitions (-DUSE_SOUND_PLAYER)


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

if (BUILD_ENABLE_THERMAL_PROTECTION)
   add_definitions (-DENABLE_THERMAL_PROTECTION)
endif()



