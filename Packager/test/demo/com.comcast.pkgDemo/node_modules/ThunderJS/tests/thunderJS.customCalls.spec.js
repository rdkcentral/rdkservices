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

const options = { host: 'localhost' }

const plugin = {
  foo() {},
  bar() {},
}

const fooSpy = sinon.spy(plugin, 'foo')
const barSpy = sinon.spy(plugin, 'bar')

test('thunderJS - calls - custom plugin - argument based', assert => {
  let thunderJS = ThunderJS(options)

  fooSpy.resetHistory()
  barSpy.resetHistory()

  thunderJS.registerPlugin('custom', plugin)

  // make calls using argument style
  thunderJS.call('custom', 'foo')
  thunderJS.call('custom', 'bar')

  assert.ok(fooSpy.calledOnce, 'Should call the foo method on the custom plugin')
  assert.ok(barSpy.calledOnce, 'Should call the bar method on the custom plugin')

  assert.end()
})

test('thunderJS - calls - custom plugin - object based', assert => {
  let thunderJS = ThunderJS(options)

  fooSpy.resetHistory()
  barSpy.resetHistory()

  thunderJS.registerPlugin('custom', plugin)

  // make calls using object style
  thunderJS.custom.foo()
  thunderJS.custom.bar()

  assert.ok(fooSpy.calledOnce, 'Should call the foo method on the custom plugin')
  assert.ok(barSpy.calledOnce, 'Should call the bar method on the custom plugin')

  assert.end()
})
