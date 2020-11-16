# localCookies

A tiny localStorage shim with fallback to Cookies. Will allow apps to use localStorage but fallsback to cookies when that is not available in the browser.

## About localCookies

The localCookies library has the exact same API as localStorage, for more information see [here](https://developer.mozilla.org/en-US/docs/Web/API/Window/localStorage).

localCookies comes in two flavors a iife function that can be directly used in the <script> tag under `./dist` and a ES6 module under `./module`

## Getting started

localCookies can be directly imported from the `dist` or `module` folder. For development:

```Shell
npm install github:WebPlatformForEmbedded/localCookie
// or
yarn add github:WebPlatformForEmbedded/localCookie
```

Next you can `import` the localCookies dependency into your own script and start implementing it from there.

```js
import Storage from './module/localCookie.js',
// or
const Storage = require('./module/localCookie.js')
```

## Build

To build a new dist or module from source execute: `npm run build`

## Running tests

This library has unit / integration tests, located in the `tests` folder.

To run all the tests execute: `npm test`

This will run rollup, start a http-server with mocha/chai and run the tests in a browser.
