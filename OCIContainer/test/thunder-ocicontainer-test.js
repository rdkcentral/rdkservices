/*
 * Thunder Plugin OCIContainer test
 *
 * author: Justin Ware
 * author: Antti Kyllonen
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
    let containerName = '';
    let testDobbySpec = '';
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
        log('\trequest: ' + cmd);

        let interval;
        const checkInputResponses = () => {
        for (let i = 0; i < wsResps.length; i++) {
            const e = wsResps[i];
            const resp = JSON.parse(e);
            if (resp.id !== undefined && resp.id === responseId) {
            log('\tresponse: ' + e);
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
            testResults.failed('\ttimeout for request: \'' + method + '\'');
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
        log('\twaitEvent \'' + id + '.' + name + '\' start');

        // Returns true if the response has been found
        const checkInputResponses = () => {
        for (let i = 0; i < wsResps.length; i++) {
            const resp = JSON.parse(wsResps[i]);
            if (resp.method !== undefined && resp.method === id + '.' + name) {
            log('\twaitEvent \'' + id + '.' + name + '\' received: ' + JSON.stringify(resp.params));
            wsResps.splice(i, 1);

            // Is the response to be checked?
            let finished = true;
            if (check !== undefined) {
                // By returning true it implies that more events of this time are expected
                let moreData = check(resp.params)
                if ((moreData !== undefined) && (moreData === true))
                {
                finished = false;
                log('\twaitEvent expecting more results...');
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
            testResults.failed('\twaitEvent \'' + id + '.' + name + '\' timeout');
            reject();
        }
        }, checkPeriod);
    });
    };

    pluginName = 'org.rdk.OCIContainer'; // Change to 'OCIContainer' if running outside RDK builds
    pluginVersion = '1';
    containerName = 'sleepy';
    testDobbySpec = {
        "version": "1.0",
        "cwd": "/",
        "args": [
            "sleep",
            "30"
        ],
        "env": [

        ],
        "user": {
            "uid": 1000,
            "gid": 1000
        },
        "console": {
            "limit": 65536,
            "path": "/tmp/container.log"
        },
        "etc": {
            "group": [
                "root:x:0:"
            ],
            "passwd": [
                "root::0:0:root:/:/bin/false"
            ]
        },
        "memLimit": 41943040,
        "network": "nat",
        "mounts": [

        ]
    };

    // Run the tests.
    connect()
    .catch( error => { log('web socket can\'t be opened: ' + error);})
    .then( () => {
        log("Register 'onContainerStarted' notification listener.");
        return invoke('register', { event: 'onContainerStarted', id: 1}, undefined, (result) => {
            testResults.isTrue(result === 0, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Register 'onContainerStopped' notification listener.");
        return invoke('register', { event: 'onContainerStopped', id: 2}, undefined, (result) => {
            testResults.isTrue(result === 0, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Starting container '" + containerName + "' from a Dobby Spec.");
        return invoke('startContainerFromDobbySpec', {containerId: containerName, dobbySpec: testDobbySpec}, undefined, (result) => {
            testResults.isTrue(result.success, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Wait for 'onContainerStarted' notification.");
        return waitEvent('onContainerStarted', 1, undefined, (params) => {
            testResults.isTrue(('descriptor' in params) && (params.name === containerName),
            "Expected 'params.name' to be " + containerName);
        });
    })
    .then( () => {
        log("List containers.");
        return invoke('listContainers', {}, undefined, (result) => {
            testResults.isTrue(result.success, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Get info of container '" + containerName + "'.");
        return invoke('getContainerInfo', {containerId: containerName}, undefined, (result) => {
            testResults.isTrue(result.success, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Pause the container '" + containerName + "'.");
        return invoke('pauseContainer', {containerId: containerName}, undefined, (result) => {
            testResults.isTrue(result.success, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Check container " + containerName + " state to make sure it's paused.");
        return invoke('getContainerState', {containerId: containerName}, undefined, (params) => {
            testResults.isTrue(('containerId' in params) && (params.state === "Paused"), "Expected 'params.state' to be 'Paused'");
        });
    })
    .then( () => {
        log("Resume the container '" + containerName + "'.");
        return invoke('resumeContainer', {containerId: containerName}, undefined, (result) => {
            testResults.isTrue(result.success, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Check container " + containerName + " to make sure it's running again.");
        return invoke('getContainerState', {containerId: containerName}, undefined, (params) => {
            testResults.isTrue(('containerId' in params) && (params.state === "Running"), "Expected 'params.state' to be 'Running'");
        });
    })
    .then( () => {
        log("Execute 'ls' in container '" + containerName + "'.");
        return invoke('executeCommand', {containerId: containerName, options: "", command: "ls"}, undefined, (result) => {
            testResults.isTrue(result.success, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Stop the container '" + containerName + "'.");
        return invoke('stopContainer', {containerId: containerName, force: "false"}, undefined, (result) => {
            testResults.isTrue(result.success, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Wait for 'onContainerStopped' notification.");
        return waitEvent('onContainerStopped', 2, undefined, (params) => {
            testResults.isTrue(('descriptor' in params) && (params.name === containerName),
            "Expected 'params.name' to be " + containerName);
        });
    })
    .then( () => {
        log("Unregister 'onContainerStarted' notification listener.");
        return invoke('unregister', { event: 'onContainerStarted', id: 1}, undefined, (result) => {
            testResults.isTrue(result === 0, 'Expected to succeed');
        });
    })
    .then( () => {
        log("Unregister 'onContainerStopped' notification listener.");
        return invoke('unregister', { event: 'onContainerStopped', id: 2}, undefined, (result) => {
            testResults.isTrue(result === 0, 'Expected to succeed');
        });
    })
    .then( () => {
        // There shouldn't be any unhandled responses / events
        return new Promise((resolve) => {
        let unhandledCount = 0;
        for (const resp of wsResps) {
            let respObj = JSON.parse(resp);

            if (unhandledCount === 0)
            log('Remaining responses / events');
            log('  ' + resp);
            ++unhandledCount;
        }

        testResults.isTrue(unhandledCount === 0,
            "Expected not to have unhandled responses / events.");
        resolve();
        });
    })
    .then( () => {
        testResults.report();
        // TODO - need to close Spark
    });
}).catch( err => { console.error('Import failed: ' + err); });
