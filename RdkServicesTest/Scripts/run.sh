#!/bin/sh

set -e

THUNDER_ROOT=$(pwd)/thunder
THUNDER_INSTALL_DIR=${THUNDER_ROOT}/install
VALGRINDLOG=$(pwd)/valgrind_log

checkInstalled() {
  command -v "$1"
  case "$?" in
  0) true ;;
  *) false ;;
  esac
}

startDummyServerDeviceDiagnostic() {
  python Scripts/DeviceDiagnosticMock.py &
  sleep 0.1
}

stopDummyServerDeviceDiagnostic() {
  pkill -f Scripts/DeviceDiagnosticMock.py
}

if ! checkInstalled "valgrind"; then
  echo "valgrind should be installed"
  exit 1
fi

startDummyServerDeviceDiagnostic

PATH=${THUNDER_INSTALL_DIR}/usr/bin:${PATH} \
  LD_LIBRARY_PATH=${THUNDER_INSTALL_DIR}/usr/lib:${THUNDER_INSTALL_DIR}/usr/lib/wpeframework/plugins:${LD_LIBRARY_PATH} \
  valgrind \
  --tool=memcheck \
  --log-file="${VALGRINDLOG}" \
  --leak-check=yes \
  --show-reachable=yes \
  --track-fds=yes \
  --fair-sched=try \
  RdkServicesTest

stopDummyServerDeviceDiagnostic

echo "==== DONE ===="

exit 0
