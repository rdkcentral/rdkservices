#!/bin/bash
set -e

############################
# From https://rdkcentral.github.io/Thunder/introduction/build_linux/
# DO NOT MODIFY

############################
# 1. Install Dependencies

sudo apt install -y build-essential cmake ninja-build libusb-1.0-0-dev zlib1g-dev libssl-dev

pip install jsonref

############################
# 2. Build Thunder Tools

git clone https://github.com/rdkcentral/ThunderTools.git

cmake -G Ninja -S ThunderTools -B build/ThunderTools -DCMAKE_INSTALL_PREFIX="install/usr"

cmake --build build/ThunderTools --target install

############################
# 3. Build Thunder

git clone https://github.com/rdkcentral/Thunder.git

cmake -G Ninja -S Thunder -B build/Thunder \
  -DBUILD_SHARED_LIBS=ON \
  -DBINDING="127.0.0.1" \
  -DCMAKE_BUILD_TYPE="Debug" \
  -DCMAKE_INSTALL_PREFIX="install/usr" \
  -DCMAKE_MODULE_PATH="${PWD}/install/usr/include/WPEFramework/Modules" \
  -DDATA_PATH="${PWD}/install/usr/share/WPEFramework" \
  -DPERSISTENT_PATH="${PWD}/install/var/wpeframework" \
  -DPORT="55555" \
  -DPROXYSTUB_PATH="${PWD}/install/usr/lib/wpeframework/proxystubs" \
  -DSYSTEM_PATH="${PWD}/install/usr/lib/wpeframework/plugins" \
  -DVOLATILE_PATH="tmp"

cmake --build build/Thunder --target install

############################
# 4. Build ThunderInterfaces

git clone https://github.com/rdkcentral/ThunderInterfaces.git

cmake -G Ninja -S ThunderInterfaces -B build/ThunderInterfaces \
  -DCMAKE_INSTALL_PREFIX="install/usr" \
  -DCMAKE_MODULE_PATH="${PWD}/install/usr/include/WPEFramework/Modules"

cmake --build build/ThunderInterfaces --target install
