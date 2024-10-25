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

cmake -G Ninja -S ThunderTools -B build/ThunderTools -DCMAKE_INSTALL_PREFIX="install"

cmake --build build/ThunderTools --target install

############################
# 3. Build Thunder

git clone https://github.com/rdkcentral/Thunder.git

cmake -G Ninja -S Thunder -B build/Thunder \
  -DBINDING="127.0.0.1" \
  -DCMAKE_BUILD_TYPE="Debug" \
  -DCMAKE_INSTALL_PREFIX="install" \
  -DPORT="55555" \
  -DTOOLS_SYSROOT="${PWD}" \
  -DINITV_SCRIPT=OFF

cmake --build build/Thunder --target install

############################
# 4. Build ThunderInterfaces

git clone -b RDK-52316 https://github.com/npoltorapavlo/ThunderInterfaces.git

cmake -G Ninja -S ThunderInterfaces -B build/ThunderInterfaces \
  -DCMAKE_INSTALL_PREFIX="install"

cmake --build build/ThunderInterfaces --target install
