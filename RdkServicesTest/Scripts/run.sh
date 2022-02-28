#!/bin/bash

set -e

COINTAINER_NAME="mockserver"
THUNDER_ROOT=$(pwd)/thunder
THUNDER_INSTALL_DIR=${THUNDER_ROOT}/install

OPTION=""
if [[ "$1" == "-D" ]]; then
    OPTION="gdb"
fi

# Create dummy server for port 10999 to simulate curl response for DeviceDiagnostic
python Source/DeviceDiagnosticMock.py &

# Waiting for server startup
sleep 0.1

PATH=${THUNDER_INSTALL_DIR}/usr/bin:${PATH} \
LD_LIBRARY_PATH=${THUNDER_INSTALL_DIR}/usr/lib:${THUNDER_INSTALL_DIR}/usr/lib/wpeframework/plugins:${LD_LIBRARY_PATH} \
$OPTION RdkServicesTest

# Stop dummy server
pkill -f "Source/DeviceDiagnosticMock.py"

echo "==== DONE ===="

exit 0
