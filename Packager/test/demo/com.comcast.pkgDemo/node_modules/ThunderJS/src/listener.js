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

import { listeners } from './store'

export default function(plugin, event, callback, errorCallback) {
  const thunder = this

  // register and keep track of the index
  const index = register.call(this, plugin, event, callback, errorCallback)

  return {
    dispose() {
      const listener_id = makeListenerId(plugin, event)
      //early return if the listener is already deleted and someone is calling dispose twice
      if (listeners[listener_id] === undefined) return

      listeners[listener_id].splice(index, 1)

      if (listeners[listener_id].length === 0) {
        unregister.call(thunder, plugin, event, errorCallback)
      }
    },
  }
}

// construct a unique id for the listener
const makeListenerId = (plugin, event) => {
  return ['client', plugin, 'events', event].join('.')
}

const register = function(plugin, event, callback, errorCallback) {
  const listener_id = makeListenerId(plugin, event)

  // no listener registered for this plugin/event yet
  if (!listeners[listener_id]) {
    // create an array to store this plugin/event's callback(s)
    listeners[listener_id] = []

    // ThunderJS as a plugin means it's an internal event, so no need to make an API call
    if (plugin !== 'ThunderJS') {
      // request the server to send us notifications for this event
      const method = 'register'

      // remove 'event' from the listener_id to send as request id
      const request_id = listener_id
        .split('.')
        .slice(0, -1)
        .join('.')

      const params = {
        event,
        id: request_id,
      }

      this.api.request(plugin, method, params).catch(e => {
        if (typeof errorCallback === 'function') errorCallback(e.message)
      })
    }
  }

  // register the callback
  listeners[listener_id].push(callback)

  // return the index of the callback (which can be used to dispose the callback)
  return listeners[listener_id].length - 1
}

const unregister = function(plugin, event, errorCallback) {
  const listener_id = makeListenerId(plugin, event)

  delete listeners[listener_id]

  // ThunderJS as a plugin means it's an internal event, so no need to make an API call
  if (plugin !== 'ThunderJS') {
    // request the server to stop sending us notifications for this event
    const method = 'unregister'

    // remove 'event' from the listener_id to send as request id
    const request_id = listener_id
      .split('.')
      .slice(0, -1)
      .join('.')

    const params = {
      event,
      id: request_id,
    }
    this.api.request(plugin, method, params).catch(e => {
      if (typeof errorCallback === 'function') errorCallback(e.message)
    })
  }
}
