#!/bin/sh

set -e

MODE="Release"
TESTS_ONLY=0

THUNDER_ROOT=$(pwd)/thunder
THUNDER_INSTALL_DIR=${THUNDER_ROOT}/install

THUNDER_BINDING=127.0.0.1
THUNDER_PORT=55555

THUNDER_URL=https://github.com/rdkcentral/Thunder
THUNDER_BRANCH=R2
THUNDER_REV=tags/R2-v1.9

INTERFACES_URL=https://github.com/rdkcentral/ThunderInterfaces
INTERFACES_BRANCH=R2
INTERFACES_REV=4ec7c6f8e14b152143a105a41f4c62b6723372c4

INCLUDE_DIR=$(pwd)/Include

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

checkPackage() {
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

  make -C build/ThunderTools && make -C build/ThunderTools install
}

buildAndInstallThunder() {
  cd "${THUNDER_ROOT}" || exit 1

  cmake -HThunder -Bbuild/Thunder \
    -DCMAKE_INSTALL_PREFIX="${THUNDER_INSTALL_DIR}/usr" \
    -DCMAKE_MODULE_PATH="${THUNDER_INSTALL_DIR}/tools/cmake" \
    -DBUILD_TYPE=Debug -DBINDING="${THUNDER_BINDING}" -DPORT="${THUNDER_PORT}"

  make -C build/Thunder && make -C build/Thunder install
}

buildAndInstallThunderInterfaces() {
  cd "${THUNDER_ROOT}" || exit 1

  cmake -HThunderInterfaces -Bbuild/ThunderInterfaces \
    -DCMAKE_INSTALL_PREFIX="${THUNDER_INSTALL_DIR}/usr" \
    -DCMAKE_MODULE_PATH="${THUNDER_INSTALL_DIR}/tools/cmake"

  make -C build/ThunderInterfaces && make -C build/ThunderInterfaces install
}

buildAndInstallRdkservices() {
  cd "${THUNDER_ROOT}" || exit 1

  cmake -H../.. -Bbuild/rdkservices \
    -DCMAKE_INSTALL_PREFIX="${THUNDER_INSTALL_DIR}/usr" \
    -DCMAKE_MODULE_PATH="${THUNDER_INSTALL_DIR}/tools/cmake" \
    -DCMAKE_CXX_FLAGS="-I ${INCLUDE_DIR} --coverage -Wall -Werror -Wno-unused-parameter -Wno-unused-result" \
    -DCOMCAST_CONFIG=OFF \
    -DPLUGIN_DATACAPTURE=ON \
    -DPLUGIN_DEVICEDIAGNOSTICS=ON \
    -DPLUGIN_LOCATIONSYNC=ON \
    -DPLUGIN_PERSISTENTSTORE=ON \
    -DPLUGIN_SECURITYAGENT=ON \
    -DPLUGIN_DEVICEIDENTIFICATION=ON -DBUILD_REALTEK=ON \
    -DPLUGIN_FRAMERATE=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_DS=ON -DCMAKE_DISABLE_FIND_PACKAGE_IARMBus=ON \
    -DPLUGIN_AVINPUT=ON \
    -DRDK_SERVICES_TEST=ON \
    -DCMAKE_BUILD_TYPE=$MODE

  make -C build/rdkservices && make -C build/rdkservices install
}

checkRequirements() {
  if ! checkPython "Python 3"; then
    echo "python3 should be installed (for Thunder)"
    exit 1
  fi
  if ! checkPip "python 3"; then
    echo "pip3 should be installed (for Thunder)"
    exit 1
  fi
  if ! checkPackage "sqlite3"; then
    echo "sqlite3 should be installed (for PersistentStore)"
    exit 1
  fi
  if ! checkPackage "libcurl"; then
    echo "libcurl should be installed (for DataCapture, DeviceDiagnostics)"
    exit 1
  fi
}

while getopts "Dt" option; do
  case $option in
  D)
    MODE="Debug"
    ;;
  t)
    TESTS_ONLY=1
    ;;
  *) ;;

  esac
done

checkRequirements

installJsonref

if [ "${TESTS_ONLY}" != "1" ]; then

  checkoutThunder
  buildAndInstallTools

  buildAndInstallThunder

  checkoutThunderInterfaces
  buildAndInstallThunderInterfaces

  checkWPEFramework

fi

buildAndInstallRdkservices

echo "==== DONE ===="

exit 0
