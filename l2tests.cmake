message("Building for L2 tests...")

message("Generating empty headers to suppress compiler errors")

file(GLOB BASEDIR Tests)
set(BASEDIR ${BASEDIR}/headers)
set(EMPTY_HEADERS_DIRS
        ${BASEDIR}
        ${BASEDIR}/rdk/ds
        ${BASEDIR}/rdk/iarmbus
        ${BASEDIR}/rdk/iarmmgrs-hal
        )

set(EMPTY_HEADERS
         ${BASEDIR}/rdk/ds/host.hpp
         ${BASEDIR}/rdk/ds/videoOutputPort.hpp
         ${BASEDIR}/rdk/ds/audioOutputPort.hpp
         ${BASEDIR}/rdk/iarmbus/libIARM.h
         ${BASEDIR}/rdk/iarmbus/libIBus.h
         ${BASEDIR}/rdk/iarmbus/libIBusDaemon.h
         ${BASEDIR}/rdk/iarmmgrs-hal/pwrMgr.h
         ${BASEDIR}/rdk/iarmmgrs-hal/mfrMgr.h
         ${BASEDIR}/rdk/iarmmgrs-hal/sysMgr.h
         ${BASEDIR}/rdk/ds/sleepMode.hpp
        ${BASEDIR}/rfcapi.h
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

file(GLOB BASEDIR Tests/L2_tests/mocks)
set(FAKE_HEADERS
        ${BASEDIR}/devicesettings.h
        ${BASEDIR}/Iarm.h
        ${BASEDIR}/Rfc.h
        )

foreach (file ${FAKE_HEADERS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${file}")
endforeach ()

add_compile_options(-Wall -Werror)


add_definitions(
         -DUSE_IARMBUS
         -DRFC_ENABLED
         -DUSE_DS
         -DENABLE_THERMAL_PROTECTION
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
set(DS_FOUND ON)
