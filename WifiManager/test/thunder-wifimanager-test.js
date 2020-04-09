/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

/*
 * Spark Thunder Plugin WifiManager test
 *
 */

'use strict';

const startTime =  new Date().valueOf();
function log(message) { console.log(((new Date().valueOf() - startTime) / 1000).toFixed(3) + '  ' + message); }

px.import({
  scene:'px:scene.1.js',
  ws: 'ws'
}).then( function ready(imported) {
  const WebSocket = imported.ws;

  let ws;
  let wsResps = [];
  const connect = () => {
    return new Promise( (resolve, reject) => {
      ws = new WebSocket('ws://localhost:9998/jsonrpc', 'notification');
      ws.onopen = () => { resolve(ws); };
      ws.onerror = error => { reject(error); };
      ws.onmessage = e => { wsResps.push(e.data); }
    });
  };

  let responseId = 0;
  let pluginName = '';
  let pluginVersion = '';
  const responseTimeout = 10000;
  const checkPeriod = 500;
  // How many tests passed and failed
  let testResults = {
    passed_: 0,
    failed_: 0,

    /**
     * A test has passed.
     */
    passed: function() {
      ++this.passed_;
    },

    /**
     * A test has failed.
     *
     * @param reason The reason why.
     */
    failed: function(reason) {
      ++this.failed_;

      log('FAILED: ' + reason);
    },

    /**
     * Test if something is true.
     *
     * @param check       If true then the test has passed.
     * @param description A description of the test.
     */
    isTrue: function(check, description) {
      if (check)
        this.passed();
      else
        this.failed(description);
    },

    /**
     * Report the results.
     */
    report: function() {
      if (this.failed_ === 0)
        log('Test Results: all ' + this.passed_ + ' tests passed');
      else
        log('Test Results: ' + this.passed_ + ' passed; ' + this.failed_ + ' failed');
    }
  };

  /**
   * Invoke a method and process response.
   *
   * @param {*} method  The name of the method.
   * @param {*} params  Any parameters as object of names and values.
   * @param {*} timeout Optional timeout to wait for response (defaults to 10s).
   * @param {*} check   Optional function used to check response.
   */
  const invoke = (method, params, timeout, check) => {
    return new Promise( (resolve, reject) => {
      const cmd = JSON.stringify({ jsonrpc: '2.0', id: ++responseId, method: pluginName + '.' + pluginVersion + '.' + method, params: params === undefined ? {} : params } );
      ws.send(cmd);
      log('request: ' + cmd);

      let interval;
      const checkInputResponses = () => {
        for (let i = 0; i < wsResps.length; i++) {
          const e = wsResps[i];
          const resp = JSON.parse(e);
          if (resp.id !== undefined && resp.id === responseId) {
            log('response: ' + e);
            if (interval !== undefined) clearInterval(interval);
            wsResps.splice(i, 1);

            if (check !== undefined)
              check(resp.result);
            resolve();
          }
        }
      };
      if (timeout === undefined) timeout = responseTimeout;
      checkInputResponses();
      interval = setInterval(() => {
        checkInputResponses();
        timeout -= checkPeriod;
        if (timeout <= 0) {
          clearInterval(interval);
          testResults.failed('timeout for request: \'' + method + '\'');
          reject();
        }
      }, checkPeriod);
    });
  };

  /**
   * Wait for an event from Thunder.
   *
   * @param {*} name    The name of the event.
   * @param {*} id      The id of the request associated with the event.
   * @param {*} timeout An optional timeout to override the default one.
   * @param {*} check   Optional function used to check event. Return true if more events in this sequence are expected.
   */
  const waitEvent = (name, id, timeout, check) => {
    return new Promise( (resolve, reject) => {
      log('waitEvent \'' + id + '.' + name + '\' start');

      // Returns true if the response has been found
      const checkInputResponses = () => {
        for (let i = 0; i < wsResps.length; i++) {
          const resp = JSON.parse(wsResps[i]);
          if (resp.method !== undefined && resp.method === id + '.' + name) {
            log('waitEvent \'' + id + '.' + name + '\' received: ' + JSON.stringify(resp.params));
            wsResps.splice(i, 1);

            // Is the response to be checked?
            let finished = true;
            if (check !== undefined) {
              // By returning true it implies that more events of this time are expected
              let moreData = check(resp.params)
              if ((moreData !== undefined) && (moreData === true))
              {
                finished = false;
                log('waitEvent expecting more results...');
              }
            }

            if (finished) {
              resolve(resp);
              return true;
            }
          }
        }

        // Haven't found the event
        return false;
      };
      if (timeout === undefined) timeout = responseTimeout;
      if (checkInputResponses())
        return;
      let interval = setInterval(() => {
        // Have we found the response?
        if (checkInputResponses()) {
          clearInterval(interval);
          return;
        }

        // Have we timed out?
        timeout -= checkPeriod;
        if (timeout <= 0) {
          clearInterval(interval);
          testResults.failed('waitEvent \'' + id + '.' + name + '\' timeout');
          reject();
        }
      }, checkPeriod);
    });
  };

  /**
   * The order of events is:
   *   1) register for events 'onWifiSignalThresholdChanged';
   *   2) check isSignalThresholdChangeEnabled value at start;
   *   3) invoke setSignalThresholdChangeEnabled = true;
   *   4) wait for 'onWifiSignalThresholdChanged' event;
   *   5) check isSignalThresholdChangeEnabled value after setSignalThresholdChangeEnabled call;
   *   6) invoke setSignalThresholdChangeEnabled = false;
   *   7) check isSignalThresholdChangeEnabled after the second setSignalThresholdChangeEnabled call;
   *
   */
  const testSetSignalThresholdChangeEnabled = () => {
    return new Promise( (resolve, reject) => {
        log('testSetSignalThresholdChangeEnabled - started');

        // Register to get events and wait for event
        const registerId = ++responseId;
        const eventId = ++responseId;

        let wait = waitEvent("onWifiSignalThresholdChanged", eventId, 5000);

        let cmd = JSON.stringify({ jsonrpc: '2.0', id: registerId, method: pluginName + '.' + pluginVersion + '.register', params: { event: 'onWifiSignalThresholdChanged', id: eventId } });
        ws.send(cmd);
        log('registered for events; id = ' + registerId);

        // Check state at the beginning
        let checkAtStart = invoke('isSignalThresholdChangeEnabled', { }, undefined, (result) => {
            testResults.isTrue(!('result' in result) || (result.result !== 0), 'SignalThresholdChange should be disabled at start');
        });

        checkAtStart.then( () => {
            // Invoke setSignalThresholdChangeEnabled
            let setSignalEnable = invoke('setSignalThresholdChangeEnabled', {enabled: true, interval: 1000}, undefined, (result) => {});

            wait.then(
                function(result) {
                    testResults.isTrue('signalStrength' in result.params && 'strength' in result.params, 'Malformed response');

                    setSignalEnable.then( () => {
                        let checkAfterCall = invoke('isSignalThresholdChangeEnabled', { }, undefined, (result) => {
                            testResults.isTrue(!('result' in result) || (result.result === 0), 'SignalThresholdChange should be enabled after setSignalThresholdChangeEnabled = true call');
                        });

                        checkAfterCall.then( () => {
                            let setSignalDisable = invoke('setSignalThresholdChangeEnabled', {enabled: false, interval: 1000}, undefined, (result) => {});

                            setSignalDisable.then( () => {
                                let checkAtEnd = invoke('isSignalThresholdChangeEnabled', { }, undefined, (result) => {
                                    testResults.isTrue(!('result' in result) || (result.result !== 0), 'SignalThresholdChange should be disabled after setSignalThresholdChangeEnabled = false call');
                                });
                                checkAtEnd
                                    .then( () => { resolve(); })
                                    .catch((reason) => { reject(reason); });
                            })
                            .catch((reason) => { reject(reason); });
                        });
                    });
                }
            )
            .catch((reason) => { reject(reason); });
        })
        .catch((reason) => { reject(reason); });
    });
  }

  /**
   * Test that stopping a scan works. The order of events is:
   *   1) register for events 'onAvailableSSIDsIncr';
   *   2) on response to that start 'getAvailableSSIDsAsyncIncr';
   *   3) on first 'onAvailableSSIDsIncr' send 'stopScanning';
   *
   */
  const testStopScanning = () => {
    return new Promise( (resolve, reject) => {
      log('testStopScan - started');

      // Register to get events
      const registerId = ++responseId;
      const eventId = ++responseId;
      let cmd = JSON.stringify({ jsonrpc: '2.0', id: registerId, method: pluginName + '.' + pluginVersion + '.register', params: { event: 'onAvailableSSIDs', id: eventId } });
      ws.send(cmd);
      log('registered for events; id = ' + registerId);

      // Wait for responses and send new requests
      let timeout = 10000;
      let scanId = undefined;
      let stopId = undefined;
      let responseCount = { register: 0, scan: 0, stop: 0, event: 0 };
      let interval = setInterval(() => {
        // Examine the request responses and events on the websocket
        for (let i = 0; i < wsResps.length; i++) {
          const e = wsResps[i];
          const resp = JSON.parse(e);

          // Request responses
          if (resp.id !== undefined) {
            if (resp.id == registerId) {
              wsResps.splice(i, 1);
              ++responseCount.register;
              testResults.isTrue(resp.result === 0, 'Expected register to succeed');

              // We've registered for events, now start a scan
              scanId = ++responseId;
              cmd = JSON.stringify({ jsonrpc: '2.0', id: scanId, method: pluginName + '.' + pluginVersion + '.startScan', params: {incremental: true} });
              ws.send(cmd);
              log('started scan; id = ' + scanId);
            } else if ((scanId !== undefined) && (resp.id == scanId)) {
              // This is the response to the scan request but the scan results come back as events
              wsResps.splice(i, 1);
              ++responseCount.scan;
              testResults.isTrue(resp.result.success === true, 'Expected scan initiation to succeed');
            } else if ((stopId !== undefined) && (resp.id == stopId)) {
              // This is the response to the stop request
              wsResps.splice(i, 1);
              ++responseCount.stop;
              testResults.isTrue(resp.result.success === true, 'Expected stop to succeed');
            }
          }

          // Events
          if ((resp.method !== undefined) && (resp.method === eventId + '.onAvailableSSIDs')) {
            wsResps.splice(i, 1);
            ++responseCount.event;
            testResults.isTrue((typeof(resp.params.ssids) === 'object') && (resp.params.ssids.length > 1) && (resp.params.moreData === true),
              'Expected each incremental scan to succeed');

            if (responseCount.event === 1) {
              // Try to cancel the scan
              stopId = ++responseId;
              cmd = JSON.stringify({ jsonrpc: '2.0', id: stopId, method: pluginName + '.' + pluginVersion + '.stopScan', params: {} });
              ws.send(cmd);
              log('stopping scan; id = ' + stopId);
            }
          }
        }

        // Have we timed out?
        timeout -= checkPeriod;
        if (timeout <= 0) {
          clearInterval(interval);

          testResults.isTrue((responseCount.register === 1) && (responseCount.scan === 1) && (responseCount.stop === 1) && (responseCount.event < 3),
            'Should only register / scan / stop once and receive less than 3 events');
          log('testStopScanned - finished');
          resolve();
        }
      }, checkPeriod);
    });
  };

  pluginName = 'WifiManager';
  pluginVersion = '1';

  // Some test connection data
  let testConnectionData = {
    ssid: 'REDGuest',
    incorrectPassphrase: 'wibble',
    correctPassphrase: 'R3Dguest',
    securityMode: 6
  };

  // Run the tests. These tests assume that wifi is enabled but not currently connected to an access point.
  connect()
    .catch( error => { log('web socket can\'t be opened: ' + error);})
    .then( () => {
      return invoke('register', { event: 'onAvailableSSIDs', id: 1}, undefined, (result) => {
        testResults.isTrue(result === 0, 'Expected to succeed');
      });
    })
    .then( () => {
      return invoke('startScan', {incremental: false}, undefined, (result) => {
        testResults.isTrue(result.success, 'Expected to succeed');
      });
    })
    .then( () => {
      return waitEvent('onAvailableSSIDs', 1, (params) => {
        testResults.isTrue((typeof(params.ssids) === 'object') && (params.ssids.length > 1),
          "Expected an array of access points in 'getAvaiableSSIDs'");
      });
    })
    .then( () => {
      return invoke('startScan', {incremental: true}, undefined, (result) => {
        testResults.isTrue(result.success, 'Expected to succeed');
      });
    })
    .then( () => {
      return waitEvent('onAvailableSSIDs', 1, undefined, (params) => {
        testResults.isTrue((typeof(params.ssids) === 'object') && (params.ssids.length > 1) && ('moreData' in params),
          "Expected an array of access points in 'ssids' and 'moreData' key");

        return ('moreData' in params) && params.moreData;
      });
    })
    .then( () => {
      // Test looking for a specific access point
      return invoke('startScan', { incremental: false, ssid: testConnectionData.ssid, frequency: "2.4"}, undefined, (result) => {
        testResults.isTrue(result.success, 'Expected to succeed');
      });
    })
    .then( () => {
      return waitEvent('onAvailableSSIDs', 1, (params) => {
        testResults.isTrue((typeof(params.ssids) === 'object') && (params.ssids.length > 1),
          "Expected an array of access points in 'getAvaiableSSIDs'");
      });
    })
    .then( () => {
      return invoke('unregister', { event: 'onAvailableSSIDs', id: 1}, undefined, (result) => {
        testResults.isTrue(result === 0, 'Expected to succeed');
      });
    })
    .then( () => {
      // Complex test to stop scanning
      return testStopScanning();
    })
    .then( () => {
      // Start listening to errors
      return invoke('register', { event: 'onError', id: 3}, undefined, (result) => {
        testResults.isTrue(result === 0, 'Expected to succeed');
      });
    })
    .then( () => {
      // Start listening to wifi state changes
      return invoke('register', { event: 'onWIFIStateChanged', id: 4}, undefined, (result) => {
        testResults.isTrue(result === 0, 'Expected to succeed');
      });
    })
    .then( () => {
      // Try to connect with wrong passphase, connect doesn't return anything that's useful to test
      return invoke('connect', { ssid: testConnectionData.ssid, passphrase: testConnectionData.incorrectPassphrase, securityMode: testConnectionData.securityMode });
    })
    .then( () => {
      // Expect state change to connecting...
      return waitEvent('onWIFIStateChanged', 4, undefined, (params) => {
        testResults.isTrue(('state' in params) && (params.state === 4),
          "Expected 'params.state' to be 4 (CONNECTING)");
      });
    })
    .then( () => {
      // ...but the credentials are wrong
      return waitEvent('onError', 3, undefined, (params) => {
        testResults.isTrue(('code' in params) && (params.code === 4),
          "Expected 'params.code' to be 4 (INVALID_CREDENTIALS)");
      });
    })
    .then( () => {
      // Try again with the right passphrase
      return invoke('connect', { ssid: testConnectionData.ssid, passphrase: testConnectionData.correctPassphrase, securityMode: testConnectionData.securityMode });
    })
    .then( () => {
      // Expect state change to connecting.
      return waitEvent('onWIFIStateChanged', 4, undefined, (params) => {
        // Might get several 'CONNECTING' events before 'CONNECTED'
        if (('state' in params) && (params.state === 4))
          return true;

        testResults.isTrue(('state' in params) && (params.state === 5),
          "Expected 'params.state' to (eventually) be 5 (CONNECTED)");
        return false;
      });
    })
    .then( () => {
      // Stop receiving 'onError' events. There will be 'SSID_CHANGED' and possibly 'NO_SSID' as we disconnected but it's not entirely predictable.
      return invoke('unregister', { event: 'onError', id: 3}, undefined, (result) => {
        testResults.isTrue(result === 0, 'Expected to succeed');
      });
    })
    .then( () => {
      // Try to disconnect, disconnect doesn't return anything that's useful to test
      return invoke('disconnect', {});
    })
    .then( () => {
      // Expect state change to disconnected
      return waitEvent('onWIFIStateChanged', 4, undefined, (params) => {
        // Might get several 'CONNECTING' events before 'DISCONNECTED'
        if (('state' in params) && (params.state === 4))
          return true;

        testResults.isTrue(('state' in params) && (params.state === 2),
          "Expected 'params.state' to (eventually) be 2 (DISCONNECTED)");
        return false;
      });
    })
    .then( () => {
      // There shouldn't be any unhandled responses / events
      return new Promise((resolve) => {
        let unhandledCount = 0;
        for (const resp of wsResps) {
          let respObj = JSON.parse(resp);

          // Sometimes we get 'CONNECTING' out of sequence
          if ((respObj.method !== undefined) && respObj.method.endsWith('.onWIFIStateChanged') && (respObj.params.state === 4))
            continue;

          if (unhandledCount === 0)
            log('Remaining responses / events');
          log('  ' + resp);
          ++unhandledCount;
        }

        testResults.isTrue(unhandledCount === 0,
          "Expected not to have unhandled responses / events (except some known ones)");
        resolve();
      });
    })
    .then( () => {
      // Complex test for set signal treshold
      return testSetSignalThresholdChangeEnabled();
    })
    .then( () => {
      testResults.report();
      // TODO - need to close Spark
    });

}).catch( err => { console.error('Import failed: ' + err); });
