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
import sinon from 'sinon'

import ThunderJS from '../src/thunderJS'
import * as API from '../src/api/index'
import * as ws from 'ws'

let wsStub

const makeWsStub = () => {
  return sinon.stub(ws, 'default').callsFake(address => {
    const websocket = {
      addEventListener() {},
      readyState: 1,
    }

    return websocket
  })
}

test('Setup - thunderJS - api', assert => {
  wsStub = makeWsStub()

  assert.end()
})

test('makeWebsocketAddress - unit test', assert => {
  const address = API.makeWebsocketAddress()
  const address2 = API.makeWebsocketAddress({
    host: '192.168.1.100',
    port: 2020,
    endpoint: '/api',
    protocol: 'wss://',
  })
  const address3 = API.makeWebsocketAddress({
    token: 'token1234',
  })

  assert.equals(
    address,
    'ws://localhost:80/jsonrpc',
    'makeWebsocketAddress should return default addresss without options'
  )
  assert.equals(
    address2,
    'wss://192.168.1.100:2020/api',
    'makeWebsocketAddress should return customized addresss when options are passed'
  )

  assert.equals(
    address3,
    'ws://localhost:80/jsonrpc?token=token1234',
    'makeWebsocketAddress should return with token as a query param when passed as an option'
  )

  assert.end()
})

test('thunderJS - api - window.thunder.token() websocket connection', assert => {
  // make thunder.token() available
  global.window = {}
  window.thunder = {
    token() {
      return 'thundertoken123'
    },
  }

  const options = {
    host: '192.168.1.100',
    port: 2020,
    endpoint: '/api',
    protocol: 'wss://',
  }
  let thunderJS = ThunderJS(options)

  // make a call, to initiate a (stubbed) websocket connection
  thunderJS.DeviceInfo.systeminfo()

  assert.ok(
    wsStub.calledWith('wss://192.168.1.100:2020/api?token=thundertoken123'),
    'Websocket with default custom address should be initiated'
  )

  delete global.window
  assert.end()
})

test('thunderJS - api - options.token websocket connection', assert => {
  const options = {
    host: '192.168.1.100',
    port: 2020,
    endpoint: '/api',
    protocol: 'wss://',
    token: 'thundertoken123',
  }
  let thunderJS = ThunderJS(options)

  // make a call, to initiate a (stubbed) websocket connection
  thunderJS.DeviceInfo.systeminfo()

  assert.ok(
    wsStub.calledWith('wss://192.168.1.100:2020/api?token=thundertoken123'),
    'Websocket with default custom address should be initiated'
  )

  assert.end()
})

test('Teardown - thunderJS - api', assert => {
  wsStub.restore()

  assert.end()
})
