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

// defaults
const protocol = 'ws://'
const host = 'localhost'
const endpoint = '/jsonrpc'
const port = 80

export default options => {
  return [
    (options && options.protocol) || protocol,
    (options && options.host) || host,
    ':' + ((options && options.port) || port),
    (options && options.endpoint) || endpoint,
    options && options.token ? '?token=' + options.token : null,
  ].join('')
}
