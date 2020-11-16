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
import Websocket from 'ws'

import ThunderJS from '../src/thunderJS'
import sinon from 'sinon'

const port = 2021

const startServer = () => {
  // create a websocket server
  const server = new Websocket.Server({
    port,
  })

  server.on('connection', function connection(ws) {
    ws.on('message', function incoming(message) {})
  })

  return server
}

// on connect event should be called when ws connection is established
test('thunderJS - connect - callback when opening connection', assert => {
  const server = startServer()

  const thunderJS = ThunderJS({
    host: 'localhost',
    port,
    endpoint: '/',
  })

  // register callbacks for connect and disconnect
  const connectCallbackFake = sinon.fake()
  thunderJS.on('connect', connectCallbackFake)

  const disconnectCallbackFake = sinon.fake()
  thunderJS.on('disconnect', disconnectCallbackFake)

  // make a dummy API call to acivate connection
  thunderJS.call('Foo', 'bar')

  // give it some time to execute the callback
  setTimeout(() => {
    assert.equals(
      connectCallbackFake.callCount,
      1,
      'Connect callback should be called once after connection is established'
    )
    // close the websocket
    server.close()

    setTimeout(() => {
      assert.equals(
        disconnectCallbackFake.callCount,
        1,
        'Disconnect callback should be called once after connection is closed'
      )
      assert.end()
    }, 1000)
  }, 1000)
})
