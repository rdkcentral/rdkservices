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

// on connect event should be called when ws connection is established
test('thunderJS - error - when opening connection', assert => {
  const thunderJS = ThunderJS({
    host: 'localhost',
    port: '1234',
    endpoint: '/',
  })

  // register callbacks for connect and disconnect
  const connectCallbackFake = sinon.fake()
  thunderJS.on('connect', connectCallbackFake)

  const errorRequestFake = sinon.fake()

  // make a dummy API call to acivate connection
  thunderJS.call('Foo', 'bar').catch(() => {
    errorRequestFake()
  })

  // give it some time to execute the callback
  setTimeout(() => {
    assert.equals(
      connectCallbackFake.callCount,
      0,
      'Connect callback should be called once after connection is established'
    )
    setTimeout(() => {
      assert.equals(
        errorRequestFake.callCount,
        1,
        'Error callback should be called once after connection is closed'
      )
      assert.end()
    }, 1000)
  }, 1000)
})
