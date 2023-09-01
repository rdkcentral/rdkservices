message("Building for unit tests...")

message("Generating empty headers to suppress compiler errors")

file(GLOB BASEDIR Tests)
set(BASEDIR ${BASEDIR}/headers)
set(EMPTY_HEADERS_DIRS
        ${BASEDIR}
        ${BASEDIR}/audiocapturemgr
        ${BASEDIR}/rdk/ds
        ${BASEDIR}/rdk/iarmbus
        ${BASEDIR}/rdk/iarmmgrs-hal
        ${BASEDIR}/ccec/drivers
        ${BASEDIR}/ccec/drivers/iarmbus
        ${BASEDIR}/ccec/host
        ${BASEDIR}/network
        ${BASEDIR}/Dobby
        ${BASEDIR}/Dobby/Public/Dobby
        ${BASEDIR}/Dobby/IpcService
        ${BASEDIR}/websocket
        ${BASEDIR}/rdk/iarmmgrs
	${BASEDIR}/rdkshell
        ${BASEDIR}/opkg
        )

set(EMPTY_HEADERS
        ${BASEDIR}/audiocapturemgr/audiocapturemgr_iarm.h
        ${BASEDIR}/ccec/drivers/CecIARMBusMgr.h
        ${BASEDIR}/ccec/drivers/iarmbus/CecIARMBusMgr.h
        ${BASEDIR}/ccec/FrameListener.hpp
        ${BASEDIR}/ccec/Connection.hpp
        ${BASEDIR}/ccec/Assert.hpp
        ${BASEDIR}/ccec/Messages.hpp
        ${BASEDIR}/ccec/MessageDecoder.hpp
        ${BASEDIR}/ccec/MessageProcessor.hpp
        ${BASEDIR}/ccec/CECFrame.hpp
        ${BASEDIR}/ccec/host/RDK.hpp
        ${BASEDIR}/ccec/MessageEncoder.hpp
        ${BASEDIR}/rdk/ds/audioOutputPort.hpp
        ${BASEDIR}/rdk/ds/compositeIn.hpp
        ${BASEDIR}/rdk/ds/dsDisplay.h
        ${BASEDIR}/rdk/ds/dsError.h
        ${BASEDIR}/rdk/ds/dsMgr.h
        ${BASEDIR}/rdk/ds/dsTypes.h
        ${BASEDIR}/rdk/ds/dsUtl.h
        ${BASEDIR}/rdk/ds/exception.hpp
        ${BASEDIR}/rdk/ds/hdmiIn.hpp
        ${BASEDIR}/rdk/ds/host.hpp
        ${BASEDIR}/rdk/ds/list.hpp
        ${BASEDIR}/rdk/ds/manager.hpp
        ${BASEDIR}/rdk/ds/sleepMode.hpp
        ${BASEDIR}/rdk/ds/videoDevice.hpp
        ${BASEDIR}/rdk/ds/videoOutputPort.hpp
        ${BASEDIR}/rdk/ds/videoOutputPortConfig.hpp
        ${BASEDIR}/rdk/ds/videoOutputPortType.hpp
        ${BASEDIR}/rdk/ds/videoResolution.hpp
        ${BASEDIR}/rdk/ds/frontPanelIndicator.hpp
        ${BASEDIR}/rdk/ds/frontPanelConfig.hpp
        ${BASEDIR}/rdk/ds/frontPanelTextDisplay.hpp
        ${BASEDIR}/rdk/iarmbus/libIARM.h
        ${BASEDIR}/rdk/iarmbus/libIBus.h
        ${BASEDIR}/rdk/iarmbus/libIBusDaemon.h
        ${BASEDIR}/rdk/iarmmgrs-hal/deepSleepMgr.h
        ${BASEDIR}/rdk/iarmmgrs-hal/mfrMgr.h
        ${BASEDIR}/rdk/iarmmgrs-hal/pwrMgr.h
        ${BASEDIR}/rdk/iarmmgrs-hal/sysMgr.h
        ${BASEDIR}/network/wifiSrvMgrIarmIf.h
        ${BASEDIR}/network/netsrvmgrIarm.h
	      ${BASEDIR}/rdkshell/rdkshellevents.h
        ${BASEDIR}/rdkshell/rdkshell.h
        ${BASEDIR}/rdkshell/compositorcontroller.h
        ${BASEDIR}/rdkshell/logger.h
        ${BASEDIR}/rdkshell/eastereggs.h
        ${BASEDIR}/rdkshell/application.h
        ${BASEDIR}/rdkshell/linuxkeys.h
        ${BASEDIR}/framebuffer-api.h
        ${BASEDIR}/libudev.h
        ${BASEDIR}/rfcapi.h
        ${BASEDIR}/rbus.h
        ${BASEDIR}/telemetry_busmessage_sender.h
        ${BASEDIR}/motionDetector.h
        ${BASEDIR}/Dobby/DobbyProtocol.h
        ${BASEDIR}/Dobby/DobbyProxy.h
        ${BASEDIR}/Dobby/Public/Dobby/IDobbyProxy.h
        ${BASEDIR}/Dobby/IpcService/IpcFactory.h
        ${BASEDIR}/ccec/FrameListener.hpp
      	${BASEDIR}/ccec/Connection.hpp
	      ${BASEDIR}/ccec/Assert.hpp
	      ${BASEDIR}/ccec/Messages.hpp
	      ${BASEDIR}/ccec/MessageDecoder.hpp
	      ${BASEDIR}/ccec/MessageProcessor.hpp
	      ${BASEDIR}/ccec/CECFrame.hpp
	      ${BASEDIR}/ccec/MessageEncoder.hpp
	      ${BASEDIR}/ccec/host/RDK.hpp
	      ${BASEDIR}/ccec/drivers/iarmbus/CecIARMBusMgr.h
        ${BASEDIR}/rtRemote.h
        ${BASEDIR}/rtObject.h
        ${BASEDIR}/rtError.h
        ${BASEDIR}/rtNotifier.h
	${BASEDIR}/dsRpc.h
	${BASEDIR}/websocket/URL.h
        ${BASEDIR}/rdk/iarmmgrs/irMgr.h
        ${BASEDIR}/rdk/iarmmgrs/comcastIrKeyCodes.h
	    ${BASEDIR}/rdk_logger_milestone.h
	    ${BASEDIR}/opkg/opkg.h
	    ${BASEDIR}/opkg/opkg_message.h
	    ${BASEDIR}/opkg/opkg_cmd.h
	    ${BASEDIR}/opkg/opkg_download.h
        )

file(MAKE_DIRECTORY ${EMPTY_HEADERS_DIRS})

file(GLOB_RECURSE EMPTY_HEADERS_AVAILABLE "${BASEDIR}/*")
if (EMPTY_HEADERS_AVAILABLE)
    message("Skip already generated headers to avoid rebuild")
    list(REMOVE_ITEM EMPTY_HEADERS ${EMPTY_HEADERS_AVAILABLE})
endif ()
if (EMPTY_HEADERS)
    file(TOUCH ${EMPTY_HEADERS})
endif ()

include_directories(${EMPTY_HEADERS_DIRS})

message("Adding compiler and linker options for all targets")

file(GLOB BASEDIR Tests/mocks)
set(FAKE_HEADERS
        ${BASEDIR}/devicesettings.h
        ${BASEDIR}/FrameBuffer.h
        ${BASEDIR}/Iarm.h
        ${BASEDIR}/Rfc.h
        ${BASEDIR}/RBus.h
        ${BASEDIR}/Telemetry.h
        ${BASEDIR}/Udev.h
        ${BASEDIR}/MotionDetection.h
        ${BASEDIR}/Dobby.h
        ${BASEDIR}/HdmiCec.h
        ${BASEDIR}/rdkshell.h
	      ${BASEDIR}/RdkLoggerMilestone.h
        )

foreach (file ${FAKE_HEADERS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${file}")
endforeach ()

add_compile_options(-Wall -Werror)


add_link_options(-Wl,-wrap,system -Wl,-wrap,popen -Wl,-wrap,syslog -Wl,-wrap,pclose -Wl,-wrap,getmntent -Wl,-wrap,setmntent)

add_definitions(
        -DENABLE_TELEMETRY_LOGGING
        -DUSE_IARMBUS
        -DENABLE_SYSTEM_GET_STORE_DEMO_LINK
        -DENABLE_DEEP_SLEEP
        -DENABLE_SET_WAKEUP_SRC_CONFIG
        -DENABLE_THERMAL_PROTECTION
        -DUSE_FRAMEBUFFER_SCREENCAPTURE
        -DHAS_API_SYSTEM
        -DHAS_API_POWERSTATE
        -DHAS_RBUS
        -DENABLE_DEVICE_MANUFACTURER_INFO
        -DRFC_ENABLED
        -DXCAST_ENABLED_BY_DEFAULT
        -DCLOCK_BRIGHTNESS_ENABLED
        -DUSE_DS
)

message("Setting build options")

set(CMAKE_DISABLE_FIND_PACKAGE_DS ON)
set(CMAKE_DISABLE_FIND_PACKAGE_IARMBus ON)
set(CMAKE_DISABLE_FIND_PACKAGE_Udev ON)
set(CMAKE_DISABLE_FIND_PACKAGE_RFC ON)
set(CMAKE_DISABLE_FIND_PACKAGE_RBus ON)
set(CMAKE_DISABLE_FIND_PACKAGE_CEC ON)
set(CMAKE_DISABLE_FIND_PACKAGE_Dobby ON)
set(CMAKE_DISABLE_FIND_PACKAGE_CEC ON)
set(LIBOPKG_LIBRARIES ${LIBOPKG_LIBRARIES} CACHE PATH "Path to LIBOPKG library")
#set(BTMGR_INCLUDE_DIRS ${BTMGR_INCLUDE_DIRS}
set(LIBOPKG_INCLUDE_DIRS ${LIBOPKG_INCLUDE_DIRS} CACHE PATH "Path to LIBOPKG include")
set(PLUGIN_DATACAPTURE ON)
set(PLUGIN_DEVICEDIAGNOSTICS ON)
set(PLUGIN_LOCATIONSYNC ON)
set(PLUGIN_PERSISTENTSTORE ON)
set(PLUGIN_TIMER ON)
set(PLUGIN_SECURITYAGENT ON)
set(PLUGIN_DEVICEIDENTIFICATION ON)
set(PLUGIN_FRAMERATE ON)
set(PLUGIN_AVINPUT ON)
set(PLUGIN_TELEMETRY ON)
set(PLUGIN_SCREENCAPTURE ON)
set(PLUGIN_USBACCESS ON)
set(PLUGIN_LOGGINGPREFERENCES ON)
set(PLUGIN_USERPREFERENCES ON)
set(PLUGIN_MESSENGER ON)
set(PLUGIN_DEVICEINFO ON)
set(PLUGIN_SYSTEMSERVICES ON)
set(PLUGIN_HDMIINPUT ON)
set(PLUGIN_HDCPPROFILE ON)
set(PLUGIN_NETWORK ON)
set(PLUGIN_WIFIMANAGER ON)
set(PLUGIN_TRACECONTROL ON)
set(PLUGIN_WAREHOUSE ON)
set(PLUGIN_ACTIVITYMONITOR ON)
set(PLUGIN_MOTION_DETECTION ON)
set(PLUGIN_COMPOSITEINPUT ON)
set(PLUGIN_OCICONTAINER ON)
set(HAS_FRONT_PANEL ON)
set(PLUGIN_XCAST ON)
set(PLUGIN_HDMICEC ON)
set(PLUGIN_HDMICEC2 ON)
set(PLUGIN_HDMICECSOURCE ON)
set(PLUGIN_HDMICECSINK ON)
set(PLUGIN_RDKSHELL ON)
set(PLUGIN_MAINTENANCEMANAGER ON)
set(PLUGIN_PACKAGER ON)

set(DS_FOUND ON)
