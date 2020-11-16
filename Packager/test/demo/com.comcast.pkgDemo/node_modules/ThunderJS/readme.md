# ThunderJS

A flexible and extensible JS library to interact with [Thunder (WPEframework)](https://github.com/WebPlatformForEmbedded/Thunder/)

## About ThunderJS

ThunderJS is an _isomorphic_ library, which means it can be used in a browser environment as well as a NodeJS environment.

ThunderJS makes is easy to make API calls to Thunder (WPEframework) over a Websocket connection. ThunderJS can also be used to listen to (and act upon) notifications broadcasted by Thunder.

## Getting started

ThunderJS can be installed into your project via _NPM_ or _Yarn_, installing directly from GitHub:

```Shell
npm install github:rdkcentral/ThunderJS
// or
yarn add github:rdkcentral/ThunderJS
```

Next you can `import` or `require` the ThunderJS dependency into your own script and start implementing it from there.

```js
import ThunderJS from 'ThunderJS',
// or
const ThunderJS = require('ThunderJS)
```

## Examples

This repository contains examples of how to use and implement this library in a browser environment and in a NodeJS environment.

In order to run the examples you should first:

1. Clone this repository
2. And run `npm install` to install the project's dependencies

### Browser

Run `npm run example:browser` to 1) install the example's dependencies and 2) fire up a local webserver that serves a browser based implementation of ThunderJS

### NodeJS

Run `npm run example:node` to 1) install the example's dependencies and 2) start a NodeJS / CLI implementatio of ThunderJS


## Using the library

Note: all examples use _ES6 syntax_. In order to use this syntax, depending on your target environment, you might need to use a [Babel](https://babeljs.io/docs/en/learn) and some kind of module bundler.

### Initializing the library

```js
import ThunderJS from './thunderJS'

const config = {
  host: '192.168.1.100', // IP address of the box that runs Thunder (WPEframework) - required
}
const thunderJS = ThunderJS(config)
```

If your box is running Thunder under a different *port* than the default port **80**, you can specify this in your configuration object (together with some extra properties of the websocket address).

```js
const config = {
  host: '192.168.1.100', // defaults to localhost,
  port: 2020, // defaults to 80
  endpoint: '/api', // defaults to '/jsonrpc'
  protocol: 'wss://', // defaults to 'ws://'
}
```

It is possible to have multiple instances of ThunderJS, with different configuration.

### Making API calls

In essence all API calls are made up of the following components:

- plugin (i.e. `Controller` or `DeviceInfo` **important**: plugin names are _case sensitive_)
- method (i.e `activate` or `systeminfo`)
- params (i.e. `{callsign: 'Bluetooth'}`, optional)

The library supports 2 ways of making API calls, depending on your coding style preferences.

**Option 1 - Argument based**

```js
const plugin = 'DeviceInfo'
const method = 'systeminfo'
const params = {
  foo: 'bar'
}

thunderJS.call(plugin, method, params)
```

**Option 2 - Object based**

```js
const params = {
  foo: 'bar'
}

thunderJS.DeviceInfo.systeminfo(params)
```

**Versions**

The Thunder API supports different versions of the same methods, with a slightly different implementation depending on the specific box or needs of the project.

By default ThunderJS calls *version 1* of all methods, for each plugin. But during the initialization of ThunderJS, you have the option to configure which version(s) to use (per plugin). The configured version will be called for every method call for that plugin (per thunderJS instance).

Configuring versions is done by passsing in a `versions` object with a key-value pair for each relevant plugin to the `ThunderJS` factory. It is also possible pass in a `default` value to overwrite the standard default value of 1.

```js
import ThunderJS from './thunderJS'

const config = {
  host: '192.168.1.100',
  versions: {
    default: 5, // use version 5 if plugin not specified
    Controller: 1,
    DeviceInfo: 15,
    Messenger: 7,
    // etc ..
  }
}
const thunderJS = ThunderJS(config)
```

In some cases you might need to call a different version for a specific *method*. In this case you can pass `version` as a `param`.

```js
import ThunderJS from './thunderJS'

const config = {
  host: '192.168.1.100',
  versions: {
    default: 5, // use version 5 if plugin not specified
    Controller: 1,
    DeviceInfo: 15,
    Messenger: 7,
    // etc ..
  }
}
const thunderJS = ThunderJS(config)

// use version 15 as specified in the config
thunderJS.DeviceInfo.systeminfo()
// override config and use version 14
thunderJS.DeviceInfo.systeminfo({
  version: 14
})
```

### Processing the result of an API call

When an API call to Thunder is made it can return a `result` in case of success or an `error`, when something goes wrong.

The ThunderJS library supports 2 ways of processing the results of API calls, depending on your coding style preferences.

**Option 1 - Promise based**

```js
thunderJS.DeviceInfo.systeminfo()
  .then(result => {
    console.log('Success', result)
  }).catch(err => {
    console.error('Error', err)
  })
```

**Option 2 - Callback based**

```js
thunderJS.DeviceInfo.systeminfo((err, result) => {
  if(err) {
    console.error('Error', err)
  }
  else {
    console.log('Success!', result)
  }
})
```

> Note that in these examples the _object based_ style of calling was used. But both ways of processing the result of an API call work with _argument based_ style as well.

### Plugin helper methods

Besides calling the available WPE Thunder API methods and returning the result, ThunderJS can also implement extra helper methods.

For example, the WPE Thunder API for the `DeviceInfo` plugin currently consists of only 3 methods (`systeminfo`, `addresses` and `socketinfo`).

On top of that the ThunderJS library implements 2 _convenience methods_ to retrieve the `version` and `freeRam` directly (which ultimately are retrieved from the API by calling the `systeminfo` method).

```js
thunderJS.DeviceInfo
  .freeRam()
  .then(ram => {
    console.log('Free ram', ram)
  })
  .catch(err => {
    console.error('Error', err)
  })
```

### Custom plugins

While it's not necesarry to create a specific plugin for every Thunder plugin (or Nano service) to be able to make API to that plugin, with ThunderJS you can easily implement custom plugins.

You would create a custom plugin only, when you want to enhance the API of the Thunder plugin / Nano service with extra conviniece methods.

A plugin consists of a plain JS object literal, that should be registered under the plugin's namespace.

```js
// register the plugin
thunderJS.registerPlugin('custom', {
  method1(bar) {
    return this.call('foo', { foo: bar })
  },
  method2() {
    return this.call('bar').then(result => {
      return result.bla
    })
  },
})

// call a method on the plugin
thunderJS.custom.method1('bar')
  .then(console.log)
  .catch(console.error)
```

### Notifications

> While partially functional, notifications are work in progress.

Thunder (WPEframework) broadcasts notifications when events ocur in the system. However it will only broadcast those events that the client has subscribed to.

ThunderJS makes it easy to subscribe to specific events, and execute a _callback-function_ upon every notification of each event.

Simply define a listener, passing in the `plugin` as a first argument and the `event` as a second. As a third argument you can pass in the callback function (that receives the `notification` as an argument) every time a notification is received. Optionally a fourth `error` callback can be provided which will be called when the notification failed to register.

```js
const listener = thunderJS.on('Controller', 'statechange', (notification) => {
  console.log('Execute this callback on every notification', notification)
})

// dispose when done listening
listener.dispose()
```

In case you want to listen only once, you can use `once` instead of `on`

> The `once` method is work in progress!

```js
const listener = thunderJS.once('Controller', 'statechange', (notification) => {
  console.log('Execute this callback once', notification)
})

// a once-listener can also be disposed (but you should do so before it's called, of course)
listener.dispose()
```

As with API calls, you can also use *object based* style to achieve the same result.

```js
const listener = thunderJS.Controller.on('statechange', (notification) => {
  console.log('Execute this callback on every notification', notification)
})
```

```js
const listener = thunderJS.Controller.once('statechange', (notification) => {
  console.log('Execute this callback once', notification)
})
```

You can attach multiple listeners to the same notification. The callbacks will then be executed in sequence.

```js
const listener1 = thunderJS.Controller.on('statechange', (notification) => {
  console.log('First callback!', notification)
})

const listener2 = thunderJS.Controller.on('statechange', (notification) => {
  console.log('Second callback!', notification)
})
```

If the event does not exist (or there is another thunder issue) the error callback will be called.

```js
const errorListener = thunderJS.Controller.on('thisdoesnotexist', () => {}, (error) => {
  console.log('This is an error callback', notification)
}))
```
#### ThunderJS connection events

Aside from the Thunder provided event system the same syntax can be used to listen for the `connect`, `disconnect` or `error` events which will be fired if there are state changes on the socket connection between ThunderJS and Thunder.

For example:

```js
thunderJS.on('connect', () => {
  console.log('Connect event!')
})

thunderJS.on('disconnect', () => {
  console.log('Disconnect event!')
})

thunderJS.on('error', () => {
  console.log('Error event!')
})
```

The `connect`, `disconnect` and `error` events are tied to the websocket events. For more information please see the [browser](https://developer.mozilla.org/en-US/docs/Web/API/WebSocket) or [nodejs](https://github.com/websockets/ws/blob/master/doc/ws.md#class-websocket) documentation respectively.

> **Proposal / Work in progress!**

If you want or need more control over listeners - for example because you need multiple listeners and want to keep track of them individually - you could also create a _subscription_ oject.

```js
const subscription = thunderJS.subscribe('Controller') // or thunderJS.Controller.subscribe()

const listener = subscription.on('statechange', (event) => {
  console.log('Execute this callback on every notification', event)
})

const listenerOnce = subscription.once('statechange', (event) => {
  console.log('Execute this callback on time', event)
})

// dispose listener when done listening
listener.dispose()

// return all listeners
const listeners = subscription.listeners()

// dispose all listeners
subscription.disposeAll()

// unsubscribe the subscription to stop acting upon incoming notifications (but keep track of the listeners!)
subscription.unsubscribe()

// subscribe to the notifications again (activating all listeners that weren't disposed of)
subscription.subscribe()

// dispose the subscription (together with all listeners)
subscription.dispose()
```

## Running tests

This library has unit / integration tests, located in the `tests` folder.

To run all the tests execute: `npm test`
