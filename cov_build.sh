# # #!/bin/bash
set -e

# # ############################
# # # EXPERIMENTAL
# # # DO NOT MODIFY

# # ############################
# # # # 1. Install Dependencies

ROOT_PATH=$(pwd)

apt update
apt install -y build-essential cmake ninja-build meson libusb-1.0-0-dev zlib1g-dev libssl-dev tree libunwind-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-good1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-pulseaudio libcunit1-dev libcurl4-openssl-dev
pip install jsonref

# # ############################
# 2. Build Thunder Tools

git clone --branch R4_4 https://github.com/rdkcentral/ThunderTools.git

cmake -G Ninja -S ThunderTools -B build/ThunderTools -DCMAKE_INSTALL_PREFIX="install"

cmake --build build/ThunderTools --target install

############################
# # 3. Build Thunder (WPEFramework)

git clone --branch R4_4 https://github.com/rdkcentral/Thunder.git

cmake -G Ninja -S Thunder -B build/Thunder \
  -DBINDING="127.0.0.1" \
  -DCMAKE_BUILD_TYPE="Debug" \
  -DCMAKE_INSTALL_PREFIX="install" \
  -DPORT="55555" \
  -DTOOLS_SYSROOT="${PWD}" \
  -DINITV_SCRIPT=OFF

cmake --build build/Thunder --target install

# ############################
# 4. Build ThunderInterfaces

git clone --branch R4_4 https://github.com/agampa263/ThunderInterfaces.git

cmake -G Ninja -S ThunderInterfaces -B build/ThunderInterfaces \
  -DCMAKE_INSTALL_PREFIX="install"

cmake --build build/ThunderInterfaces --target install

############################
# Build trevor-base64
if [ ! -d "trower-base64" ]; then
git clone https://github.com/xmidt-org/trower-base64.git
fi
cd trower-base64
meson setup --warnlevel 3 --werror build
ninja -C build
ninja -C build install
cd ..

############################
# 5. Build rdkservices
git clone --branch sprint/25Q1 https://github.com/agampa263/rdkservices.git

echo "======================================================================================"
echo "before creating dir and files"
echo "======================================================================================"

cd rdkservices/Tests
echo "current working dir: "$(pwd)
echo " Empty mocks creation to avoid compilation errors"
echo "======================================================================================"
mkdir -p headers
mkdir -p headers/audiocapturemgr
mkdir -p headers/rdk/ds
mkdir -p headers/rdk/iarmbus
mkdir -p headers/rdk/iarmmgrs-hal
mkdir -p headers/rdk/halif/
mkdir -p headers/rdk/halif/deepsleep-manager
mkdir -p headers/ccec/drivers
mkdir -p headers/network
mkdir -p headers/proc
echo "dir created successfully"

echo "======================================================================================"
echo "after dir creation"
cd ..
cd Tests
echo "======================================================================================"



echo "empty files creation to avoid compilation errors"
echo "======================================================================================"
cd headers
echo "current working dir: "$(pwd)
touch audiocapturemgr/audiocapturemgr_iarm.h
touch ccec/drivers/CecIARMBusMgr.h
touch rdk/ds/audioOutputPort.hpp
touch rdk/ds/compositeIn.hpp
touch rdk/ds/dsDisplay.h
touch rdk/ds/dsError.h
touch rdk/ds/dsMgr.h
touch rdk/ds/dsTypes.h
touch rdk/ds/dsUtl.h
touch rdk/ds/exception.hpp
touch rdk/ds/hdmiIn.hpp
touch rdk/ds/host.hpp
touch rdk/ds/list.hpp
touch rdk/ds/manager.hpp
touch rdk/ds/sleepMode.hpp
touch rdk/ds/videoDevice.hpp
touch rdk/ds/videoOutputPort.hpp
touch rdk/ds/videoOutputPortConfig.hpp
touch rdk/ds/videoOutputPortType.hpp
touch rdk/ds/videoResolution.hpp
touch rdk/iarmbus/libIARM.h
touch rdk/iarmbus/libIBus.h
touch rdk/iarmbus/libIBusDaemon.h
touch rdk/iarmmgrs-hal/deepSleepMgr.h
touch rdk/iarmmgrs-hal/mfrMgr.h
touch rdk/iarmmgrs-hal/pwrMgr.h
touch rdk/iarmmgrs-hal/sysMgr.h
touch network/wifiSrvMgrIarmIf.h
touch network/netsrvmgrIarm.h
touch libudev.h
touch rfcapi.h
touch rbus.h
touch telemetry_busmessage_sender.h
touch maintenanceMGR.h
touch pkg.h
touch secure_wrapper.h
touch wpa_ctrl.h
touch btmgr.h
echo "files created successfully"
echo "======================================================================================"
echo "current working dir: "$(pwd)
echo "======================================================================================"
cd ../../../
echo "current working dir after cd : "$(pwd)
echo "======================================================================================"
echo "after creating dir and files"
echo "======================================================================================"
cp -r /usr/include/gstreamer-1.0/gst /usr/include/glib-2.0/* /usr/lib/x86_64-linux-gnu/glib-2.0/include/* /usr/local/include/trower-base64/base64.h .

echo "root path -> ${ROOT_PATH}"
cd ${ROOT_PATH}

echo "starting builing rdkservices"
cmake -G Ninja -S rdkservices -B build/rdkservices \
  -DCMAKE_INSTALL_PREFIX="install" \
  -DCMAKE_VERBOSE_MAKEFILE=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_IARMBus=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_RFC=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_DS=ON \
  -DCMAKE_CXX_FLAGS="-DEXCEPTIONS_ENABLE=ON \
                      -I ${PWD}/rdkservices/Tests/headers \
                      -I ${PWD}/rdkservices/Tests/headers/audiocapturemgr \
                      -I ${PWD}/rdkservices/Tests/headers/rdk/ds \
                      -I ${PWD}/rdkservices/Tests/headers/rdk/iarmbus \
                      -I ${PWD}/rdkservices/Tests/headers/rdk/iarmmgrs-hal \
                      -I ${PWD}/rdkservices/Tests/headers/ccec/drivers \
                      -I ${PWD}/rdkservices/Tests/headers/network \
                      -include ${PWD}/rdkservices/Tests/mocks/devicesettings.h \
                      -include ${PWD}/rdkservices/Tests/mocks/Iarm.h \
                      -include ${PWD}/rdkservices/Tests/mocks/Rfc.h \
                      -include ${PWD}/rdkservices/Tests/mocks/RBus.h \
                      -include ${PWD}/rdkservices/Tests/mocks/Telemetry.h \
                      -include ${PWD}/rdkservices/Tests/mocks/Udev.h \
                      -include ${PWD}/rdkservices/Tests/mocks/pkg.h \
                      -include ${PWD}/rdkservices/Tests/mocks/maintenanceMGR.h \
                      -include ${PWD}/rdkservices/Tests/mocks/secure_wrappermock.h \
                      -include ${PWD}/rdkservices/Tests/mocks/WpaCtrl.h \
                      --coverage -Wall -Werror -Wno-error=format \
                      -Wl,-wrap,system -Wl,-wrap,popen -Wl,-wrap,syslog \
                      -DENABLE_TELEMETRY_LOGGING -DUSE_IARMBUS \
                      -DENABLE_SYSTEM_GET_STORE_DEMO_LINK -DENABLE_DEEP_SLEEP \
                      -DENABLE_SET_WAKEUP_SRC_CONFIG -DENABLE_THERMAL_PROTECTION \
                      -DUSE_DRM_SCREENCAPTURE -DHAS_API_SYSTEM -DHAS_API_POWERSTATE \
                      -DHAS_RBUS -DDISABLE_SECURITY_TOKEN -DENABLE_DEVICE_MANUFACTURER_INFO -DUSE_THUNDER_R4" \
  -DCOMCAST_CONFIG=OFF \
  -DDS_FOUND=ON \
  -DPLUGIN_DEVICEIDENTIFICATION=ON \

if [ $? -eq 0 ]; then
    echo "rdkservices source successful"
else
    echo "rdkservices source failed"
fi

cmake --build build/rdkservices --target install
if [ $? -eq 0 ]; then
    echo "rdkservices build successful"
else
    echo "rdkservices build failed"
fi
echo "======================================================================================"
echo "path of so files"
find . -iname "*.so*"
exit 0
