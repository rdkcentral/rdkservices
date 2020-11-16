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

export default (requestId, plugin, method, params, version) => {
  // delete possible version key from params
  params ? delete params.version : null
  const body = {
    jsonrpc: '2.0',
    id: requestId,
    method: [plugin, version, method].join('.'),
  }

  // params exist (or explicitely false)
  params || params === false
    ? // params is not an empty object, or it is a boolean or a number
      typeof params === 'object' && Object.keys(params).length === 0
      ? null
      : (body.params = params)
    : null
  return body
}
