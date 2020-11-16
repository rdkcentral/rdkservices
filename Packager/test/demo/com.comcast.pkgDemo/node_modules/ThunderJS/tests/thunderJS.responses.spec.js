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

import { requestQueueResolver, notificationListener } from '../src/api/index'

const options = { host: 'localhost' }

const plugin = {
  success() {
    return new Promise(resolve => {
      resolve('😎')
    })
  },
  failure() {
    return new Promise((resolve, reject) => {
      reject('😭')
    })
  },
}

test('thunderJS - responses - promise', assert => {
  let thunderJS = ThunderJS(options)

  thunderJS.registerPlugin('custom', {
    promise() {
      return new Promise((resolve, reject) => {})
    },
    value() {
      return 'hello!'
    },
    object() {
      return { hi: 'there' }
    },
    err() {
      return new Error('this is an error')
    },
  })

  // call promise method and see if it has a then function (as promises do)
  let actual = thunderJS.custom.promise().then
  assert.ok(actual, 'Calls on thunderJS should return a promise')

  // call value method and see if it has a then function (as promises do)
  actual = thunderJS.custom.value().then
  assert.ok(
    actual,
    'Calls on thunderJS should return a promise (even if the method only returns a value)'
  )

  actual = thunderJS.custom.object().then
  assert.ok(
    actual,
    'Calls on thunderJS should return a promise (even if the method returns an object literal)'
  )

  let result = thunderJS.custom.err()
  actual = result.then
  // handle the error properly
  result.catch(err => {})

  assert.ok(
    actual,
    'Calls on thunderJS should return a promise (even if the method returns an Error)'
  )

  assert.end()
})

test('thunderJS - responses - then / catch', assert => {
  let thunderJS = ThunderJS(options)

  const successSpy = sinon.spy()
  const failureSpy = sinon.spy()

  thunderJS.registerPlugin('custom', plugin)

  assert.plan(2)

  thunderJS
    .call('custom', 'success')
    .then(successSpy)
    .catch(failureSpy)
    .finally(() => {
      assert.ok(successSpy.calledOnceWith('😎'), 'Success method should be called once')
    })

  thunderJS
    .call('custom', 'failure')
    .then(successSpy)
    .catch(failureSpy)
    .finally(() => {
      assert.ok(failureSpy.calledOnceWith('😭'), 'Failure method should be called once')
    })
})

test('thunderJS - responses - passing callback', assert => {
  let thunderJS = ThunderJS(options)

  const callback = () => {}

  const callbackSpy = sinon.spy(callback)

  thunderJS.registerPlugin('custom', plugin)

  thunderJS.call('custom', 'success', callbackSpy)
  thunderJS.call('custom', 'failure', callbackSpy)

  // next tick
  setTimeout(() => {
    assert.ok(
      callbackSpy.calledWith(null, '😎'),
      'Callback should be called once with null as first param and success as second'
    )
    assert.ok(
      callbackSpy.calledWith('😭'),
      'Callback should be called once with only the error as first param'
    )
    assert.end()
  }, 0)
})

test('thunderJS - responses - string with illegal characters for json', assert => {
  const requestQueueResolverSpy = sinon.spy(requestQueueResolver)
  const notificationListenerSpy = sinon.spy(notificationListener)

  const response =
    '{"jsonrpc":"2.0","id":2,"result":[{"ssid":"MARIRO","pairs":[{"method":"WPA","keys":["PSK","CCMP","TKIP"]},{"method":"WPA2","keys":["PSK","CCMP","TKIP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":2417,"signal":4294967254},{"ssid":"MARIRO-5GHz","pairs":[{"method":"WPA","keys":["PSK","CCMP","TKIP"]},{"method":"WPA2","keys":["PSK","CCMP","TKIP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5785,"signal":4294967229},{"ssid":"RODRIGO_2G","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":2412,"signal":4294967250},{"ssid":"RODRIGO_5G","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5200,"signal":4294967245},{"ssid":"LUCIANO","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5805,"signal":4294967211},{"ssid":"#NET-CLARO-WIFI","pairs":[{"method":"ESS"}],"frequency":2412,"signal":4294967251},{"ssid":"NET_5G546318","pairs":[{"method":"WPA","keys":["PSK","CCMP","TKIP"]},{"method":"WPA2","keys":["PSK","CCMP","TKIP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5745,"signal":4294967208},{"ssid":"MARIRO","pairs":[{"method":"WPA","keys":["PSK","CCMP","TKIP"]},{"method":"WPA2","keys":["PSK","CCMP","TKIP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":2437,"signal":4294967223},{"ssid":"Tibernet","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"ESS"}],"frequency":5745,"signal":4294967213},{"ssid":"\xf0\x9f\x92\xa9\xf0\x9f\x98\x9c","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":2462,"signal":4294967256},{"ssid":"\xf0\x9f\x9a\x80\xf0\x9f\x9a\x80\xf0\x9f\x9a\x80\xf0\x9f\x9a\x80","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5520,"signal":4294967267},{"ssid":"\xf0\x9f\x92\xa9\xf0\x9f\x98\x9c","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5520,"signal":4294967267},{"ssid":"Lucia","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5240,"signal":4294967221},{"ssid":"Lucia","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":2437,"signal":4294967235},{"ssid":"Lucia5G","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5240,"signal":4294967220},{"ssid":"2.4G ap172","pairs":[{"method":"WPA","keys":["PSK","CCMP"]},{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":2462,"signal":4294967233},{"ssid":"#NET-CLARO-WIFI","pairs":[{"method":"WPS"},{"method":"ESS"}],"frequency":2462,"signal":4294967233},{"ssid":"KEVIN","pairs":[{"method":"WPA","keys":["PSK","CCMP","TKIP"]},{"method":"WPA2","keys":["PSK","CCMP","TKIP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":2437,"signal":4294967237},{"ssid":"KEVIN-5G","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5745,"signal":4294967222},{"ssid":"DIRECT-","pairs":[{"method":"WEP"}],"frequency":2462,"signal":4294967241},{"ssid":"WiFi_Home","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"ESS"}],"frequency":5745,"signal":4294967211},{"ssid":"WiFi_Home-guest","pairs":[{"method":"WPA2","keys":["PSK","CCMP"]},{"method":"ESS"}],"frequency":5745,"signal":4294967208},{"ssid":"NETVIRTUA151_5G","pairs":[{"method":"WPA","keys":["PSK","CCMP","TKIP"]},{"method":"WPA2","keys":["PSK","CCMP","TKIP"]},{"method":"WPS"},{"method":"ESS"}],"frequency":5240,"signal":4294967207}]}'

  try {
    requestQueueResolverSpy(response)
  } catch (e) {
    //
  }

  assert.notOk(
    requestQueueResolverSpy.threw(),
    'requestQueueResolver should not have thrown an error'
  )

  try {
    notificationListenerSpy(response)
  } catch (e) {
    //
  }

  assert.notOk(
    notificationListenerSpy.threw(),
    'notificationListener should not have thrown an error'
  )

  assert.end()
})
