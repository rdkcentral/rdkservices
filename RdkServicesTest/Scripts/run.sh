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
if [ $( docker ps -a | grep ${COINTAINER_NAME} | wc -l ) -eq 0 ]; then
    docker run -d --rm --name mockserver -p 10999:10999 mockserver/mockserver -serverPort 10999 -logLevel INFO -proxyRemoteHost 127.0.0.1
fi

echo "Waiting for server startup"
sleep 0.5;
echo "Server is up"

# Create an expectations for server
node Source/DeviceInfoMock.js
echo "Expectations were set"

PATH=${THUNDER_INSTALL_DIR}/usr/bin:${PATH} \
LD_LIBRARY_PATH=${THUNDER_INSTALL_DIR}/usr/lib:${THUNDER_INSTALL_DIR}/usr/lib/wpeframework/plugins:${LD_LIBRARY_PATH} \
$OPTION RdkServicesTest

# Stop dummy server
docker kill ${COINTAINER_NAME}

echo "==== DONE ===="

exit 0
