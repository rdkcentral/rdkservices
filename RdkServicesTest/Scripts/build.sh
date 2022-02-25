#!/bin/bash

set -e

THREADS="-j"
if [[ "${1}" == *"-j"* ]]; then
  THREADS="$1"
fi

MODE="Release"
if [ "$1" == "-D" -o "$2" == "-D" ]; then
  MODE="Debug"
fi

ROOT=$(pwd)

THUNDER_ROOT=$(pwd)/thunder
THUNDER_INSTALL_DIR=${THUNDER_ROOT}/install

THUNDER_BINDING=127.0.0.1
THUNDER_PORT=55555

THUNDER_URL=https://github.com/rdkcentral/Thunder
THUNDER_BRANCH=R2
THUNDER_REV=54c2404197f16255cc47543e2d861e2c8137ee51

INTERFACES_URL=https://github.com/rdkcentral/ThunderInterfaces
INTERFACES_BRANCH=R2
INTERFACES_REV=1ed7eee3e833ed2f7d6a39624f0e32d1659d3f03

checkPython() {
  case "$(python --version)" in
  *"$1"*) true ;;
  *) false ;;
  esac
}

checkPip() {
  case "$(pip --version)" in
  *"$1"*) true ;;
  *) false ;;
  esac
}

checkInstalled() {
  pkg-config "$1"
  case "$?" in
  0) true ;;
  *) false ;;
  esac
}

checkWPEFramework() {
  PATH="${THUNDER_INSTALL_DIR}/usr/bin:${PATH}" \
    LD_LIBRARY_PATH="${THUNDER_INSTALL_DIR}/usr/lib:${LD_LIBRARY_PATH}" \
    WPEFramework -c "${THUNDER_INSTALL_DIR}/etc/WPEFramework/config.json" &

  sleep 5

  curl -d '{"jsonrpc":"2.0","id":0,"method":"Controller.1.processinfo"}' "http://${THUNDER_BINDING}:${THUNDER_PORT}/jsonrpc"
  kill -9 "$(pidof WPEFramework)"
}

installJsonref() {
  pip install jsonref
}

checkoutThunder() {
  if [ ! -d "${THUNDER_INSTALL_DIR}" ]; then
    mkdir -p "${THUNDER_INSTALL_DIR}"
    cd "${THUNDER_ROOT}" && git clone "${THUNDER_URL}" || exit 1
  fi

  cd "${THUNDER_ROOT}/Thunder" && git checkout "${THUNDER_BRANCH}" && git checkout "${THUNDER_REV}" || exit 1
}

checkoutThunderInterfaces() {
  if [ ! -d "${THUNDER_ROOT}/ThunderInterfaces" ]; then
    cd "${THUNDER_ROOT}" && git clone "${INTERFACES_URL}" || exit 1
  fi

  cd "${THUNDER_ROOT}/ThunderInterfaces" && git checkout "${INTERFACES_BRANCH}" && git checkout "${INTERFACES_REV}" || exit 1
}

buildAndInstallTools() {
  cd "${THUNDER_ROOT}" || exit 1

  cmake -HThunder/Tools -Bbuild/ThunderTools \
    -DCMAKE_INSTALL_PREFIX="${THUNDER_INSTALL_DIR}/usr" \
    -DCMAKE_MODULE_PATH="${THUNDER_INSTALL_DIR}/tools/cmake" \
    -DGENERIC_CMAKE_MODULE_PATH="${THUNDER_INSTALL_DIR}/tools/cmake"

  make -C build/ThunderTools $THREADS && make -C build/ThunderTools install $THREADS
}

buildAndInstallThunder() {
  cd "${THUNDER_ROOT}" || exit 1

  cmake -HThunder -Bbuild/Thunder \
    -DCMAKE_INSTALL_PREFIX="${THUNDER_INSTALL_DIR}/usr" \
    -DCMAKE_MODULE_PATH="${THUNDER_INSTALL_DIR}/tools/cmake" \
    -DBUILD_TYPE=Debug -DBINDING="${THUNDER_BINDING}" -DPORT="${THUNDER_PORT}"

  make -C build/Thunder $THREADS && make -C build/Thunder install $THREADS
}

buildAndInstallThunderInterfaces() {
  cd "${THUNDER_ROOT}" || exit 1

  cmake -HThunderInterfaces -Bbuild/ThunderInterfaces \
    -DCMAKE_INSTALL_PREFIX="${THUNDER_INSTALL_DIR}/usr" \
    -DCMAKE_MODULE_PATH="${THUNDER_INSTALL_DIR}/tools/cmake"

  make -C build/ThunderInterfaces $THREADS && make -C build/ThunderInterfaces install $THREADS
}

buildAndInstallRdkservices() {
  cd "${THUNDER_ROOT}" || exit 1

  cmake -H../.. -Bbuild/rdkservices \
    -DCMAKE_INSTALL_PREFIX="${THUNDER_INSTALL_DIR}/usr" \
    -DCMAKE_MODULE_PATH="${THUNDER_INSTALL_DIR}/tools/cmake" \
    -DCOMCAST_CONFIG=OFF \
    -DPLUGIN_DEVICEDIAGNOSTICS=ON \
    -DPLUGIN_LOCATIONSYNC=ON -DPLUGIN_LOCATIONSYNC_URI="http://jsonip.metrological.com/?maf=true" \
    -DPLUGIN_PERSISTENTSTORE=ON \
    -DPLUGIN_SECURITYAGENT=ON \
    -DRDK_SERVICES_TEST=ON \
    -DCMAKE_BUILD_TYPE=$MODE

  make -C build/rdkservices $THREADS && make -C build/rdkservices install $THREADS
}

installMockServer() {
  cd "$ROOT"
  docker pull mockserver/mockserver
  npm install mockserver-node --save-dev
  npm install mockserver-client --save-dev
}

if ! checkPython "Python 3"; then
  echo "python3 should be installed (for Thunder)"
  exit 1
fi
if ! checkPip "python 3"; then
  echo "pip3 should be installed (for Thunder)"
  exit 1
fi

if ! checkInstalled "glib-2.0"; then
  echo "glib-2.0 should be installed (for PersistentStore)"
  exit 1
fi
if ! checkInstalled "sqlite3"; then
  echo "sqlite3 should be installed (for PersistentStore)"
  exit 1
fi

installJsonref

checkoutThunder
buildAndInstallTools

buildAndInstallThunder

checkoutThunderInterfaces
buildAndInstallThunderInterfaces

checkWPEFramework

buildAndInstallRdkservices

installMockServer

echo "==== DONE ===="

exit 0
