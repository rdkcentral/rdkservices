#!/bin/sh

set -e

GDB=0

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

checkRequirements() {
  if ! checkInstalled "valgrind"; then
    echo "valgrind should be installed"
    exit 1
  fi
}

runUnitTests() {
  VGDB=""
  if [ "${GDB}" = "1" ]; then
    VGDB="--vgdb=yes --vgdb-error=0"
  fi

  # shellcheck disable=SC2086
  PATH=${THUNDER_INSTALL_DIR}/usr/bin:${PATH} \
    LD_LIBRARY_PATH=${THUNDER_INSTALL_DIR}/usr/lib:${THUNDER_INSTALL_DIR}/usr/lib/wpeframework/plugins:${LD_LIBRARY_PATH} \
    valgrind \
    --tool=memcheck \
    --log-file="${VALGRINDLOG}" \
    --leak-check=yes \
    --show-reachable=yes \
    --track-fds=yes \
    --fair-sched=try \
    ${VGDB} \
    RdkServicesTest
}

while getopts "d" option; do
  case $option in
  d)
    GDB=1
    ;;
  *) ;;

  esac
done

checkRequirements

startDummyServerDeviceDiagnostic

runUnitTests

stopDummyServerDeviceDiagnostic

echo "==== DONE ===="

exit 0
