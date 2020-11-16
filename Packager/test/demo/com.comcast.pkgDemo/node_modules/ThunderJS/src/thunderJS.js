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

import API from './api'
import plugins from './plugins/index'
import listener from './listener'

let api

export default options => {
  // add extra option with token when thunder.token() is available
  if (
    options.token === undefined &&
    typeof window !== 'undefined' &&
    window.thunder &&
    typeof window.thunder.token === 'function'
  ) {
    options.token = window.thunder.token()
  }

  api = API(options)
  return wrapper({ ...thunder(options), ...plugins })
}

const resolve = (result, args) => {
  // make sure we always have a promise
  if (
    // not an object so definitely not a promise
    typeof result !== 'object' ||
    // an object that doesn't look like a promise
    (typeof result === 'object' && (!result.then || typeof result.then !== 'function'))
  ) {
    result = new Promise((resolve, reject) => {
      result instanceof Error === false ? resolve(result) : reject(result)
    })
  }

  // see if the last argument is a function (and assume it's the callback)
  const cb = typeof args[args.length - 1] === 'function' ? args[args.length - 1] : null
  if (cb) {
    result.then(res => cb(null, res)).catch(err => cb(err))
  } else {
    return result
  }
}

const thunder = options => ({
  options,
  plugin: false,
  call() {
    // little trick to set the plugin name when calling from a plugin context (if not already set)
    const args = [...arguments]
    if (this.plugin) {
      if (args[0] !== this.plugin) {
        args.unshift(this.plugin)
      }
    }

    const plugin = args[0]
    const method = args[1]

    if (typeof this[plugin][method] == 'function') {
      return this[plugin][method](args[2])
    }

    return this.api.request.apply(this, args)
  },
  registerPlugin(name, plugin) {
    this[name] = wrapper(Object.assign(Object.create(thunder), plugin, { plugin: name }))
  },
  subscribe() {
    // subscribe to notification
    // to do
  },
  on() {
    const args = [...arguments]
    // first make sure the plugin is the first argument (independent from being called as argument style or object style)
    // except when listening to a 'special ThunderJS' event
    if (['connect', 'disconnect', 'error'].indexOf(args[0]) !== -1) {
      args.unshift('ThunderJS')
    } else {
      if (this.plugin) {
        if (args[0] !== this.plugin) {
          args.unshift(this.plugin)
        }
      }
    }

    return listener.apply(this, args)
  },
  once() {
    console.log('todo ...')
  },
})

const wrapper = obj => {
  return new Proxy(obj, {
    get(target, propKey) {
      const prop = target[propKey]

      // return the initialized api object, when key is api
      if (propKey === 'api') {
        return api
      }

      if (typeof prop !== 'undefined') {
        if (typeof prop === 'function') {
          // on, once and subscribe don't need to be wrapped in a resolve
          if (['on', 'once', 'subscribe'].indexOf(propKey) > -1) {
            return function(...args) {
              return prop.apply(this, args)
            }
          }
          return function(...args) {
            return resolve(prop.apply(this, args), args)
          }
        }
        if (typeof prop === 'object') {
          return wrapper(
            Object.assign(Object.create(thunder(target.options)), prop, { plugin: propKey })
          )
        }
        return prop
      } else {
        if (target.plugin === false) {
          return wrapper(
            Object.assign(Object.create(thunder(target.options)), {}, { plugin: propKey })
          )
        }
        return function(...args) {
          args.unshift(propKey)
          return target.call.apply(this, args)
        }
      }
    },
  })
}
