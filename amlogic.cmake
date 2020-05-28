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


# Realtek specifics
message("Building for amlogic...")

add_definitions (-DPLATFORM_AMLOGIC)
add_definitions (-DUSE_SOUND_PLAYER)


#DeadCodeStrip(DCS)...forRNG150sizereduction
add_definitions (-DENABLE_DCS_BUILD_FLAGS)

add_definitions (-DUSE_IARM)
option(USE_IARM "USE_IARM" ON)
add_definitions (-DUSE_DEVICE_SETTINGS_SERVICE)
add_definitions (-DHAS_API_SYSTEM)
add_definitions (-DCROSS_COMPILED_FOR_DEVICE)
add_definitions (-DHAS_API_HOME_NETWORKING)
#add_definitions (-DHAS_API_QAM_METADATA)
add_definitions (-DHAS_API_POWERSTATE)
add_definitions (-DDISABLE_PRE_RES_CHANGE_EVENTS)
add_definitions (-DHAS_API_APPLICATION)
add_definitions (-DHAS_API_PING)
add_definitions (-DHAS_API_HDCP_COMPLIANCE)
add_definitions (-DHAS_API_DOWNLOADS)

# Enabling image caching for all realtek devices
add_definitions (-DHAS_API_DEVICEPROVISIONING)

if (BUILD_DONT_SET_POWER_BRIGHTNESS)
    message("Building with dont set power brightness")
    add_definitions (-DBUILD_DONT_SET_POWER_BRIGHTNESS)
    add_definitions (-DDONT_SET_POWER_BRIGHTNESS)
endif()

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
#    LIBS += -ldbus-1
endif()

if (BUILD_ENABLE_THERMAL_PROTECTION)
add_definitions (-DBUILD_ENABLE_THERMAL_PROTECTION)
   add_definitions (-DENABLE_THERMAL_PROTECTION)
endif()


if (BUILD_LLAMA)
    message("Building for LLAMA")
    add_definitions (-DBUILD_LLAMA)

    add_definitions (-DENABLE_DEEP_SLEEP)
    add_definitions (-DUSE_LLAMA_MORE_DEFINITIONS)
    add_definitions (-DUSE_UIMAF)

    if (USE_IARM)
        add_definitions (-DUSE_IARM_BUS)
        option(USE_IARM_BUS "USE_IARM_BUS" ON)
        add_definitions (-DUSE_DISPLAY_SETTINGS)
        add_definitions (-DUSE_TSB_SETTINGS)
    endif()

    add_definitions (-DMSO_PAIRING)
    add_definitions (-DENABLE_VREX_SERVICE)
    #add_definitions (-DENABLE_UPDATE_MANAGER_SERVICE)
    add_definitions (-DENABLE_REMOTE_ACTION_MAPPING_SERVICE)
    add_definitions (-DENABLE_CONTROL_SERVICE)
    add_definitions (-DMSO_PAIRING_ENABLE_LOS)
	add_definitions (-DENABLE_MSOPAIRING_4)
    add_definitions (-DENABLE_MSOPAIRING_5)
    add_definitions (-DENABLE_MSOPAIRING_6)
    add_definitions (-DHAS_FRONT_PANEL)
    option(HAS_FRONT_PANEL "HAS_FRONT_PANEL" ON)
    add_definitions (-DENABLE_FRONT_PANEL_API_V4)
    #add_definitions (-DHAS_STATE_OBSERVER)
    add_definitions (-DUSE_DS)
    add_definitions (-DENABLE_GET_SET_APIVERSION)
    add_definitions (-DUSE_UPNP_DISCOVERY)
    add_definitions (-DUSE_TR_69)
    add_definitions (-DENABLE_VREX_SERVICE)
    add_definitions (-DRF4CE_API)
    add_definitions (-DHAS_STATE_OBSERVER)
endif()

if(SCREEN_CAPTURE)
    if(NOT DISABLE_SCREEN_CAPTURE)
	    message("Yocto AMLOGIC build: Building Amlogic specific screen capturing")
        add_definitions (-DPLATFORM_SCREEN_CAPTURE)
    endif()
endif()

if(DEFINES,USE_LLAMA_MORE_DEFINITIONS)
    add_definitions (-DHAS_API_AVINPUT)
    add_definitions (-DUSE_AVINPUT)
    add_definitions (-DQT_GUI_LIB)
    add_definitions (-DQT_NO_QWS_CURSOR)
    add_definitions (-DQT_NETWORK_LIB)
    add_definitions (-DQT_SHARED)
    add_definitions (-DQT_CORE_LIB)
    add_definitions (-DQT_NO_DEBUG)
    add_definitions (-DMSO_PAIRING_ENABLE_LOS)
    add_definitions (-DDISABLE_API_HOME_NETWORKING_2)
    add_definitions (-DDISABLE_WEBKIT_WIDGETS)
    add_definitions (-DENABLE_GET_SET_APIVERSION)   
endif()


