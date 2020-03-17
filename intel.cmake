# Canmore specifics
add_definitions (-DPLATFORM_CANMORE)
#Uncomment this for IP Only

add_definitions (-DUSE_DS)
option(USE_DS "USE_DS" ON)

add_definitions (-DUSE_IARM_BUS)
option(USE_IARM_BUS "USE_IARM_BUS" ON)

add_definitions (-DHAS_FRONT_PANEL)
option(HAS_FRONT_PANEL "HAS_FRONT_PANEL" ON)

add_definitions (-DCLOCK_BRIGHTNESS_ENABLED)
option(CLOCK_BRIGHTNESS_ENABLED "CLOCK_BRIGHTNESS_ENABLED" ON)

add_definitions (-DUSE_UPNP_DISCOVERY)
add_definitions (-DUSE_LSB)


if (BUILD_ENABLE_RFREMOTE_API)
    message("Building with rf remote API")
    add_definitions (-DBUILD_ENABLE_RFREMOTE_API)
    add_definitions (-DHAS_API_RFREMOTE)
endif()

if (BUILD_ENABLE_HDCP)
    message("Building xre with hdcp profile")
    add_definitions (-DBUILD_ENABLE_HDCP)
    option(BUILD_ENABLE_HDCP "BUILD_ENABLE_HDCP" ON)
    add_definitions (-DENABLE_HDCP_PROFILE)
endif()

if (BUILD_X1)
    message("Building for X1")
    add_definitions (-DBUILD_X1)
    add_definitions (-DSUPPORT_HOME_NETWORKING_V3)

    if (IP_VIDEO_ONLY)
        message("Using IP video only")
#                       DEFINES -= PLATFORM_CANMORE
        add_definitions (-DUSE_IP_VIDEO_ONLY)
        add_definitions (-DUSE_TSB_SETTINGS)
        add_definitions (-DENABLE_TSB_SETTING)
    endif()

    if (BUILD_DELIA)
        message("Building for Delia")
        add_definitions (-DUSE_DS)
        add_definitions (-DUSE_IARMBUS)
        add_definitions (-DPLATFORM_INTEL_IP_ONLY)
        add_definitions (-DDELIA_MODE)
        add_definitions (-DUSE_DISPLAY_SETTINGS)
        add_definitions (-DENABLE_EXIT_D_D_2_WEB_DIAGNOSTICS)

        if (BUILD_ENABLE_HDCP)
            message("Building x1 xre with hdcp profile")
            add_definitions (-DENABLE_HDCP_PROFILE)
        endif()
        if (BUILD_DBUS)
             message("Building for DBUS")
             add_definitions (-DIARM_USE_DBUS)
        endif()
    endif()

    add_definitions (-DMSO_PAIRING)
    add_definitions (-DMSO_PAIRING_ENABLE_LOS)
    add_definitions (-DRF4CE_API)
    add_definitions (-DUSE_IARM)
    add_definitions (-DHAS_SNAG_AUDIO)
    add_definitions (-DHAS_STATE_OBSERVER)
    add_definitions (-DENABLE_SERVICEPROXY_SYSTEM3_SYSTEM4)
    add_definitions (-DENABLE_MSOPAIRING_4)
    add_definitions (-DENABLE_MSOPAIRING_5)
    add_definitions (-DENABLE_MSOPAIRING_6)
    add_definitions (-DUSE_DISPLAY_SETTINGS)
    add_definitions (-DUSE_THUNDER_DISPLAY_SETTINGS)
    add_definitions (-DUSE_DEVICE_SETTINGS_SERVICE)
    add_definitions (-DUSE_RFBYPASS_LED_KEYPRESS)
    add_definitions (-DENABLE_VREX_SERVICE)
    add_definitions (-DENABLE_REMOTE_ACTION_MAPPING_SERVICE)
    add_definitions (-DENABLE_CONTROL_SERVICE)
    add_definitions (-DUSE_UNIFIED_RF4CE_MGR_API_4)
    add_definitions (-DENABLE_FRONT_PANEL_API_V4)
endif()

if(PLATFORM_INTEL_IP_ONLY)
    message("Building for IP Only...")
    add_definitions (-DPLATFORM_INTEL_IP_ONLY)
else()
    message("Building for Canmore...")
    add_definitions (-DHAS_GMA)
    add_definitions (-DHAS_QAM)
endif()

#add_definitions (-DUSE_SOUND_PLAYER)

#add_definitions (-DUSE_AVINPUT)
add_definitions (-DCROSS_COMPILED_FOR_DEVICE)
add_definitions (-DENABLE_GET_SET_APIVERSION)

#!contains(DEFINES, USE_DS) {
#    HEADERS += ../../platform/broadcom/include/rdk_fpd.h
#}

add_definitions (-DHAS_API_RFREMOTE)
add_definitions (-DHAS_API_SYSTEM)
add_definitions (-DHAS_API_POWERSTATE)
add_definitions (-DHAS_API_AVINPUT)
add_definitions (-DHAS_API_HOME_NETWORKING)
add_definitions (-DHAS_API_APPLICATION)
add_definitions (-DHAS_API_PING)
add_definitions (-DHAS_API_HDCP_COMPLIANCE)

add_definitions (-DHAS_API_DEVICEPROVISIONING)
#add_definitions (-DHAS_API_QAM_METADATA)
add_definitions (-DHAS_API_DOWNLOADS)

