/**
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

const WebSocket = require('ws');

let ws = null;
if (typeof WebSocket !== 'undefined') {
  ws = WebSocket;
}
var ws_1 = ws;

const requestsQueue = {};
const listeners = {};

var requestQueueResolver = data => {
  if (typeof data === 'string') {
    data = JSON.parse(data.normalize().replace(/\\x([0-9A-Fa-f]{2})/g, ''));
  }
  if (data.id) {
    const request = requestsQueue[data.id];
    if (request) {
      if ('result' in data) request.resolve(data.result);
      else request.reject(data.error);
      delete requestsQueue[data.id];
    } else {
      console.log('no pending request found with id ' + data.id);
    }
  }
};

var notificationListener = data => {
  if (typeof data === 'string') {
    data = JSON.parse(data.normalize().replace(/\\x([0-9A-Fa-f]{2})/g, ''));
  }
  if (!data.id && data.method) {
    const callbacks = listeners[data.method];
    if (callbacks && Array.isArray(callbacks) && callbacks.length) {
      callbacks.forEach(callback => {
        callback(data.params);
      });
    }
  }
};

const protocol = 'ws://';
const host = 'localhost';
const endpoint = '/jsonrpc';
const port = 80;
var makeWebsocketAddress = options => {
  return [
    (options && options.protocol) || protocol,
    (options && options.host) || host,
    ':' + ((options && options.port) || port),
    (options && options.endpoint) || endpoint,
    options && options.token ? '?token=' + options.token : null,
  ].join('')
};

const protocols = 'notification';
let socket = null;
var connect = options => {
  return new Promise((resolve, reject) => {
    if (socket && socket.readyState === 1) return resolve(socket)
    if (socket && socket.readyState === 0) {
      const waitForOpen = () => {
        socket.removeEventListener('open', waitForOpen);
        resolve(socket);
      };
      return socket.addEventListener('open', waitForOpen)
    }
    if (socket === null) {
      socket = new ws_1(makeWebsocketAddress(options), protocols);
      socket.addEventListener('message', message => {
        if (options.debug) {
          console.log(' ');
          console.log('API REPONSE:');
          console.log(JSON.stringify(message.data, null, 2));
          console.log(' ');
        }
        requestQueueResolver(message.data);
      });
      socket.addEventListener('message', message => {
        notificationListener(message.data);
      });
      socket.addEventListener('error', () => {
        notificationListener({
          method: 'client.ThunderJS.events.error',
        });
        socket = null;
      });
      const handleConnectClosure = event => {
        socket = null;
        reject(event);
      };
      socket.addEventListener('close', handleConnectClosure);
      socket.addEventListener('open', () => {
        notificationListener({
          method: 'client.ThunderJS.events.connect',
        });
        socket.removeEventListener('close', handleConnectClosure);
        socket.addEventListener('close', () => {
          notificationListener({
            method: 'client.ThunderJS.events.disconnect',
          });
          socket = null;
        });
        resolve(socket);
      });
    } else {
      socket = null;
      reject('Socket error');
    }
  })
};

var makeBody = (requestId, plugin, method, params, version) => {
  params ? delete params.version : null;
  const body = {
    jsonrpc: '2.0',
    id: requestId,
    method: [plugin, version, method].join('.'),
  };
  params || params === false
    ?
      typeof params === 'object' && Object.keys(params).length === 0
      ? null
      : (body.params = params)
    : null;
  return body
};

var getVersion = (versionsConfig, plugin, params) => {
  const defaultVersion = 1;
  let version;
  if ((version = params && params.version)) {
    return version
  }
  return versionsConfig
    ? versionsConfig[plugin] || versionsConfig.default || defaultVersion
    : defaultVersion
};

let id = 0;
var makeId = () => {
  id = id + 1;
  return id
};

var execRequest = (options, body) => {
  return connect(options).then(connection => {
    connection.send(JSON.stringify(body));
  })
};

var API = options => {
  return {
    request(plugin, method, params) {
      return new Promise((resolve, reject) => {
        const requestId = makeId();
        const version = getVersion(options.versions, plugin, params);
        const body = makeBody(requestId, plugin, method, params, version);
        if (options.debug) {
          console.log(' ');
          console.log('API REQUEST:');
          console.log(JSON.stringify(body, null, 2));
          console.log(' ');
        }
        requestsQueue[requestId] = {
          body,
          resolve,
          reject,
        };
        execRequest(options, body).catch(e => {
          reject(e);
        });
      })
    },
  }
};

var DeviceInfo = {
  freeRam(params) {
    return this.call('systeminfo', params).then(res => {
      return res.freeram
    })
  },
  version(params) {
    return this.call('systeminfo', params).then(res => {
      return res.version
    })
  },
};

var plugins = {
  DeviceInfo,
};

function listener(plugin, event, callback, errorCallback) {
  const thunder = this;
  const index = register.call(this, plugin, event, callback, errorCallback);
  return {
    dispose() {
      const listener_id = makeListenerId(plugin, event);
      if (listeners[listener_id] === undefined) return
      listeners[listener_id].splice(index, 1);
      if (listeners[listener_id].length === 0) {
        unregister.call(thunder, plugin, event, errorCallback);
      }
    },
  }
}
const makeListenerId = (plugin, event) => {
  return ['client', plugin, 'events', event].join('.')
};
const register = function(plugin, event, callback, errorCallback) {
  const listener_id = makeListenerId(plugin, event);
  if (!listeners[listener_id]) {
    listeners[listener_id] = [];
    if (plugin !== 'ThunderJS') {
      const method = 'register';
      const request_id = listener_id
        .split('.')
        .slice(0, -1)
        .join('.');
      const params = {
        event,
        id: request_id,
      };
      this.api.request(plugin, method, params).catch(e => {
        if (typeof errorCallback === 'function') errorCallback(e.message);
      });
    }
  }
  listeners[listener_id].push(callback);
  return listeners[listener_id].length - 1
};
const unregister = function(plugin, event, errorCallback) {
  const listener_id = makeListenerId(plugin, event);
  delete listeners[listener_id];
  if (plugin !== 'ThunderJS') {
    const method = 'unregister';
    const request_id = listener_id
      .split('.')
      .slice(0, -1)
      .join('.');
    const params = {
      event,
      id: request_id,
    };
    this.api.request(plugin, method, params).catch(e => {
      if (typeof errorCallback === 'function') errorCallback(e.message);
    });
  }
};

let api;
var thunderJS = options => {
  if (
    options.token === undefined &&
    typeof window !== 'undefined' &&
    window.thunder &&
    typeof window.thunder.token === 'function'
  ) {
    options.token = window.thunder.token();
  }
  api = API(options);
  return wrapper({ ...thunder(options), ...plugins })
};
const resolve = (result, args) => {
  if (
    typeof result !== 'object' ||
    (typeof result === 'object' && (!result.then || typeof result.then !== 'function'))
  ) {
    result = new Promise((resolve, reject) => {
      result instanceof Error === false ? resolve(result) : reject(result);
    });
  }
  const cb = typeof args[args.length - 1] === 'function' ? args[args.length - 1] : null;
  if (cb) {
    result.then(res => cb(null, res)).catch(err => cb(err));
  } else {
    return result
  }
};
const thunder = options => ({
  options,
  plugin: false,
  call() {
    const args = [...arguments];
    if (this.plugin) {
      if (args[0] !== this.plugin) {
        args.unshift(this.plugin);
      }
    }
    const plugin = args[0];
    const method = args[1];
    if (typeof this[plugin][method] == 'function') {
      return this[plugin][method](args[2])
    }
    return this.api.request.apply(this, args)
  },
  registerPlugin(name, plugin) {
    this[name] = wrapper(Object.assign(Object.create(thunder), plugin, { plugin: name }));
  },
  subscribe() {
  },
  on() {
    const args = [...arguments];
    if (['connect', 'disconnect', 'error'].indexOf(args[0]) !== -1) {
      args.unshift('ThunderJS');
    } else {
      if (this.plugin) {
        if (args[0] !== this.plugin) {
          args.unshift(this.plugin);
        }
      }
    }
    return listener.apply(this, args)
  },
  once() {
    console.log('todo ...');
  },
});
const wrapper = obj => {
  return new Proxy(obj, {
    get(target, propKey) {
      const prop = target[propKey];
      if (propKey === 'api') {
        return api
      }
      if (typeof prop !== 'undefined') {
        if (typeof prop === 'function') {
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
          args.unshift(propKey);
          return target.call.apply(this, args)
        }
      }
    },
  })
};

export default thunderJS;
