#!/bin/bash

set -e

THUNDER_ROOT=$(pwd)/thunder
THUNDER_INSTALL_DIR=${THUNDER_ROOT}/install

OPTION=""
if [[ "$1" == "-D" ]]; then
    OPTION="gdb"
fi

PATH=${THUNDER_INSTALL_DIR}/usr/bin:${PATH} \
LD_LIBRARY_PATH=${THUNDER_INSTALL_DIR}/usr/lib:${THUNDER_INSTALL_DIR}/usr/lib/wpeframework/plugins:${LD_LIBRARY_PATH} \
$OPTION RdkServicesTest

echo "==== DONE ===="

exit 0
