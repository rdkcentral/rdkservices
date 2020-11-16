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

import { requestsQueue } from '../store'

export default data => {
  if (typeof data === 'string') {
    data = JSON.parse(data.normalize().replace(/\\x([0-9A-Fa-f]{2})/g, ''))
  }
  if (data.id) {
    const request = requestsQueue[data.id]
    if (request) {
      // result can also be null, that's why we check for the existence of the key
      if ('result' in data) request.resolve(data.result)
      else request.reject(data.error)
      delete requestsQueue[data.id]
    } else {
      console.log('no pending request found with id ' + data.id)
    }
  }
}
