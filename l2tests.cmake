# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
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


message("Building for L2 tests...")

message("Generating empty headers to suppress compiler errors")

file(GLOB BASEDIR Tests)
set(BASEDIR ${BASEDIR}/headers)
set(EMPTY_HEADERS_DIRS
        ${BASEDIR}
        ${BASEDIR}/rdk/ds
        ${BASEDIR}/rdk/iarmbus
        ${BASEDIR}/rdk/iarmmgrs-hal
        ${BASEDIR}/systemservices
        ${BASEDIR}/systemservices/proc
        )

set(EMPTY_HEADERS
         ${BASEDIR}/rdk/ds/host.hpp
         ${BASEDIR}/rdk/ds/videoOutputPort.hpp
         ${BASEDIR}/rdk/ds/audioOutputPort.hpp
         ${BASEDIR}/rdk/ds/dsMgr.h
         ${BASEDIR}/rdk/ds/manager.hpp
         ${BASEDIR}/rdk/iarmbus/libIARM.h
         ${BASEDIR}/rdk/iarmbus/libIBus.h
         ${BASEDIR}/rdk/iarmbus/libIBusDaemon.h
         ${BASEDIR}/rdk/iarmmgrs-hal/pwrMgr.h
         ${BASEDIR}/rdk/iarmmgrs-hal/mfrMgr.h
         ${BASEDIR}/rdk/iarmmgrs-hal/sysMgr.h
         ${BASEDIR}/rdk/ds/videoOutputPortConfig.hpp
         ${BASEDIR}/rdk/ds/sleepMode.hpp
         ${BASEDIR}/rdk/ds/frontPanelConfig.hpp
         ${BASEDIR}/rdk/ds/frontPanelTextDisplay.hpp
         ${BASEDIR}/rfcapi.h
         ${BASEDIR}/rbus.h
         ${BASEDIR}/systemservices/proc/readproc.h
         ${BASEDIR}/systemservices/secure_wrapper.h
         ${BASEDIR}/libudev.h
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
        ${BASEDIR}/Iarm.h
        ${BASEDIR}/Rfc.h
        ${BASEDIR}/RBus.h
        ${BASEDIR}/Udev.h
        ${BASEDIR}/secure_wrappermock.h
        )

foreach (file ${FAKE_HEADERS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${file}")
endforeach ()

add_compile_options(-Wall -Werror)

add_link_options(-Wl,-wrap,system -Wl,-wrap,setmntent -Wl,-wrap,getmntent -Wl,-wrap,v_secure_popen -Wl,-wrap,v_secure_pclose -Wl,-wrap,v_secure_system -Wl,-wrap,readlink)

add_definitions(
         -DUSE_IARMBUS
         -DRFC_ENABLED
         -DUSE_DS
         -DENABLE_THERMAL_PROTECTION
         -DHAS_API_SYSTEM
         -DHAS_API_POWERSTATE
         -DHAS_RBUS
         -DUNIT_TESTING
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
set(PLUGIN_SYSTEMSERVICES ON)
set(PLUGIN_HDCPPROFILE ON)
set(PLUGIN_NETWORK ON)
set(PLUGIN_TEXTTOSPEECH ON)
set(PLUGIN_TELEMETRY ON)
set(PLUGIN_WAREHOUSE ON)
set(PLUGIN_USBACCESS ON)
if (USE_THUNDER_R4)
   set(PLUGIN_PERSISTENTSTORE ON)
   set(PLUGIN_USERSETTINGS ON)
   set(PLUGIN_SYSTEMMODE ON)
endif (USE_THUNDER_R4)
set(PLUGIN_L2Tests ON)
set(BUILD_SHARED_LIBS ON)
set(DS_FOUND ON)
