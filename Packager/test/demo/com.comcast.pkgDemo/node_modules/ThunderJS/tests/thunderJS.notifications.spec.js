/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the License);
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
 */

import test from 'tape'
import ThunderJS from '../src/thunderJS'
import sinon from 'sinon'

import * as API from '../src/api/index'
import * as connect from '../src/api/connect'
import { listeners } from '../src/store'

const options = { host: 'localhost' }

let makeBodySpy
let apiRequestSpy
let connectStub
let makeIdStub

const resetStubsAndSpies = () => {
  makeIdStub.resetHistory()
  connectStub.resetHistory()
  apiRequestSpy.resetHistory()
  makeBodySpy.resetHistory()
}

test('Setup - thunderJS - calls', assert => {
  makeBodySpy = sinon.spy(API, 'makeBody')
  apiRequestSpy = sinon.spy(API, 'execRequest')

  connectStub = sinon.stub(connect, 'default').callsFake(() => {
    return new Promise(resolve => {
      resolve({
        // stubbed send
        send() {},
      })
    })
  })

  makeIdStub = sinon.stub(API, 'makeId').callsFake(() => {
    // always return id of 1 to facilitate testing (in isolation)
    return 1
  })

  assert.end()
})

// should be able to register a listener argument based
test('thunderJS - notifications - register a listener argument based', assert => {
  resetStubsAndSpies()

  let thunderJS = ThunderJS(options)

  const callback = () => {}
  thunderJS.on('Controller', 'all', callback)

  assert.ok(
    makeBodySpy.returned(
      sinon.match({
        jsonrpc: '2.0',
        id: 1,
        method: 'Controller.1.register',
        params: {
          event: 'all',
        },
      })
    ),
    'Should make a jsonrpc body with method Controller.1.register and params.event all'
  )

  assert.deepEquals(
    apiRequestSpy.firstCall.args[1].method,
    'Controller.1.register',
    'Should make a request for Controller.1.register'
  )

  // should have a listener registered with the callback (id of listener is client.Controller.events.all)
  assert.equals(
    listeners['client.Controller.events.all'][0],
    callback,
    'Should have a listener registered with the callback'
  )

  assert.end()
})

// should be able to register a listener object based
test('thunderJS - notifications - register a listener object based', assert => {
  resetStubsAndSpies()
  let thunderJS = ThunderJS(options)

  const callback = () => {}
  thunderJS.WebKitBrowser.on('urlchange', callback)

  assert.ok(
    makeBodySpy.returned(
      sinon.match({
        jsonrpc: '2.0',
        id: 1,
        method: 'WebKitBrowser.1.register',
        params: {
          event: 'urlchange',
        },
      })
    ),
    'Should make a jsonrpc body with method WebKitBrowser.1.register and params.event urlchange'
  )

  assert.deepEquals(
    apiRequestSpy.firstCall.args[1].method,
    'WebKitBrowser.1.register',
    'Should make a request for WebKitBrowser.1.register'
  )

  // should have a listener registered with the callback (id of listener is client.WebKitBrowser.events.urlchange)
  assert.equals(
    listeners['client.WebKitBrowser.events.urlchange'][0],
    callback,
    'Should have a listener registered with the callback'
  )

  assert.end()
})

// should execute callback each time when notification listener is called
test('thunderJS - notifications - execute callback upon each notification', assert => {
  resetStubsAndSpies()
  let thunderJS = ThunderJS(options)

  const callbackSpy = sinon.spy()

  thunderJS.WebKitBrowser.on('visibilitychange', callbackSpy)

  API.notificationListener({
    method: 'client.WebKitBrowser.events.visibilitychange',
    params: {
      hidden: false,
    },
  })

  API.notificationListener({
    method: 'client.WebKitBrowser.events.visibilitychange',
    params: {
      hidden: true,
    },
  })

  assert.ok(
    callbackSpy.calledWith({ hidden: false }),
    'Callback should be called once with hidden false'
  )

  assert.ok(
    callbackSpy.calledWith({ hidden: true }),
    'Callback should be called once with hidden true'
  )

  assert.ok(callbackSpy.calledTwice, 'Callback should be called twice')

  assert.end()
})

// should be able to register a more callback for the same listener
test('thunderJS - notifications - register a mutiple listeners or the same event', assert => {
  resetStubsAndSpies()
  let thunderJS = ThunderJS(options)

  const callbackSpy1 = sinon.spy()
  const callbackSpy2 = sinon.spy()

  thunderJS.WebKitBrowser.on('statechange', callbackSpy1)
  thunderJS.WebKitBrowser.on('statechange', callbackSpy2)

  API.notificationListener({
    method: 'client.WebKitBrowser.events.statechange',
    params: {
      suspended: false,
    },
  })

  assert.ok(callbackSpy1.calledOnce, 'Callback 1 should be called once')
  assert.ok(callbackSpy2.calledOnce, 'Callback 2 should be called once')

  assert.end()
})

// should execute callback each time when notification listener is called
test('thunderJS - notifications - error on invalid event', assert => {
  resetStubsAndSpies()
  let thunderJS = ThunderJS(options)

  const successCallbackSpy = sinon.spy()
  const errorCallbackSpy = sinon.spy()

  thunderJS.WebKitBrowser.on('invalidevent', successCallbackSpy, errorCallbackSpy)

  API.requestQueueResolver({
    id: 1,
    error: {
      code: -32603,
      message: 'Could not access requested service',
    },
  })

  setTimeout(() => {
    assert.ok(successCallbackSpy.callCount === 0, 'Notification should not be recevied')
    assert.ok(errorCallbackSpy.callCount === 1, 'Error Callback should be called once')
    assert.end()
  }, 250)
})

// should execute callback each time when notification listener is called
test('thunderJS - notifications - call dispose twice', assert => {
  resetStubsAndSpies()
  let thunderJS = ThunderJS(options)

  const TestListener = thunderJS.FooPlugin.on(
    'bar',
    () => {},
    () => {}
  )

  assert.doesNotThrow(TestListener.dispose, 'First dispose should work')
  assert.doesNotThrow(
    TestListener.dispose,
    'Second dispose should not an error, even though it no longer exists'
  )
  assert.end()
})

test('Teardown - thunderJS - calls', assert => {
  makeBodySpy.restore()
  apiRequestSpy.restore()
  connectStub.restore()
  makeIdStub.restore()

  assert.end()
})
