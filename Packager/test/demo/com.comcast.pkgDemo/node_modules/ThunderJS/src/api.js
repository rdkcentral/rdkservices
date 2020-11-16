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

import { makeBody, getVersion, makeId, execRequest } from './api/index'
import { requestsQueue } from './store'

export default options => {
  return {
    request(plugin, method, params) {
      return new Promise((resolve, reject) => {
        const requestId = makeId()
        const version = getVersion(options.versions, plugin, params)
        const body = makeBody(requestId, plugin, method, params, version)

        if (options.debug) {
          console.log(' ')
          console.log('API REQUEST:')
          console.log(JSON.stringify(body, null, 2))
          console.log(' ')
        }

        requestsQueue[requestId] = {
          body,
          resolve,
          reject,
        }

        execRequest(options, body).catch(e => {
          reject(e)
        })
      })
    },
  }
}
