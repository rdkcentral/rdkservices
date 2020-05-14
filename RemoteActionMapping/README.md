-----------------
Build:

bitbake thunder-plugins

-----------------
Test:


Methods common to typical plugins - getQuirks() and getApiVersionNumber() (not documented in official RemoteActionMapping API)
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.RemoteActionMapping.1.getQuirks"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"4","method":"org.rdk.RemoteActionMapping.1.getApiVersionNumber"}' http://127.0.0.1:9998/jsonrpc


An example getLastUsedDeviceID() call (takes no arguments)
curl -d '{"jsonrpc":"2.0","id":"5","method":"org.rdk.RemoteActionMapping.1.getLastUsedDeviceID"}' http://127.0.0.1:9998/jsonrpc


An example getKeymap() method call (takes 'deviceID' and 'keymapType' parameters)
curl -d '{"jsonrpc":"2.0","id":"6","method":"org.rdk.RemoteActionMapping.1.getKeymap","params": {"deviceID":2, "keymapType":0}}' http://127.0.0.1:9998/jsonrpc


An example setKeyActionMapping() method call (takes 'deviceID' and 'keymapType' integer parameters, and a 'keyActionMapping' array of keyActionMap objects, which contain IR waveform data, among other things).  This example is for Samsung TV only (no AVR data).
curl -d '{"jsonrpc":"2.0","id":"7","method":"org.rdk.RemoteActionMapping.1.setKeyActionMapping","params": {"deviceID":1, "keymapType":0, "keyActionMapping":[{"keyName":80,"rfKeyCode":109,"tvIRKeyCode":[4,19,4,0,34,0,210,0,144,0,4,45,101,4,101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,50,51,34,51,35,34,51,34,48],"avrIRKeyCode":[]},{"keyName":81,"rfKeyCode":108,"tvIRKeyCode":[4,19,4,0,34,0,210,0,144,0,4,45,101,4,101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,51,51,34,51,34,34,51,34,48],"avrIRKeyCode":[]},{"keyName":128,"rfKeyCode":107,"tvIRKeyCode":[4,19,4,0,34,0,210,0,144,0,4,45,101,4,101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,51,35,51,51,50,50,34,34,32],"avrIRKeyCode":[]},{"keyName":138,"rfKeyCode":65,"tvIRKeyCode":[4,17,4,0,34,0,210,0,144,0,4,45,101,4,101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,50,34,51,51,51,51,34,34,32],"avrIRKeyCode":[]},{"keyName":139,"rfKeyCode":66,"tvIRKeyCode":[4,17,4,0,34,0,210,0,144,0,4,45,101,4,101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,50,35,35,51,51,50,50,34,32],"avrIRKeyCode":[]},{"keyName":140,"rfKeyCode":67,"tvIRKeyCode":[4,17,4,0,34,0,210,0,144,0,4,45,101,4,101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,50,34,35,51,51,51,50,34,32],"avrIRKeyCode":[]},{"keyName":208,"rfKeyCode":52,"tvIRKeyCode":[4,17,4,0,34,0,210,0,144,0,4,45,101,4,101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,50,51,51,51,51,34,34,34,32],"avrIRKeyCode":[]}]}}' http://127.0.0.1:9998/jsonrpc


An example setFiveDigitCode() method call (takes 'deviceID', 'tvFiveDigitCode', and 'avrFiveDigitCode' integer parameters)
curl -d '{"jsonrpc":"2.0","id":"8","method":"org.rdk.RemoteActionMapping.1.setFiveDigitCode","params": {"deviceID":1, "tvFiveDigitCode":12051, "avrFiveDigitCode":32610}}' http://127.0.0.1:9998/jsonrpc

Another setFiveDigitCode() example - this one clears both the TV and AVR 5-digit-codes, if they are set
curl -d '{"jsonrpc":"2.0","id":"8","method":"org.rdk.RemoteActionMapping.1.setFiveDigitCode","params": {"deviceID":1, "tvFiveDigitCode":0, "avrFiveDigitCode":0}}' http://127.0.0.1:9998/jsonrpc

Another setFiveDigitCode() example - this one sets the AVR 5-digit-code, but leaves any existing TV 5-digit-code untouched
curl -d '{"jsonrpc":"2.0","id":"8","method":"org.rdk.RemoteActionMapping.1.setFiveDigitCode","params": {"deviceID":1, "tvFiveDigitCode":0, "avrFiveDigitCode":32610}}' http://127.0.0.1:9998/jsonrpc


An example clearKeyActionMapping() method call (takes 'deviceID' and 'keymapType', integer parameters, and a 'keyNames' byte array of KED key values, to indicate which database slots to clear).  The example below clears them all (the most reasonable choice).
curl -d '{"jsonrpc":"2.0","id":"9","method":"org.rdk.RemoteActionMapping.1.clearKeyActionMapping","params": {"deviceID":1, "keymapType":0, "keyNames":[128,81,80,138,139,140,208]}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"9","method":"org.rdk.RemoteActionMapping.1.clearKeyActionMapping","params": {"deviceID":1, "keymapType":0, "keyNames":[0x80,0x51,0x50,0x8A,0x8B,0x8C,0xD0]}}' http://127.0.0.1:9998/jsonrpc


An example getFullKeyActionMapping() method call (takes 'deviceID' and 'keymapType' integer parameters).
curl -d '{"jsonrpc":"2.0","id":"10","method":"org.rdk.RemoteActionMapping.1.getFullKeyActionMapping","params": {"deviceID":1, "keymapType":0}}' http://127.0.0.1:9998/jsonrpc


An example getSingleKeyActionMapping() method call (takes 'deviceID', 'keymapType', and 'keyName' integer parameters). KED_MUTE used here.
curl -d '{"jsonrpc":"2.0","id":"11","method":"org.rdk.RemoteActionMapping.1.getSingleKeyActionMapping","params": {"deviceID":1, "keymapType":0, "keyName":140}}' http://127.0.0.1:9998/jsonrpc


An example cancelCodeDownload() method call (takes 'deviceID' integer parameter).
curl -d '{"jsonrpc":"2.0","id":"12","method":"org.rdk.RemoteActionMapping.1.cancelCodeDownload","params": {"deviceID":2}}' http://127.0.0.1:9998/jsonrpc


