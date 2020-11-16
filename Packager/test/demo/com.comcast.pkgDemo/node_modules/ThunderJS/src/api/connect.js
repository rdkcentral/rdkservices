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

import WebSocket from 'ws'

import requestQueueResolver from './requestQueueResolver'
import notificationListener from './notificationListener'
import makeWebsocketAddress from './makeWebsocketAddress'

const protocols = 'notification'
let socket = null

export default options => {
  return new Promise((resolve, reject) => {
    //return socket
    if (socket && socket.readyState === 1) return resolve(socket)

    //wait for socket to be opened
    //FIXME OR FIXME NOT: we could throttle how many event listeners we allow while we're in "connecting" state
    if (socket && socket.readyState === 0) {
      const waitForOpen = () => {
        socket.removeEventListener('open', waitForOpen)
        resolve(socket)
      }

      return socket.addEventListener('open', waitForOpen)
    }

    // create a new socket
    if (socket === null) {
      socket = new WebSocket(makeWebsocketAddress(options), protocols)
      socket.addEventListener('message', message => {
        if (options.debug) {
          console.log(' ')
          console.log('API REPONSE:')
          console.log(JSON.stringify(message.data, null, 2))
          console.log(' ')
        }

        requestQueueResolver(message.data)
      })

      socket.addEventListener('message', message => {
        notificationListener(message.data)
      })

      socket.addEventListener('error', () => {
        notificationListener({
          method: 'client.ThunderJS.events.error',
        })
        socket = null
      })

      // Browser always first error followed by a close, never just an error event
      // so lets look at close events to detect if it worked or not
      const handleConnectClosure = event => {
        socket = null
        reject(event)
      }

      socket.addEventListener('close', handleConnectClosure)

      socket.addEventListener('open', () => {
        notificationListener({
          method: 'client.ThunderJS.events.connect',
        })

        //remove our connect close event listener to avoid sending reject() out of scope
        socket.removeEventListener('close', handleConnectClosure)

        //setup our permanent close event listeners
        socket.addEventListener('close', () => {
          notificationListener({
            method: 'client.ThunderJS.events.disconnect',
          })

          // cleanup the socket
          socket = null
        })

        resolve(socket)
      })
    } else {
      socket = null
      reject('Socket error')
    }
  })
}
