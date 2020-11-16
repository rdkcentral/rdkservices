/**
 * App version: 1.0.0
 * SDK version: 2.6.0
 * CLI version: 1.7.4
 *
 * Generated: Thu, 08 Oct 2020 15:10:00 GMT
 */

var APP_com_comcast_pkgDemo = (function () {
	'use strict';

	var isMergeableObject = function isMergeableObject(value) {
		return isNonNullObject(value)
			&& !isSpecial(value)
	};

	function isNonNullObject(value) {
		return !!value && typeof value === 'object'
	}

	function isSpecial(value) {
		var stringValue = Object.prototype.toString.call(value);

		return stringValue === '[object RegExp]'
			|| stringValue === '[object Date]'
			|| isReactElement(value)
	}

	// see https://github.com/facebook/react/blob/b5ac963fb791d1298e7f396236383bc955f916c1/src/isomorphic/classic/element/ReactElement.js#L21-L25
	var canUseSymbol = typeof Symbol === 'function' && Symbol.for;
	var REACT_ELEMENT_TYPE = canUseSymbol ? Symbol.for('react.element') : 0xeac7;

	function isReactElement(value) {
		return value.$$typeof === REACT_ELEMENT_TYPE
	}

	function emptyTarget(val) {
		return Array.isArray(val) ? [] : {}
	}

	function cloneUnlessOtherwiseSpecified(value, options) {
		return (options.clone !== false && options.isMergeableObject(value))
			? deepmerge(emptyTarget(value), value, options)
			: value
	}

	function defaultArrayMerge(target, source, options) {
		return target.concat(source).map(function(element) {
			return cloneUnlessOtherwiseSpecified(element, options)
		})
	}

	function getMergeFunction(key, options) {
		if (!options.customMerge) {
			return deepmerge
		}
		var customMerge = options.customMerge(key);
		return typeof customMerge === 'function' ? customMerge : deepmerge
	}

	function getEnumerableOwnPropertySymbols(target) {
		return Object.getOwnPropertySymbols
			? Object.getOwnPropertySymbols(target).filter(function(symbol) {
				return target.propertyIsEnumerable(symbol)
			})
			: []
	}

	function getKeys(target) {
		return Object.keys(target).concat(getEnumerableOwnPropertySymbols(target))
	}

	function propertyIsOnObject(object, property) {
		try {
			return property in object
		} catch(_) {
			return false
		}
	}

	// Protects from prototype poisoning and unexpected merging up the prototype chain.
	function propertyIsUnsafe(target, key) {
		return propertyIsOnObject(target, key) // Properties are safe to merge if they don't exist in the target yet,
			&& !(Object.hasOwnProperty.call(target, key) // unsafe if they exist up the prototype chain,
				&& Object.propertyIsEnumerable.call(target, key)) // and also unsafe if they're nonenumerable.
	}

	function mergeObject(target, source, options) {
		var destination = {};
		if (options.isMergeableObject(target)) {
			getKeys(target).forEach(function(key) {
				destination[key] = cloneUnlessOtherwiseSpecified(target[key], options);
			});
		}
		getKeys(source).forEach(function(key) {
			if (propertyIsUnsafe(target, key)) {
				return
			}

			if (propertyIsOnObject(target, key) && options.isMergeableObject(source[key])) {
				destination[key] = getMergeFunction(key, options)(target[key], source[key], options);
			} else {
				destination[key] = cloneUnlessOtherwiseSpecified(source[key], options);
			}
		});
		return destination
	}

	function deepmerge(target, source, options) {
		options = options || {};
		options.arrayMerge = options.arrayMerge || defaultArrayMerge;
		options.isMergeableObject = options.isMergeableObject || isMergeableObject;
		// cloneUnlessOtherwiseSpecified is added to `options` so that custom arrayMerge()
		// implementations can use it. The caller may not replace it.
		options.cloneUnlessOtherwiseSpecified = cloneUnlessOtherwiseSpecified;

		var sourceIsArray = Array.isArray(source);
		var targetIsArray = Array.isArray(target);
		var sourceAndTargetTypesMatch = sourceIsArray === targetIsArray;

		if (!sourceAndTargetTypesMatch) {
			return cloneUnlessOtherwiseSpecified(source, options)
		} else if (sourceIsArray) {
			return options.arrayMerge(target, source, options)
		} else {
			return mergeObject(target, source, options)
		}
	}

	deepmerge.all = function deepmergeAll(array, options) {
		if (!Array.isArray(array)) {
			throw new Error('first argument should be an array')
		}

		return array.reduce(function(prev, next) {
			return deepmerge(prev, next, options)
		}, {})
	};

	var deepmerge_1 = deepmerge;

	var cjs = deepmerge_1;

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

	var Lightning = window.lng;

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

	const settings = {};
	const subscribers = {};

	const initSettings = (appSettings, platformSettings) => {
	  settings['app'] = appSettings;
	  settings['platform'] = platformSettings;
	  settings['user'] = {};
	};

	const publish = (key, value) => {
	  subscribers[key] && subscribers[key].forEach(subscriber => subscriber(value));
	};

	const dotGrab = (obj = {}, key) => {
	  const keys = key.split('.');
	  for (let i = 0; i < keys.length; i++) {
	    obj = obj[keys[i]] = obj[keys[i]] !== undefined ? obj[keys[i]] : {};
	  }
	  return typeof obj === 'object' ? (Object.keys(obj).length ? obj : undefined) : obj
	};

	var Settings = {
	  get(type, key, fallback = undefined) {
	    const val = dotGrab(settings[type], key);
	    return val !== undefined ? val : fallback
	  },
	  has(type, key) {
	    return !!this.get(type, key)
	  },
	  set(key, value) {
	    settings['user'][key] = value;
	    publish(key, value);
	  },
	  subscribe(key, callback) {
	    subscribers[key] = subscribers[key] || [];
	    subscribers[key].push(callback);
	  },
	  unsubscribe(key, callback) {
	    if (callback) {
	      const index = subscribers[key] && subscribers[key].findIndex(cb => cb === callback);
	      index > -1 && subscribers[key].splice(index, 1);
	    } else {
	      if (key in subscribers) {
	        subscribers[key] = [];
	      }
	    }
	  },
	  clearSubscribers() {
	    for (const key of Object.getOwnPropertyNames(subscribers)) {
	      delete subscribers[key];
	    }
	  },
	};

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

	const prepLog = (type, args) => {
	  const colors = {
	    Info: 'green',
	    Debug: 'gray',
	    Warn: 'orange',
	    Error: 'red',
	  };

	  args = Array.from(args);
	  return [
	    '%c' + (args.length > 1 && typeof args[0] === 'string' ? args.shift() : type),
	    'background-color: ' + colors[type] + '; color: white; padding: 2px 4px; border-radius: 2px',
	    args,
	  ]
	};

	var Log = {
	  info() {
	    Settings.get('platform', 'log') && console.log.apply(console, prepLog('Info', arguments));
	  },
	  debug() {
	    Settings.get('platform', 'log') && console.debug.apply(console, prepLog('Debug', arguments));
	  },
	  error() {
	    Settings.get('platform', 'log') && console.error.apply(console, prepLog('Error', arguments));
	  },
	  warn() {
	    Settings.get('platform', 'log') && console.warn.apply(console, prepLog('Warn', arguments));
	  },
	};

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
	class Locale {
	  constructor() {
	    this.__enabled = false;
	  }

	  /**
	   * Loads translation object from external json file.
	   *
	   * @param {String} path Path to resource.
	   * @return {Promise}
	   */
	  async load(path) {
	    if (!this.__enabled) {
	      return
	    }

	    await fetch(path)
	      .then(resp => resp.json())
	      .then(resp => {
	        this.loadFromObject(resp);
	      });
	  }

	  /**
	   * Sets language used by module.
	   *
	   * @param {String} lang
	   */
	  setLanguage(lang) {
	    this.__enabled = true;
	    this.language = lang;
	  }

	  /**
	   * Returns reference to translation object for current language.
	   *
	   * @return {Object}
	   */
	  get tr() {
	    return this.__trObj[this.language]
	  }

	  /**
	   * Loads translation object from existing object (binds existing object).
	   *
	   * @param {Object} trObj
	   */
	  loadFromObject(trObj) {
	    const fallbackLanguage = 'en';
	    if (Object.keys(trObj).indexOf(this.language) === -1) {
	      Log.warn('No translations found for: ' + this.language);
	      if (Object.keys(trObj).indexOf(fallbackLanguage) > -1) {
	        Log.warn('Using fallback language: ' + fallbackLanguage);
	        this.language = fallbackLanguage;
	      } else {
	        const error = 'No translations found for fallback language: ' + fallbackLanguage;
	        Log.error(error);
	        throw Error(error)
	      }
	    }

	    this.__trObj = trObj;
	    for (const lang of Object.values(this.__trObj)) {
	      for (const str of Object.keys(lang)) {
	        lang[str] = new LocalizedString(lang[str]);
	      }
	    }
	  }
	}

	/**
	 * Extended string class used for localization.
	 */
	class LocalizedString extends String {
	  /**
	   * Returns formatted LocalizedString.
	   * Replaces each placeholder value (e.g. {0}, {1}) with corresponding argument.
	   *
	   * E.g.:
	   * > new LocalizedString('{0} and {1} and {0}').format('A', 'B');
	   * A and B and A
	   *
	   * @param  {...any} args List of arguments for placeholders.
	   */
	  format(...args) {
	    const sub = args.reduce((string, arg, index) => string.split(`{${index}}`).join(arg), this);
	    return new LocalizedString(sub)
	  }
	}

	var Locale$1 = new Locale();

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

	let sendMetric = (type, event, params) => {
	  Log.info('Sending metric', type, event, params);
	};

	const initMetrics = config => {
	  sendMetric = config.sendMetric;
	};

	// available metric per category
	const metrics = {
	  app: ['launch', 'loaded', 'ready', 'close'],
	  page: ['view', 'leave'],
	  user: ['click', 'input'],
	  media: [
	    'abort',
	    'canplay',
	    'ended',
	    'pause',
	    'play',
	    'suspend',
	    'volumechange',
	    'waiting',
	    'seeking',
	    'seeked',
	  ],
	};

	// error metric function (added to each category)
	const errorMetric = (type, message, code, visible, params = {}) => {
	  params = { params, ...{ message, code, visible } };
	  sendMetric(type, 'error', params);
	};

	const Metric = (type, events, options = {}) => {
	  return events.reduce(
	    (obj, event) => {
	      obj[event] = (name, params = {}) => {
	        params = { ...options, ...(name ? { name } : {}), ...params };
	        sendMetric(type, event, params);
	      };
	      return obj
	    },
	    {
	      error(message, code, params) {
	        errorMetric(type, message, code, params);
	      },
	      event(name, params) {
	        sendMetric(type, name, params);
	      },
	    }
	  )
	};

	const Metrics = types => {
	  return Object.keys(types).reduce(
	    (obj, type) => {
	      // media metric works a bit different!
	      // it's a function that accepts a url and returns an object with the available metrics
	      // url is automatically passed as a param in every metric
	      type === 'media'
	        ? (obj[type] = url => Metric(type, types[type], { url }))
	        : (obj[type] = Metric(type, types[type]));
	      return obj
	    },
	    { error: errorMetric, event: sendMetric }
	  )
	};

	var Metrics$1 = Metrics(metrics);

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

	class VersionLabel extends Lightning.Component {
	  static _template() {
	    return {
	      rect: true,
	      color: 0xbb0078ac,
	      h: 40,
	      w: 100,
	      x: w => w - 50,
	      y: h => h - 50,
	      mount: 1,
	      Text: {
	        w: w => w,
	        h: h => h,
	        y: 5,
	        x: 20,
	        text: {
	          fontSize: 22,
	          lineHeight: 26,
	        },
	      },
	    }
	  }

	  _firstActive() {
	    this.tag('Text').text = `APP - v${this.version}\nSDK - v${this.sdkVersion}`;
	    this.tag('Text').loadTexture();
	    this.w = this.tag('Text').renderWidth + 40;
	    this.h = this.tag('Text').renderHeight + 5;
	  }
	}

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
	class FpsIndicator extends Lightning.Component {
	  static _template() {
	    return {
	      rect: true,
	      color: 0xffffffff,
	      texture: Lightning.Tools.getRoundRect(80, 80, 40),
	      h: 80,
	      w: 80,
	      x: 100,
	      y: 100,
	      mount: 1,
	      Background: {
	        x: 3,
	        y: 3,
	        texture: Lightning.Tools.getRoundRect(72, 72, 36),
	        color: 0xff008000,
	      },
	      Counter: {
	        w: w => w,
	        h: h => h,
	        y: 10,
	        text: {
	          fontSize: 32,
	          textAlign: 'center',
	        },
	      },
	      Text: {
	        w: w => w,
	        h: h => h,
	        y: 48,
	        text: {
	          fontSize: 15,
	          textAlign: 'center',
	          text: 'FPS',
	        },
	      },
	    }
	  }

	  _setup() {
	    this.config = {
	      ...{
	        log: false,
	        interval: 500,
	        threshold: 1,
	      },
	      ...Settings.get('platform', 'showFps'),
	    };

	    this.fps = 0;
	    this.lastFps = this.fps - this.config.threshold;

	    const fpsCalculator = () => {
	      this.fps = ~~(1 / this.stage.dt);
	    };
	    this.stage.on('frameStart', fpsCalculator);
	    this.stage.off('framestart', fpsCalculator);
	    this.interval = setInterval(this.showFps.bind(this), this.config.interval);
	  }

	  _firstActive() {
	    this.showFps();
	  }

	  _detach() {
	    clearInterval(this.interval);
	  }

	  showFps() {
	    if (Math.abs(this.lastFps - this.fps) <= this.config.threshold) return
	    this.lastFps = this.fps;
	    // green
	    let bgColor = 0xff008000;
	    // orange
	    if (this.fps <= 40 && this.fps > 20) bgColor = 0xffffa500;
	    // red
	    else if (this.fps <= 20) bgColor = 0xffff0000;

	    this.tag('Background').setSmooth('color', bgColor);
	    this.tag('Counter').text = `${this.fps}`;

	    this.config.log && Log.info('FPS', this.fps);
	  }
	}

	var version = "2.6.0";

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

	let AppInstance;

	const defaultOptions = {
	  stage: { w: 1920, h: 1080, clearColor: 0x00000000, canvas2d: false },
	  debug: false,
	  defaultFontFace: 'RobotoRegular',
	  keys: {
	    8: 'Back',
	    13: 'Enter',
	    27: 'Menu',
	    37: 'Left',
	    38: 'Up',
	    39: 'Right',
	    40: 'Down',
	    174: 'ChannelDown',
	    175: 'ChannelUp',
	    178: 'Stop',
	    250: 'PlayPause',
	    191: 'Search', // Use "/" for keyboard
	    409: 'Search',
	  },
	};

	if (window.innerHeight === 720) {
	  defaultOptions.stage['w'] = 1280;
	  defaultOptions.stage['h'] = 720;
	  defaultOptions.stage['precision'] = 0.6666666667;
	}

	function Application(App, appData, platformSettings) {
	  return class Application extends Lightning.Application {
	    constructor(options) {
	      const config = cjs(defaultOptions, options);
	      super(config);
	      this.config = config;
	    }

	    static _template() {
	      return {
	        w: 1920,
	        h: 1080,
	        rect: true,
	        color: 0x00000000,
	      }
	    }

	    _setup() {
	      Promise.all([
	        this.loadFonts((App.config && App.config.fonts) || (App.getFonts && App.getFonts()) || []),
	        Locale$1.load((App.config && App.config.locale) || (App.getLocale && App.getLocale())),
	      ])
	        .then(() => {
	          Metrics$1.app.loaded();

	          AppInstance = this.stage.c({
	            ref: 'App',
	            type: App,
	            forceZIndexContext: !!platformSettings.showVersion || !!platformSettings.showFps,
	          });

	          this.childList.a(AppInstance);

	          Log.info('App version', this.config.version);
	          Log.info('SDK version', version);

	          if (platformSettings.showVersion) {
	            this.childList.a({
	              ref: 'VersionLabel',
	              type: VersionLabel,
	              version: this.config.version,
	              sdkVersion: version,
	            });
	          }

	          if (platformSettings.showFps) {
	            this.childList.a({
	              ref: 'FpsCounter',
	              type: FpsIndicator,
	            });
	          }

	          super._setup();
	        })
	        .catch(console.error);
	    }

	    _handleBack() {
	      this.closeApp();
	    }

	    _handleExit() {
	      this.closeApp();
	    }

	    closeApp() {
	      Log.info('Closing App');

	      Settings.clearSubscribers();

	      if (platformSettings.onClose && typeof platformSettings.onClose === 'function') {
	        platformSettings.onClose();
	      } else {
	        this.close();
	      }
	    }

	    close() {
	      Log.info('Closing App');
	      this.childList.remove(this.tag('App'));

	      // force texture garbage collect
	      this.stage.gc();
	      this.destroy();
	    }

	    loadFonts(fonts) {
	      return new Promise((resolve, reject) => {
	        fonts
	          .map(({ family, url, descriptors }) => () => {
	            const fontFace = new FontFace(family, 'url(' + url + ')', descriptors || {});
	            document.fonts.add(fontFace);
	            return fontFace.load()
	          })
	          .reduce((promise, method) => {
	            return promise.then(() => method())
	          }, Promise.resolve(null))
	          .then(resolve)
	          .catch(reject);
	      })
	    }

	    set focus(v) {
	      this._focussed = v;
	      this._refocus();
	    }

	    _getFocused() {
	      return this._focussed || this.tag('App')
	    }
	  }
	}

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

	let basePath;
	let proxyUrl;

	const initUtils = config => {
	  basePath = ensureUrlWithProtocol(makeFullStaticPath(window.location.pathname, config.path || '/'));

	  if (config.proxyUrl) {
	    proxyUrl = ensureUrlWithProtocol(config.proxyUrl);
	  }
	};

	var Utils = {
	  asset(relPath) {
	    return basePath + relPath
	  },
	  proxyUrl(url, options = {}) {
	    return proxyUrl ? proxyUrl + '?' + makeQueryString(url, options) : url
	  },
	  makeQueryString() {
	    return makeQueryString(...arguments)
	  },
	  // since imageworkers don't work without protocol
	  ensureUrlWithProtocol() {
	    return ensureUrlWithProtocol(...arguments)
	  },
	};

	const ensureUrlWithProtocol = url => {
	  if (/^\/\//.test(url)) {
	    return window.location.protocol + url
	  }
	  if (!/^(?:https?:)/i.test(url)) {
	    return window.location.origin + url
	  }
	  return url
	};

	const makeFullStaticPath = (pathname = '/', path) => {
	  // ensure path has traling slash
	  path = path.charAt(path.length - 1) !== '/' ? path + '/' : path;

	  // if path is URL, we assume it's already the full static path, so we just return it
	  if (/^(?:https?:)?(?:\/\/)/.test(path)) {
	    return path
	  }

	  if (path.charAt(0) === '/') {
	    return path
	  } else {
	    // cleanup the pathname (i.e. remove possible index.html)
	    pathname = cleanUpPathName(pathname);

	    // remove possible leading dot from path
	    path = path.charAt(0) === '.' ? path.substr(1) : path;
	    // ensure path has leading slash
	    path = path.charAt(0) !== '/' ? '/' + path : path;
	    return pathname + path
	  }
	};

	const cleanUpPathName = pathname => {
	  if (pathname.slice(-1) === '/') return pathname.slice(0, -1)
	  const parts = pathname.split('/');
	  if (parts[parts.length - 1].indexOf('.') > -1) parts.pop();
	  return parts.join('/')
	};

	const makeQueryString = (url, options = {}, type = 'url') => {
	  // add operator as an option
	  options.operator = 'metrological'; // Todo: make this configurable (via url?)
	  // add type (= url or qr) as an option, with url as the value
	  options[type] = url;

	  return Object.keys(options)
	    .map(key => {
	      return encodeURIComponent(key) + '=' + encodeURIComponent('' + options[key])
	    })
	    .join('&')
	};

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

	class ScaledImageTexture extends Lightning.textures.ImageTexture {
	  constructor(stage) {
	    super(stage);
	    this._scalingOptions = undefined;
	  }

	  set options(options) {
	    this.resizeMode = this._scalingOptions = options;
	  }

	  _getLookupId() {
	    return `${this._src}-${this._scalingOptions.type}-${this._scalingOptions.w}-${this._scalingOptions.h}`
	  }

	  getNonDefaults() {
	    const obj = super.getNonDefaults();
	    if (this._src) {
	      obj.src = this._src;
	    }
	    return obj
	  }
	}

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

	const formatLocale = locale => {
	  if (locale && locale.length === 2) {
	    return `${locale.toLowerCase()}-${locale.toUpperCase()}`
	  } else {
	    return locale
	  }
	};

	const getLocale = defaultValue => {
	  if ('language' in navigator) {
	    const locale = formatLocale(navigator.language);
	    return Promise.resolve(locale)
	  } else {
	    return Promise.resolve(defaultValue)
	  }
	};

	const getLanguage = defaultValue => {
	  if ('language' in navigator) {
	    const language = formatLocale(navigator.language).slice(0, 2);
	    return Promise.resolve(language)
	  } else {
	    return Promise.resolve(defaultValue)
	  }
	};

	const getCountryCode = defaultValue => {
	  if ('language' in navigator) {
	    const countryCode = formatLocale(navigator.language).slice(3, 5);
	    return Promise.resolve(countryCode)
	  } else {
	    return Promise.resolve(defaultValue)
	  }
	};

	const hasOrAskForGeoLocationPermission = () => {
	  return new Promise(resolve => {
	    // force to prompt for location permission
	    if (Settings.get('platform', 'forceBrowserGeolocation') === true) resolve(true);
	    if ('permissions' in navigator && typeof navigator.permissions.query === 'function') {
	      navigator.permissions.query({ name: 'geolocation' }).then(status => {
	        resolve(status.state === 'granted' || status.status === 'granted');
	      });
	    } else {
	      resolve(false);
	    }
	  })
	};

	const getLatLon = defaultValue => {
	  return new Promise(resolve => {
	    hasOrAskForGeoLocationPermission().then(granted => {
	      if (granted === true) {
	        if ('geolocation' in navigator) {
	          navigator.geolocation.getCurrentPosition(
	            // success
	            result =>
	              result && result.coords && resolve([result.coords.latitude, result.coords.longitude]),
	            // error
	            () => resolve(defaultValue),
	            // options
	            {
	              enableHighAccuracy: true,
	              timeout: 5000,
	              maximumAge: 0,
	            }
	          );
	        } else {
	          return queryForLatLon().then(result => resolve(result || defaultValue))
	        }
	      } else {
	        return queryForLatLon().then(result => resolve(result || defaultValue))
	      }
	    });
	  })
	};

	const queryForLatLon = () => {
	  return new Promise(resolve => {
	    fetch('https://geolocation-db.com/json/')
	      .then(response => response.json())
	      .then(({ latitude, longitude }) =>
	        latitude && longitude ? resolve([latitude, longitude]) : resolve(false)
	      )
	      .catch(() => resolve(false));
	  })
	};

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

	const defaultProfile = {
	  ageRating: 'adult',
	  city: 'New York',
	  zipCode: '27505',
	  countryCode: () => getCountryCode('US'),
	  ip: '127.0.0.1',
	  household: 'b2244e9d4c04826ccd5a7b2c2a50e7d4',
	  language: () => getLanguage('en'),
	  latlon: () => getLatLon([40.7128, 74.006]),
	  locale: () => getLocale('en-US'),
	  mac: '00:00:00:00:00:00',
	  operator: 'Metrological',
	  platform: 'Metrological',
	  packages: [],
	  uid: 'ee6723b8-7ab3-462c-8d93-dbf61227998e',
	  stbType: 'Metrological',
	};

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

	let getInfo = key => {
	  const profile = { ...defaultProfile, ...Settings.get('platform', 'profile') };
	  return Promise.resolve(typeof profile[key] === 'function' ? profile[key]() : profile[key])
	};

	let setInfo = (key, params) => {
	  if (key in defaultProfile) defaultProfile[key] = params;
	};

	const initProfile = config => {
	  getInfo = config.getInfo;
	  setInfo = config.setInfo;
	};

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

	const events = [
	  'timeupdate',
	  'error',
	  'ended',
	  'loadeddata',
	  'canplay',
	  'play',
	  'playing',
	  'pause',
	  'loadstart',
	  'seeking',
	  'seeked',
	  'encrypted',
	];

	let mediaUrl = url => url;

	const initMediaPlayer = config => {
	  if (config.mediaUrl) {
	    mediaUrl = config.mediaUrl;
	  }
	};

	class Mediaplayer extends Lightning.Component {
	  _construct() {
	    this._skipRenderToTexture = false;
	    this._metrics = null;
	    this._textureMode = Settings.get('platform', 'textureMode') || false;
	    Log.info('Texture mode: ' + this._textureMode);
	  }

	  static _template() {
	    return {
	      Video: {
	        VideoWrap: {
	          VideoTexture: {
	            visible: false,
	            pivot: 0.5,
	            texture: { type: Lightning.textures.StaticTexture, options: {} },
	          },
	        },
	      },
	    }
	  }

	  set skipRenderToTexture(v) {
	    this._skipRenderToTexture = v;
	  }

	  get textureMode() {
	    return this._textureMode
	  }

	  get videoView() {
	    return this.tag('Video')
	  }

	  _init() {
	    //re-use videotag if already there
	    const videoEls = document.getElementsByTagName('video');
	    if (videoEls && videoEls.length > 0) this.videoEl = videoEls[0];
	    else {
	      this.videoEl = document.createElement('video');
	      this.videoEl.setAttribute('id', 'video-player');
	      this.videoEl.style.position = 'absolute';
	      this.videoEl.style.zIndex = '1';
	      this.videoEl.style.display = 'none';
	      this.videoEl.setAttribute('width', '100%');
	      this.videoEl.setAttribute('height', '100%');

	      this.videoEl.style.visibility = this.textureMode ? 'hidden' : 'visible';
	      document.body.appendChild(this.videoEl);
	    }
	    if (this.textureMode && !this._skipRenderToTexture) {
	      this._createVideoTexture();
	    }

	    this.eventHandlers = [];
	  }

	  _registerListeners() {
	    events.forEach(event => {
	      const handler = e => {
	        if (this._metrics && this._metrics[event] && typeof this._metrics[event] === 'function') {
	          this._metrics[event]({ currentTime: this.videoEl.currentTime });
	        }
	        this.fire(event, { videoElement: this.videoEl, event: e });
	      };
	      this.eventHandlers.push(handler);
	      this.videoEl.addEventListener(event, handler);
	    });
	  }

	  _deregisterListeners() {
	    Log.info('Deregistering event listeners MediaPlayer');
	    events.forEach((event, index) => {
	      this.videoEl.removeEventListener(event, this.eventHandlers[index]);
	    });
	    this.eventHandlers = [];
	  }

	  _attach() {
	    this._registerListeners();
	  }

	  _detach() {
	    this._deregisterListeners();
	    this.close();
	  }

	  _createVideoTexture() {
	    const stage = this.stage;

	    const gl = stage.gl;
	    const glTexture = gl.createTexture();
	    gl.bindTexture(gl.TEXTURE_2D, glTexture);
	    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
	    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

	    this.videoTexture.options = { source: glTexture, w: this.videoEl.width, h: this.videoEl.height };
	  }

	  _startUpdatingVideoTexture() {
	    if (this.textureMode && !this._skipRenderToTexture) {
	      const stage = this.stage;
	      if (!this._updateVideoTexture) {
	        this._updateVideoTexture = () => {
	          if (this.videoTexture.options.source && this.videoEl.videoWidth && this.active) {
	            const gl = stage.gl;

	            const currentTime = new Date().getTime();

	            // When BR2_PACKAGE_GST1_PLUGINS_BAD_PLUGIN_DEBUGUTILS is not set in WPE, webkitDecodedFrameCount will not be available.
	            // We'll fallback to fixed 30fps in this case.
	            const frameCount = this.videoEl.webkitDecodedFrameCount;

	            const mustUpdate = frameCount
	              ? this._lastFrame !== frameCount
	              : this._lastTime < currentTime - 30;

	            if (mustUpdate) {
	              this._lastTime = currentTime;
	              this._lastFrame = frameCount;
	              try {
	                gl.bindTexture(gl.TEXTURE_2D, this.videoTexture.options.source);
	                gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
	                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.videoEl);
	                this._lastFrame = this.videoEl.webkitDecodedFrameCount;
	                this.videoTextureView.visible = true;

	                this.videoTexture.options.w = this.videoEl.videoWidth;
	                this.videoTexture.options.h = this.videoEl.videoHeight;
	                const expectedAspectRatio = this.videoTextureView.w / this.videoTextureView.h;
	                const realAspectRatio = this.videoEl.videoWidth / this.videoEl.videoHeight;
	                if (expectedAspectRatio > realAspectRatio) {
	                  this.videoTextureView.scaleX = realAspectRatio / expectedAspectRatio;
	                  this.videoTextureView.scaleY = 1;
	                } else {
	                  this.videoTextureView.scaleY = expectedAspectRatio / realAspectRatio;
	                  this.videoTextureView.scaleX = 1;
	                }
	              } catch (e) {
	                Log.error('texImage2d video', e);
	                this._stopUpdatingVideoTexture();
	                this.videoTextureView.visible = false;
	              }
	              this.videoTexture.source.forceRenderUpdate();
	            }
	          }
	        };
	      }
	      if (!this._updatingVideoTexture) {
	        stage.on('frameStart', this._updateVideoTexture);
	        this._updatingVideoTexture = true;
	      }
	    }
	  }

	  _stopUpdatingVideoTexture() {
	    if (this.textureMode) {
	      const stage = this.stage;
	      stage.removeListener('frameStart', this._updateVideoTexture);
	      this._updatingVideoTexture = false;
	      this.videoTextureView.visible = false;

	      if (this.videoTexture.options.source) {
	        const gl = stage.gl;
	        gl.bindTexture(gl.TEXTURE_2D, this.videoTexture.options.source);
	        gl.clearColor(0, 0, 0, 1);
	        gl.clear(gl.COLOR_BUFFER_BIT);
	      }
	    }
	  }

	  updateSettings(settings = {}) {
	    // The Component that 'consumes' the media player.
	    this._consumer = settings.consumer;

	    if (this._consumer && this._consumer.getMediaplayerSettings) {
	      // Allow consumer to add settings.
	      settings = Object.assign(settings, this._consumer.getMediaplayerSettings());
	    }

	    if (!Lightning.Utils.equalValues(this._stream, settings.stream)) {
	      if (settings.stream && settings.stream.keySystem) {
	        navigator
	          .requestMediaKeySystemAccess(
	            settings.stream.keySystem.id,
	            settings.stream.keySystem.config
	          )
	          .then(keySystemAccess => {
	            return keySystemAccess.createMediaKeys()
	          })
	          .then(createdMediaKeys => {
	            return this.videoEl.setMediaKeys(createdMediaKeys)
	          })
	          .then(() => {
	            if (settings.stream && settings.stream.src) this.open(settings.stream.src);
	          })
	          .catch(() => {
	            console.error('Failed to set up MediaKeys');
	          });
	      } else if (settings.stream && settings.stream.src) {
	        // This is here to be backwards compatible, will be removed
	        // in future sdk release
	        if (Settings.get('app', 'hls')) {
	          if (!window.Hls) {
	            window.Hls = class Hls {
	              static isSupported() {
	                console.warn('hls-light not included');
	                return false
	              }
	            };
	          }
	          if (window.Hls.isSupported()) {
	            if (!this._hls) this._hls = new window.Hls({ liveDurationInfinity: true });
	            this._hls.loadSource(settings.stream.src);
	            this._hls.attachMedia(this.videoEl);
	            this.videoEl.style.display = 'block';
	          }
	        } else {
	          this.open(settings.stream.src);
	        }
	      } else {
	        this.close();
	      }
	      this._stream = settings.stream;
	    }

	    this._setHide(settings.hide);
	    this._setVideoArea(settings.videoPos);
	  }

	  _setHide(hide) {
	    if (this.textureMode) {
	      this.tag('Video').setSmooth('alpha', hide ? 0 : 1);
	    } else {
	      this.videoEl.style.visibility = hide ? 'hidden' : 'visible';
	    }
	  }

	  open(url, settings = { hide: false, videoPosition: null }) {
	    // prep the media url to play depending on platform (mediaPlayerplugin)
	    url = mediaUrl(url);
	    this._metrics = Metrics$1.media(url);
	    Log.info('Playing stream', url);
	    if (this.application.noVideo) {
	      Log.info('noVideo option set, so ignoring: ' + url);
	      return
	    }
	    // close the video when opening same url as current (effectively reloading)
	    if (this.videoEl.getAttribute('src') === url) {
	      this.close();
	    }
	    this.videoEl.setAttribute('src', url);

	    // force hide, then force show (in next tick!)
	    // (fixes comcast playback rollover issue)
	    this.videoEl.style.visibility = 'hidden';
	    this.videoEl.style.display = 'none';

	    setTimeout(() => {
	      this.videoEl.style.display = 'block';
	      this.videoEl.style.visibility = 'visible';
	    });

	    this._setHide(settings.hide);
	    this._setVideoArea(settings.videoPosition || [0, 0, 1920, 1080]);
	  }

	  close() {
	    // We need to pause first in order to stop sound.
	    this.videoEl.pause();
	    this.videoEl.removeAttribute('src');

	    // force load to reset everything without errors
	    this.videoEl.load();

	    this._clearSrc();

	    this.videoEl.style.display = 'none';
	  }

	  playPause() {
	    if (this.isPlaying()) {
	      this.doPause();
	    } else {
	      this.doPlay();
	    }
	  }

	  get muted() {
	    return this.videoEl.muted
	  }

	  set muted(v) {
	    this.videoEl.muted = v;
	  }

	  get loop() {
	    return this.videoEl.loop
	  }

	  set loop(v) {
	    this.videoEl.loop = v;
	  }

	  isPlaying() {
	    return this._getState() === 'Playing'
	  }

	  doPlay() {
	    this.videoEl.play();
	  }

	  doPause() {
	    this.videoEl.pause();
	  }

	  reload() {
	    var url = this.videoEl.getAttribute('src');
	    this.close();
	    this.videoEl.src = url;
	  }

	  getPosition() {
	    return Promise.resolve(this.videoEl.currentTime)
	  }

	  setPosition(pos) {
	    this.videoEl.currentTime = pos;
	  }

	  getDuration() {
	    return Promise.resolve(this.videoEl.duration)
	  }

	  seek(time, absolute = false) {
	    if (absolute) {
	      this.videoEl.currentTime = time;
	    } else {
	      this.videoEl.currentTime += time;
	    }
	  }

	  get videoTextureView() {
	    return this.tag('Video').tag('VideoTexture')
	  }

	  get videoTexture() {
	    return this.videoTextureView.texture
	  }

	  _setVideoArea(videoPos) {
	    if (Lightning.Utils.equalValues(this._videoPos, videoPos)) {
	      return
	    }

	    this._videoPos = videoPos;

	    if (this.textureMode) {
	      this.videoTextureView.patch({
	        smooth: {
	          x: videoPos[0],
	          y: videoPos[1],
	          w: videoPos[2] - videoPos[0],
	          h: videoPos[3] - videoPos[1],
	        },
	      });
	    } else {
	      const precision = this.stage.getRenderPrecision();
	      this.videoEl.style.left = Math.round(videoPos[0] * precision) + 'px';
	      this.videoEl.style.top = Math.round(videoPos[1] * precision) + 'px';
	      this.videoEl.style.width = Math.round((videoPos[2] - videoPos[0]) * precision) + 'px';
	      this.videoEl.style.height = Math.round((videoPos[3] - videoPos[1]) * precision) + 'px';
	    }
	  }

	  _fireConsumer(event, args) {
	    if (this._consumer) {
	      this._consumer.fire(event, args);
	    }
	  }

	  _equalInitData(buf1, buf2) {
	    if (!buf1 || !buf2) return false
	    if (buf1.byteLength != buf2.byteLength) return false
	    const dv1 = new Int8Array(buf1);
	    const dv2 = new Int8Array(buf2);
	    for (let i = 0; i != buf1.byteLength; i++) if (dv1[i] != dv2[i]) return false
	    return true
	  }

	  error(args) {
	    this._fireConsumer('$mediaplayerError', args);
	    this._setState('');
	    return ''
	  }

	  loadeddata(args) {
	    this._fireConsumer('$mediaplayerLoadedData', args);
	  }

	  play(args) {
	    this._fireConsumer('$mediaplayerPlay', args);
	  }

	  playing(args) {
	    this._fireConsumer('$mediaplayerPlaying', args);
	    this._setState('Playing');
	  }

	  canplay(args) {
	    this.videoEl.play();
	    this._fireConsumer('$mediaplayerStart', args);
	  }

	  loadstart(args) {
	    this._fireConsumer('$mediaplayerLoad', args);
	  }

	  seeked() {
	    this._fireConsumer('$mediaplayerSeeked', {
	      currentTime: this.videoEl.currentTime,
	      duration: this.videoEl.duration || 1,
	    });
	  }

	  seeking() {
	    this._fireConsumer('$mediaplayerSeeking', {
	      currentTime: this.videoEl.currentTime,
	      duration: this.videoEl.duration || 1,
	    });
	  }

	  durationchange(args) {
	    this._fireConsumer('$mediaplayerDurationChange', args);
	  }

	  encrypted(args) {
	    const video = args.videoElement;
	    const event = args.event;
	    // FIXME: Double encrypted events need to be properly filtered by Gstreamer
	    if (video.mediaKeys && !this._equalInitData(this._previousInitData, event.initData)) {
	      this._previousInitData = event.initData;
	      this._fireConsumer('$mediaplayerEncrypted', args);
	    }
	  }

	  static _states() {
	    return [
	      class Playing extends this {
	        $enter() {
	          this._startUpdatingVideoTexture();
	        }
	        $exit() {
	          this._stopUpdatingVideoTexture();
	        }
	        timeupdate() {
	          this._fireConsumer('$mediaplayerProgress', {
	            currentTime: this.videoEl.currentTime,
	            duration: this.videoEl.duration || 1,
	          });
	        }
	        ended(args) {
	          this._fireConsumer('$mediaplayerEnded', args);
	          this._setState('');
	        }
	        pause(args) {
	          this._fireConsumer('$mediaplayerPause', args);
	          this._setState('Playing.Paused');
	        }
	        _clearSrc() {
	          this._fireConsumer('$mediaplayerStop', {});
	          this._setState('');
	        }
	        static _states() {
	          return [class Paused extends this {}]
	        }
	      },
	    ]
	  }
	}

	class localCookie{constructor(e){return e=e||{},this.forceCookies=e.forceCookies||!1,!0===this._checkIfLocalStorageWorks()&&!0!==e.forceCookies?{getItem:this._getItemLocalStorage,setItem:this._setItemLocalStorage,removeItem:this._removeItemLocalStorage,clear:this._clearLocalStorage}:{getItem:this._getItemCookie,setItem:this._setItemCookie,removeItem:this._removeItemCookie,clear:this._clearCookies}}_checkIfLocalStorageWorks(){if("undefined"==typeof localStorage)return !1;try{return localStorage.setItem("feature_test","yes"),"yes"===localStorage.getItem("feature_test")&&(localStorage.removeItem("feature_test"),!0)}catch(e){return !1}}_getItemLocalStorage(e){return window.localStorage.getItem(e)}_setItemLocalStorage(e,t){return window.localStorage.setItem(e,t)}_removeItemLocalStorage(e){return window.localStorage.removeItem(e)}_clearLocalStorage(){return window.localStorage.clear()}_getItemCookie(e){var t=document.cookie.match(RegExp("(?:^|;\\s*)"+function(e){return e.replace(/([.*+?\^${}()|\[\]\/\\])/g,"\\$1")}(e)+"=([^;]*)"));return t&&""===t[1]&&(t[1]=null),t?t[1]:null}_setItemCookie(e,t){document.cookie=`${e}=${t}`;}_removeItemCookie(e){document.cookie=`${e}=;Max-Age=-99999999;`;}_clearCookies(){document.cookie.split(";").forEach(e=>{document.cookie=e.replace(/^ +/,"").replace(/=.*/,"=;expires=Max-Age=-99999999");});}}

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

	let namespace;
	let lc;

	const initStorage = () => {
	  namespace = Settings.get('platform', 'appId');
	  // todo: pass options (for example to force the use of cookies)
	  lc = new localCookie();
	};

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

	const isFunction = v => {
	  return typeof v === 'function'
	};

	const isObject = v => {
	  return typeof v === 'object' && v !== null
	};

	const isPage = v => {
	  if (v instanceof Lightning.Element || isLightningComponent(v)) {
	    return true
	  }
	  return false
	};

	const isLightningComponent = type => {
	  return type.prototype && 'isComponent' in type.prototype
	};

	const isArray = v => {
	  return Array.isArray(v)
	};

	const ucfirst = v => {
	  return `${v.charAt(0).toUpperCase()}${v.slice(1)}`
	};

	const isString = v => {
	  return typeof v === 'string'
	};

	const isPromise = (method, args) => {
	  let result;
	  if (isFunction(method)) {
	    try {
	      result = method.apply(null);
	    } catch (e) {
	      result = e;
	    }
	  } else {
	    result = method;
	  }
	  return isObject(result) && isFunction(result.then)
	};

	const incorrectParams = (cb, route) => {
	  const isIncorrect = /^\w*?\s?\(\s?\{.*?\}\s?\)/i;
	  if (isIncorrect.test(cb.toString())) {
	    console.warn(
	      [
	        `DEPRECATION: The data-provider for route: ${route} is not correct.`,
	        '"page" is no longer a property of the params object but is now the first function parameter: ',
	        'https://github.com/rdkcentral/Lightning-SDK/blob/feature/router/docs/plugins/router/dataproviding.md#data-providing',
	        "It's supported for now but will be removed in a future release.",
	      ].join('\n')
	    );
	    return true
	  }
	  return false
	};

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

	const fade = (i, o) => {
	  return new Promise(resolve => {
	    i.patch({
	      alpha: 0,
	      visible: true,
	      smooth: {
	        alpha: [1, { duration: 0.5, delay: 0.1 }],
	      },
	    });
	    // resolve on y finish
	    i.transition('alpha').on('finish', () => {
	      if (o) {
	        o.visible = false;
	      }
	      resolve();
	    });
	  })
	};

	const crossFade = (i, o) => {
	  return new Promise(resolve => {
	    i.patch({
	      alpha: 0,
	      visible: true,
	      smooth: {
	        alpha: [1, { duration: 0.5, delay: 0.1 }],
	      },
	    });
	    if (o) {
	      o.patch({
	        smooth: {
	          alpha: [0, { duration: 0.5, delay: 0.3 }],
	        },
	      });
	    }
	    // resolve on y finish
	    i.transition('alpha').on('finish', () => {
	      resolve();
	    });
	  })
	};

	const moveOnAxes = (axis, direction, i, o) => {
	  const bounds = axis === 'x' ? 1920 : 1080;
	  return new Promise(resolve => {
	    i.patch({
	      [`${axis}`]: direction ? bounds * -1 : bounds,
	      visible: true,
	      smooth: {
	        [`${axis}`]: [0, { duration: 0.4, delay: 0.2 }],
	      },
	    });
	    // out is optional
	    if (o) {
	      o.patch({
	        [`${axis}`]: 0,
	        smooth: {
	          [`${axis}`]: [direction ? bounds : bounds * -1, { duration: 0.4, delay: 0.2 }],
	        },
	      });
	    }
	    // resolve on y finish
	    i.transition(axis).on('finish', () => {
	      resolve();
	    });
	  })
	};

	const up = (i, o) => {
	  return moveOnAxes('y', 0, i, o)
	};

	const down = (i, o) => {
	  return moveOnAxes('y', 1, i, o)
	};

	const left = (i, o) => {
	  return moveOnAxes('x', 0, i, o)
	};

	const right = (i, o) => {
	  return moveOnAxes('x', 1, i, o)
	};

	var Transitions = {
	  fade,
	  crossFade,
	  up,
	  down,
	  left,
	  right,
	};

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

	class RoutedApp extends Lightning.Component {
	  static _template() {
	    return {
	      Pages: {
	        forceZIndexContext: true,
	      },
	      /**
	       * This is a default Loading page that will be made visible
	       * during data-provider on() you CAN override in child-class
	       */
	      Loading: {
	        rect: true,
	        w: 1920,
	        h: 1080,
	        color: 0xff000000,
	        visible: false,
	        zIndex: 99,
	        Label: {
	          mount: 0.5,
	          x: 960,
	          y: 540,
	          text: {
	            text: 'Loading..',
	          },
	        },
	      },
	    }
	  }

	  static _states() {
	    return [
	      class Loading extends this {
	        $enter() {
	          this.tag('Loading').visible = true;
	        }

	        $exit() {
	          this.tag('Loading').visible = false;
	        }
	      },
	      class Widgets extends this {
	        $enter(args, widget) {
	          // store widget reference
	          this._widget = widget;

	          // since it's possible that this behaviour
	          // is non-remote driven we force a recalculation
	          // of the focuspath
	          this._refocus();
	        }

	        _getFocused() {
	          // we delegate focus to selected widget
	          // so it can consume remotecontrol presses
	          return this._widget
	        }

	        // if we want to widget to widget focus delegation
	        reload(widget) {
	          this._widget = widget;
	          this._refocus();
	        }

	        _handleKey() {
	          restore();
	        }
	      },
	    ]
	  }

	  /**
	   * Return location where pages need to be stored
	   */
	  get pages() {
	    return this.tag('Pages')
	  }

	  /**
	   * Tell router where widgets are stored
	   */
	  get widgets() {
	    return this.tag('Widgets')
	  }

	  /**
	   * we MUST register _handleBack method so the Router
	   * can override it
	   * @private
	   */
	  _handleBack() {}

	  /**
	   * we MUST register _captureKey for dev quick-navigation
	   * (via keyboard 1-9)
	   */
	  _captureKey() {}

	  /**
	   * We MUST return Router.activePage() so the new Page
	   * can listen to the remote-control.
	   */
	  _getFocused() {
	    return getActivePage()
	  }
	}

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

	let getHash = () => {
	  return document.location.hash
	};

	let setHash = url => {
	  document.location.hash = url;
	};

	const initRouter = config => {
	  if (config.getHash) {
	    getHash = config.getHash;
	  }
	  if (config.setHash) {
	    setHash = config.setHash;
	  }
	};

	//instance of Lightning.Component
	let app;

	let stage;
	let widgetsHost;
	let pagesHost;

	const pages = new Map();
	const providers = new Map();
	const widgetsPerRoute = new Map();

	let register = new Map();
	let routerConfig;

	// widget that has focus
	let activeWidget;

	// page that has focus
	let activePage;
	const hasRegex = /\{\/(.*?)\/([igm]{0,3})\}/g;

	const create = type => {
	  const page = stage.c({ type, visible: false });

	  return page
	};

	/**
	 * The actual loading of the component
	 * @param {String} route - the route blueprint, used for data provider look up
	 * @param {String} hash - current hash we're routing to
	 * */
	const load = async ({ route, hash }) => {
	  const type = getPageByRoute(route);
	  let routesShareInstance = false;
	  let provide = false;
	  let page = null;
	  let isCreated = false;

	  // if page is instanceof Component
	  if (!isLightningComponent(type)) {
	    page = type;
	    // if we have have a data route for current page
	    if (providers.has(route)) {
	      // if page is expired or new hash is different
	      // from previous hash when page was loaded
	      // effectively means: page could be loaded
	      // with new url parameters
	      if (isPageExpired(type) || type[Symbol.for('hash')] !== hash) {
	        provide = true;
	      }
	    }

	    let currentRoute = activePage && activePage[Symbol.for('route')];

	    // if the new route is equal to the current route it means that both
	    // route share the Component instance and stack location / since this case
	    // is conflicting with the way before() and after() loading works we flag it,
	    // and check platform settings in we want to re-use instance
	    if (route === currentRoute) {
	      routesShareInstance = true;
	    }
	  } else {
	    page = create(type);
	    pagesHost.a(page);

	    // update stack
	    const location = getPageStackLocation(route);
	    if (!isNaN(location)) {
	      let stack = pages.get(route);
	      stack[location] = page;
	      pages.set(route, stack);
	    }

	    // test if need to request data provider
	    if (providers.has(route)) {
	      provide = true;
	    }

	    isCreated = true;
	  }

	  // we store hash and route as properties on the page instance
	  // that way we can easily calculate new behaviour on page reload
	  page[Symbol.for('hash')] = hash;
	  page[Symbol.for('route')] = route;

	  // if routes share instance we only update
	  // update the page data if needed
	  if (routesShareInstance) {
	    if (provide) {
	      try {
	        await updatePageData({ page, route, hash });
	        emit(page, ['dataProvided', 'changed']);
	      } catch (e) {
	        // show error page with route / hash
	        // and optional error code
	        handleError(e);
	      }
	    } else {
	      providePageData({ page, route, hash, provide: false });
	      emit(page, 'changed');
	    }
	  } else {
	    if (provide) {
	      const { type: loadType } = providers.get(route);
	      const properties = {
	        page,
	        old: activePage,
	        route,
	        hash,
	      };
	      try {
	        if (triggers[loadType]) {
	          await triggers[loadType](properties);
	          emit(page, ['dataProvided', isCreated ? 'mounted' : 'changed']);
	        } else {
	          throw new Error(`${loadType} is not supported`)
	        }
	      } catch (e) {
	        handleError(page, e);
	      }
	    } else {
	      const p = activePage;
	      const r = p && p[Symbol.for('route')];

	      providePageData({ page, route, hash, provide: false });
	      doTransition(page, activePage).then(() => {
	        // manage cpu/gpu memory
	        if (p) {
	          cleanUp(p, r);
	        }

	        emit(page, isCreated ? 'mounted' : 'changed');

	        // force focus calculation
	        app._refocus();
	      });
	    }
	  }

	  // store reference to active page, probably better to store the
	  // route in the future
	  activePage = page;

	  if (widgetsPerRoute.size && widgetsHost) {
	    updateWidgets(page);
	  }

	  Log.info('[route]:', route);
	  Log.info('[hash]:', hash);

	  return page
	};

	const triggerAfter = ({ page, old, route, hash }) => {
	  return doTransition(page, old).then(() => {
	    // if the current and previous route (blueprint) are equal
	    // we're loading the same page again but provide it with new data
	    // in that case we don't clean-up the old page (since we're re-using)
	    if (old) {
	      cleanUp(old, old[Symbol.for('route')]);
	    }

	    // update provided page data
	    return updatePageData({ page, route, hash })
	  })
	};

	const triggerBefore = ({ page, old, route, hash }) => {
	  return updatePageData({ page, route, hash })
	    .then(() => {
	      return doTransition(page, old)
	    })
	    .then(() => {
	      if (old) {
	        cleanUp(old, old[Symbol.for('route')]);
	      }
	    })
	};

	const triggerOn = ({ page, old, route, hash }) => {
	  app._setState('Loading');

	  if (old) {
	    cleanUp(old, old[Symbol.for('route')]);
	  }

	  return updatePageData({ page, route, hash })
	    .then(() => {
	      // @todo: fix zIndex for transition
	      return doTransition(page)
	    })
	    .then(() => {
	      // @todo: make state configurable
	      {
	        app._setState('');
	      }
	    })
	};

	const emit = (page, events = [], params = {}) => {
	  if (!isArray(events)) {
	    events = [events];
	  }
	  events.forEach(e => {
	    const event = `_on${ucfirst(e)}`;
	    if (isFunction(page[event])) {
	      page[event](params);
	    }
	  });
	};

	const handleError = (page, error = 'error unkown') => {
	  // force expire
	  page[Symbol.for('expires')] = Date.now();

	  if (pages.has('!')) {
	    load({ route: '!', hash: page[Symbol.for('hash')] }).then(errorPage => {
	      errorPage.error = { page, error };

	      // on() loading type will force the app to go
	      // in a loading state so on error we need to
	      // go back to root state
	      if (app.state === 'Loading') ;

	      // make sure we delegate focus to the error page
	      if (activePage !== errorPage) {
	        activePage = errorPage;
	        app._refocus();
	      }
	    });
	  } else {
	    Log.error(page, error);
	  }
	};

	const triggers = {
	  on: triggerOn,
	  after: triggerAfter,
	  before: triggerBefore,
	};

	const providePageData = ({ page, route, hash }) => {
	  const urlValues = getValuesFromHash(hash, route);
	  const pageData = new Map([...urlValues, ...register]);
	  const params = {};

	  // make dynamic url data available to the page
	  // as instance properties
	  for (let [name, value] of pageData) {
	    page[name] = value;
	    params[name] = value;
	  }

	  // check navigation register for persistent data
	  if (register.size) {
	    const obj = {};
	    for (let [k, v] of register) {
	      obj[k] = v;
	    }
	    page.persist = obj;
	  }

	  // make url data and persist data available
	  // via params property
	  page.params = params;

	  emit(page, ['urlParams'], params);

	  return params
	};

	const updatePageData = ({ page, route, hash, provide = true }) => {
	  const { cb, expires } = providers.get(route);
	  const params = providePageData({ page, route, hash });

	  if (!provide) {
	    return Promise.resolve()
	  }
	  /**
	   * In the first version of the Router, a reference to the page is made
	   * available to the callback function as property of {params}.
	   * Since this is error prone (named url parts are also being spread inside this object)
	   * we made the page reference the first parameter and url values the second.
	   * -
	   * We keep it backwards compatible for now but a warning is showed in the console.
	   */
	  if (incorrectParams(cb, route)) {
	    // keep page as params property backwards compatible for now
	    return cb({ page, ...params }).then(() => {
	      page[Symbol.for('expires')] = Date.now() + expires;
	    })
	  } else {
	    return cb(page, { ...params }).then(() => {
	      page[Symbol.for('expires')] = Date.now() + expires;
	    })
	  }
	};

	/**
	 * execute transition between new / old page and
	 * toggle the defined widgets
	 * @todo: platform override default transition
	 * @param pageIn
	 * @param pageOut
	 */
	const doTransition = (pageIn, pageOut = null) => {
	  const transition = pageIn.pageTransition || pageIn.easing;
	  const hasCustomTransitions = !!(pageIn.smoothIn || pageIn.smoothInOut || transition);
	  const transitionsDisabled = routerConfig.get('disableTransitions');

	  // default behaviour is a visibility toggle
	  if (!hasCustomTransitions || transitionsDisabled) {
	    pageIn.visible = true;
	    if (pageOut) {
	      pageOut.visible = false;
	    }
	    return Promise.resolve()
	  }

	  if (transition) {
	    let type;
	    try {
	      type = transition.call(pageIn, pageIn, pageOut);
	    } catch (e) {
	      type = 'crossFade';
	    }

	    if (isPromise(type)) {
	      return type
	    }

	    if (isString(type)) {
	      const fn = Transitions[type];
	      if (fn) {
	        return fn(pageIn, pageOut)
	      }
	    }

	    // keep backwards compatible for now
	    if (pageIn.smoothIn) {
	      // provide a smooth function that resolves itself
	      // on transition finish
	      const smooth = (p, v, args = {}) => {
	        return new Promise(resolve => {
	          pageIn.visible = true;
	          pageIn.setSmooth(p, v, args);
	          pageIn.transition(p).on('finish', () => {
	            resolve();
	          });
	        })
	      };
	      return pageIn.smoothIn({ pageIn, smooth })
	    }
	  }

	  return Transitions.crossFade(pageIn, pageOut)
	};

	/**
	 * update the visibility of the available widgets
	 * for the current page / route
	 * @param page
	 */
	const updateWidgets = page => {
	  const route = page[Symbol.for('route')];

	  // force lowercase lookup
	  const configured = (widgetsPerRoute.get(route) || []).map(ref => ref.toLowerCase());

	  widgetsHost.forEach(widget => {
	    widget.visible = configured.indexOf(widget.ref.toLowerCase()) !== -1;
	    if (widget.visible) {
	      emit(widget, ['activated'], page);
	    }
	  });

	  if (app.state === 'Widgets' ) ;
	};

	const cleanUp = (page, route) => {
	  let doCleanup = false;
	  const lazyDestroy = routerConfig.get('lazyDestroy');
	  const destroyOnBack = routerConfig.get('destroyOnHistoryBack');
	  const keepAlive = read('keepAlive');
	  const isFromHistory = read('@router:backtrack');

	  if (isFromHistory && (destroyOnBack || lazyDestroy)) {
	    doCleanup = true;
	  } else if (lazyDestroy && !keepAlive) {
	    doCleanup = true;
	  }

	  if (doCleanup) {
	    // in lazy create mode we store constructor
	    // and remove the actual page from host
	    const stack = pages.get(route);
	    const location = getPageStackLocation(route);

	    // grab original class constructor if statemachine routed
	    // else store constructor
	    stack[location] = page._routedType || page.constructor;
	    pages.set(route, stack);

	    // actual remove of page from memory
	    pagesHost.remove(page);

	    // force texture gc() if configured
	    // so we can cleanup textures in the same tick
	    if (routerConfig.get('gcOnUnload')) {
	      stage.gc();
	    }
	  }
	};

	/**
	 * Test if page passed cache-time
	 * @param page
	 * @returns {boolean}
	 */
	const isPageExpired = page => {
	  if (!page[Symbol.for('expires')]) {
	    return false
	  }

	  const expires = page[Symbol.for('expires')];
	  const now = Date.now();

	  return now >= expires
	};

	const getPageByRoute = route => {
	  return getPageFromStack(route).item
	};

	/**
	 * Returns the current location of a page constructor or
	 * page instance for a route
	 * @param route
	 */
	const getPageStackLocation = route => {
	  return getPageFromStack(route).index
	};

	const getPageFromStack = route => {
	  if (!pages.has(route)) {
	    return false
	  }

	  let index = -1;
	  let item = null;
	  let stack = pages.get(route);
	  if (!Array.isArray(stack)) {
	    stack = [stack];
	  }

	  for (let i = 0, j = stack.length; i < j; i++) {
	    if (isPage(stack[i])) {
	      index = i;
	      item = stack[i];
	      break
	    }
	  }

	  return { index, item }
	};

	/**
	 * Simple route length calculation
	 * @param route {string}
	 * @returns {number} - floor
	 */
	const getFloor = route => {
	  return stripRegex(route).split('/').length
	};

	/**
	 * Test if a route is part regular expressed
	 * and replace it for a simple character
	 * @param route
	 * @returns {*}
	 */
	const stripRegex = (route, char = 'R') => {
	  // if route is part regular expressed we replace
	  // the regular expression for a character to
	  // simplify floor calculation and backtracking
	  if (hasRegex.test(route)) {
	    route = route.replace(hasRegex, char);
	  }
	  return route
	};

	/**
	 * return all stored routes that live on the same floor
	 * @param floor
	 * @returns {Array}
	 */
	const getRoutesByFloor = floor => {
	  const matches = [];
	  // simple filter of level candidates
	  for (let [route] of pages.entries()) {
	    if (getFloor(route) === floor) {
	      matches.push(route);
	    }
	  }
	  return matches
	};

	/**
	 * return a matching route by provided hash
	 * hash: home/browse/12 will match:
	 * route: home/browse/:categoryId
	 * @param hash {string}
	 * @returns {string|boolean} - route
	 */
	const getRouteByHash = hash => {
	  const getUrlParts = /(\/?:?[@\w%\s-]+)/g;
	  // grab possible candidates from stored routes
	  const candidates = getRoutesByFloor(getFloor(hash));
	  // break hash down in chunks
	  const hashParts = hash.match(getUrlParts) || [];
	  // test if the part of the hash has a replace
	  // regex lookup id
	  const hasLookupId = /\/:\w+?@@([0-9]+?)@@/;
	  const isNamedGroup = /^\/:/;

	  // we skip wildcard routes
	  const skipRoutes = ['!', '*', '$'];

	  // to simplify the route matching and prevent look around
	  // in our getUrlParts regex we get the regex part from
	  // route candidate and store them so that we can reference
	  // them when we perform the actual regex against hash
	  let regexStore = [];

	  let matches = candidates.filter(route => {
	    let isMatching = true;

	    if (skipRoutes.indexOf(route) !== -1) {
	      return false
	    }

	    // replace regex in route with lookup id => @@{storeId}@@
	    if (hasRegex.test(route)) {
	      const regMatches = route.match(hasRegex);
	      if (regMatches && regMatches.length) {
	        route = regMatches.reduce((fullRoute, regex) => {
	          const lookupId = regexStore.length;
	          fullRoute = fullRoute.replace(regex, `@@${lookupId}@@`);
	          regexStore.push(regex.substring(1, regex.length - 1));
	          return fullRoute
	        }, route);
	      }
	    }

	    const routeParts = route.match(getUrlParts) || [];

	    for (let i = 0, j = routeParts.length; i < j; i++) {
	      const routePart = routeParts[i];
	      const hashPart = hashParts[i];

	      // Since we support catch-all and regex driven name groups
	      // we first test for regex lookup id and see if the regex
	      // matches the value from the hash
	      if (hasLookupId.test(routePart)) {
	        const routeMatches = hasLookupId.exec(routePart);
	        const storeId = routeMatches[1];
	        const routeRegex = regexStore[storeId];

	        // split regex and modifiers so we can use both
	        // to create a new RegExp
	        // eslint-disable-next-line
	        const regMatches = /\/([^\/]+)\/([igm]{0,3})/.exec(routeRegex);

	        if (regMatches && regMatches.length) {
	          const expression = regMatches[1];
	          const modifiers = regMatches[2];

	          const regex = new RegExp(`^/${expression}$`, modifiers);

	          if (!regex.test(hashPart)) {
	            isMatching = false;
	          }
	        }
	      } else if (isNamedGroup.test(routePart)) {
	        // we kindly skip namedGroups because this is dynamic
	        // we only need to the static and regex drive parts
	        continue
	      } else if (hashPart && routePart.toLowerCase() !== hashPart.toLowerCase()) {
	        isMatching = false;
	      }
	    }
	    return isMatching
	  });

	  if (matches.length) {
	    // we give prio to static routes over dynamic
	    matches = matches.sort(a => {
	      return isNamedGroup.test(a) ? -1 : 1
	    });
	    return matches[0]
	  }

	  return false
	};

	/**
	 * Extract dynamic values from location hash and return a namedgroup
	 * of key (from route) value (from hash) pairs
	 * @param hash {string} - the actual location hash
	 * @param route {string} - the route as defined in route
	 */
	const getValuesFromHash = (hash, route) => {
	  // replace the regex definition from the route because
	  // we already did the matching part
	  route = stripRegex(route, '');

	  const getUrlParts = /(\/?:?[\w%\s-]+)/g;
	  const hashParts = hash.match(getUrlParts) || [];
	  const routeParts = route.match(getUrlParts) || [];
	  const getNamedGroup = /^\/:([\w-]+)\/?/;

	  return routeParts.reduce((storage, value, index) => {
	    const match = getNamedGroup.exec(value);
	    if (match && match.length) {
	      storage.set(match[1], decodeURIComponent(hashParts[index].replace(/^\//, '')));
	    }
	    return storage
	  }, new Map())
	};

	const handleHashChange = override => {
	  const hash = override || getHash();
	  const route = getRouteByHash(hash);

	  if (route) {
	    // would be strange if this fails but we do check
	    if (pages.has(route)) {
	      let stored = pages.get(route);
	      if (!isArray(stored)) {
	        stored = [stored];
	      }
	      let n = stored.length;
	      while (n--) {
	        const type = stored[n];
	        if (isPage(type)) {
	          load({ route, hash }).then(() => {
	            app._refocus();
	          });
	        } else {
	          const urlParams = getValuesFromHash(hash, route);
	          const params = {};
	          for (const key of urlParams.keys()) {
	            params[key] = urlParams.get(key);
	          }
	          // invoke
	          type.call(null, app, { ...params });
	        }
	      }
	    }
	  } else {
	    if (pages.has('*')) {
	      load({ route: '*', hash }).then(() => {
	        app._refocus();
	      });
	    }
	  }
	};

	const read = flag => {
	  if (register.has(flag)) {
	    return register.get(flag)
	  }
	  return false
	};

	const getWidgetByName = name => {
	  name = ucfirst(name);
	  return widgetsHost.getByRef(name) || false
	};

	/**
	 * delegate app focus to a on-screen widget
	 * @param name - {string}
	 */
	const focusWidget = name => {
	  const widget = getWidgetByName(name);
	  if (name) {
	    // store reference
	    activeWidget = widget;
	    // somewhat experimental
	    if (app.state === 'Widgets') ; else {
	      app._setState('Widgets', [activeWidget]);
	    }
	  }
	};

	const handleRemote = (type, name) => {
	  if (type === 'widget') {
	    focusWidget(name);
	  } else if (type === 'page') {
	    restoreFocus();
	  }
	};

	const restore = () => {
	  if (routerConfig.get('autoRestoreRemote')) {
	    handleRemote('page');
	  }
	};

	const restoreFocus = () => {
	  activeWidget = null;
	  app._setState('');
	};

	const getActivePage = () => {
	  if (activePage && activePage.attached) {
	    return activePage
	  } else {
	    return app
	  }
	};

	// listen to url changes
	window.addEventListener('hashchange', () => {
	  handleHashChange();
	});

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

	const defaultChannels = [
	  {
	    number: 1,
	    name: 'Metro News 1',
	    description: 'New York Cable News Channel',
	    entitled: true,
	    program: {
	      title: 'The Morning Show',
	      description: "New York's best morning show",
	      startTime: new Date(new Date() - 60 * 5 * 1000).toUTCString(), // started 5 minutes ago
	      duration: 60 * 30, // 30 minutes
	      ageRating: 0,
	    },
	  },
	  {
	    number: 2,
	    name: 'MTV',
	    description: 'Music Television',
	    entitled: true,
	    program: {
	      title: 'Beavis and Butthead',
	      description: 'American adult animated sitcom created by Mike Judge',
	      startTime: new Date(new Date() - 60 * 20 * 1000).toUTCString(), // started 20 minutes ago
	      duration: 60 * 45, // 45 minutes
	      ageRating: 18,
	    },
	  },
	  {
	    number: 3,
	    name: 'NBC',
	    description: 'NBC TV Network',
	    entitled: false,
	    program: {
	      title: 'The Tonight Show Starring Jimmy Fallon',
	      description: 'Late-night talk show hosted by Jimmy Fallon on NBC',
	      startTime: new Date(new Date() - 60 * 10 * 1000).toUTCString(), // started 10 minutes ago
	      duration: 60 * 60, // 1 hour
	      ageRating: 10,
	    },
	  },
	];

	const channels = () => Settings.get('platform', 'tv', defaultChannels);

	const randomChannel = () => channels()[~~(channels.length * Math.random())];

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

	let currentChannel;
	const callbacks = {};

	const emit$1 = (event, ...args) => {
	  callbacks[event] &&
	    callbacks[event].forEach(cb => {
	      cb.apply(null, args);
	    });
	};

	// local mock methods
	let methods = {
	  getChannel() {
	    if (!currentChannel) currentChannel = randomChannel();
	    return new Promise((resolve, reject) => {
	      if (currentChannel) {
	        const channel = { ...currentChannel };
	        delete channel.program;
	        resolve(channel);
	      } else {
	        reject('No channel found');
	      }
	    })
	  },
	  getProgram() {
	    if (!currentChannel) currentChannel = randomChannel();
	    return new Promise((resolve, reject) => {
	      currentChannel.program ? resolve(currentChannel.program) : reject('No program found');
	    })
	  },
	  setChannel(number) {
	    return new Promise((resolve, reject) => {
	      if (number) {
	        const newChannel = channels().find(c => c.number === number);
	        if (newChannel) {
	          currentChannel = newChannel;
	          const channel = { ...currentChannel };
	          delete channel.program;
	          emit$1('channelChange', channel);
	          resolve(channel);
	        } else {
	          reject('Channel not found');
	        }
	      } else {
	        reject('No channel number supplied');
	      }
	    })
	  },
	};

	const initTV = config => {
	  methods = {};
	  if (config.getChannel && typeof config.getChannel === 'function') {
	    methods.getChannel = config.getChannel;
	  }
	  if (config.getProgram && typeof config.getProgram === 'function') {
	    methods.getProgram = config.getProgram;
	  }
	  if (config.setChannel && typeof config.setChannel === 'function') {
	    methods.setChannel = config.setChannel;
	  }
	  if (config.emit && typeof config.emit === 'function') {
	    config.emit(emit$1);
	  }
	};

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

	let ApplicationInstance;

	var Launch = (App, appSettings, platformSettings, appData) => {
	  initSettings(appSettings, platformSettings);

	  initUtils(platformSettings);
	  initStorage();

	  // Initialize plugins
	  if (platformSettings.plugins) {
	    platformSettings.plugins.profile && initProfile(platformSettings.plugins.profile);
	    platformSettings.plugins.metrics && initMetrics(platformSettings.plugins.metrics);
	    platformSettings.plugins.mediaPlayer && initMediaPlayer(platformSettings.plugins.mediaPlayer);
	    platformSettings.plugins.router && initRouter(platformSettings.plugins.router);
	    platformSettings.plugins.tv && initTV(platformSettings.plugins.tv);
	  }

	  const app = Application(App, appData, platformSettings);
	  ApplicationInstance = new app(appSettings);
	  return ApplicationInstance
	};

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

	class PinInput extends Lightning.Component {
	  static _template() {
	    return {
	      w: 120,
	      h: 150,
	      rect: true,
	      color: 0xff949393,
	      alpha: 0.5,
	      shader: { type: Lightning.shaders.RoundedRectangle, radius: 10 },
	      Nr: {
	        w: w => w,
	        y: 24,
	        text: {
	          text: '',
	          textColor: 0xff333333,
	          fontSize: 80,
	          textAlign: 'center',
	          verticalAlign: 'middle',
	        },
	      },
	    }
	  }

	  set index(v) {
	    this.x = v * (120 + 24);
	  }

	  set nr(v) {
	    this._timeout && clearTimeout(this._timeout);

	    if (v) {
	      this.setSmooth('alpha', 1);
	    } else {
	      this.setSmooth('alpha', 0.5);
	    }

	    this.tag('Nr').patch({
	      text: {
	        text: (v && v.toString()) || '',
	        fontSize: v === '*' ? 120 : 80,
	      },
	    });

	    if (v && v !== '*') {
	      this._timeout = setTimeout(() => {
	        this._timeout = null;
	        this.nr = '*';
	      }, 750);
	    }
	  }
	}

	class PinDialog extends Lightning.Component {
	  static _template() {
	    return {
	      w: w => w,
	      h: h => h,
	      rect: true,
	      color: 0xdd000000,
	      alpha: 0.000001,
	      Dialog: {
	        w: 648,
	        h: 320,
	        y: h => (h - 320) / 2,
	        x: w => (w - 648) / 2,
	        rect: true,
	        color: 0xdd333333,
	        shader: { type: Lightning.shaders.RoundedRectangle, radius: 10 },
	        Info: {
	          y: 24,
	          x: 48,
	          text: { text: 'Please enter your PIN', fontSize: 32 },
	        },
	        Msg: {
	          y: 260,
	          x: 48,
	          text: { text: '', fontSize: 28, textColor: 0xffffffff },
	        },
	        Code: {
	          x: 48,
	          y: 96,
	        },
	      },
	    }
	  }

	  _init() {
	    const children = [];
	    for (let i = 0; i < 4; i++) {
	      children.push({
	        type: PinInput,
	        index: i,
	      });
	    }

	    this.tag('Code').children = children;
	  }

	  get pin() {
	    if (!this._pin) this._pin = '';
	    return this._pin
	  }

	  set pin(v) {
	    if (v.length <= 4) {
	      const maskedPin = new Array(Math.max(v.length - 1, 0)).fill('*', 0, v.length - 1);
	      v.length && maskedPin.push(v.length > this._pin.length ? v.slice(-1) : '*');
	      for (let i = 0; i < 4; i++) {
	        this.tag('Code').children[i].nr = maskedPin[i] || '';
	      }
	      this._pin = v;
	    }
	  }

	  get msg() {
	    if (!this._msg) this._msg = '';
	    return this._msg
	  }

	  set msg(v) {
	    this._timeout && clearTimeout(this._timeout);

	    this._msg = v;
	    if (this._msg) {
	      this.tag('Msg').text = this._msg;
	      this.tag('Info').setSmooth('alpha', 0.5);
	      this.tag('Code').setSmooth('alpha', 0.5);
	    } else {
	      this.tag('Msg').text = '';
	      this.tag('Info').setSmooth('alpha', 1);
	      this.tag('Code').setSmooth('alpha', 1);
	    }
	    this._timeout = setTimeout(() => {
	      this.msg = '';
	    }, 2000);
	  }

	  _firstActive() {
	    this.setSmooth('alpha', 1);
	  }

	  _handleKey(event) {
	    if (this.msg) {
	      this.msg = false;
	    } else {
	      const val = parseInt(event.key);
	      if (val > -1) {
	        this.pin += val;
	      }
	    }
	  }

	  _handleBack() {
	    if (this.msg) {
	      this.msg = false;
	    } else {
	      if (this.pin.length) {
	        this.pin = this.pin.slice(0, this.pin.length - 1);
	      } else {
	        Pin.hide();
	        this.resolve(false);
	      }
	    }
	  }

	  _handleEnter() {
	    if (this.msg) {
	      this.msg = false;
	    } else {
	      Pin.submit(this.pin)
	        .then(val => {
	          this.msg = 'Unlocking ...';
	          setTimeout(() => {
	            Pin.hide();
	          }, 1000);
	          this.resolve(val);
	        })
	        .catch(e => {
	          this.msg = e;
	          this.reject(e);
	        });
	    }
	  }
	}

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

	// only used during local development
	let unlocked = false;

	let submit = pin => {
	  return new Promise((resolve, reject) => {
	    if (pin.toString() === Settings.get('platform', 'pin', '0000').toString()) {
	      unlocked = true;
	      resolve(unlocked);
	    } else {
	      reject('Incorrect pin');
	    }
	  })
	};

	let check = () => {
	  return new Promise(resolve => {
	    resolve(unlocked);
	  })
	};

	let pinDialog = null;

	// Public API
	var Pin = {
	  show() {
	    return new Promise((resolve, reject) => {
	      pinDialog = ApplicationInstance.stage.c({
	        ref: 'PinDialog',
	        type: PinDialog,
	        resolve,
	        reject,
	      });
	      ApplicationInstance.childList.a(pinDialog);
	      ApplicationInstance.focus = pinDialog;
	    })
	  },
	  hide() {
	    ApplicationInstance.focus = null;
	    ApplicationInstance.children = ApplicationInstance.children.map(
	      child => child !== pinDialog && child
	    );
	    pinDialog = null;
	  },
	  submit(pin) {
	    return new Promise((resolve, reject) => {
	      try {
	        submit(pin)
	          .then(resolve)
	          .catch(reject);
	      } catch (e) {
	        reject(e);
	      }
	    })
	  },
	  unlocked() {
	    return new Promise((resolve, reject) => {
	      try {
	        check()
	          .then(resolve)
	          .catch(reject);
	      } catch (e) {
	        reject(e);
	      }
	    })
	  },
	  locked() {
	    return new Promise((resolve, reject) => {
	      try {
	        check()
	          .then(unlocked => resolve(!!!unlocked))
	          .catch(reject);
	      } catch (e) {
	        reject(e);
	      }
	    })
	  },
	};

	var rx_escapable = /[\\\"\u0000-\u001f\u007f-\u009f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g;

	var gap,
	    indent,
	    meta = { // table of character substitutions
	      '\b': '\\b',
	      '\t': '\\t',
	      '\n': '\\n',
	      '\f': '\\f',
	      '\r': '\\r',
	      '"': '\\"',
	      '\\': '\\\\'
	    },
	    rep;

	function quote(string) {

	// If the string contains no control characters, no quote characters, and no
	// backslash characters, then we can safely slap some quotes around it.
	// Otherwise we must also replace the offending characters with safe escape
	// sequences.

	    rx_escapable.lastIndex = 0;
	    return rx_escapable.test(string)
	        ? '"' + string.replace(rx_escapable, function (a) {
	            var c = meta[a];
	            return typeof c === 'string'
	                ? c
	                : '\\u' + ('0000' + a.charCodeAt(0).toString(16)).slice(-4);
	        }) + '"'
	        : '"' + string + '"';
	}


	function str(key, holder, limit) {

	// Produce a string from holder[key].

	    var i,          // The loop counter.
	        k,          // The member key.
	        v,          // The member value.
	        length,
	        mind = gap,
	        partial,
	        value = holder[key];

	// If the value has a toJSON method, call it to obtain a replacement value.

	    if (value && typeof value === 'object' &&
	            typeof value.toJSON === 'function') {
	        value = value.toJSON(key);
	    }

	// If we were called with a replacer function, then call the replacer to
	// obtain a replacement value.

	    if (typeof rep === 'function') {
	        value = rep.call(holder, key, value);
	    }

	// What happens next depends on the value's type.

	    switch (typeof value) {
	    case 'string':
	        return quote(value);

	    case 'number':

	// JSON numbers must be finite. Encode non-finite numbers as null.

	        return isFinite(value)
	            ? String(value)
	            : 'null';

	    case 'boolean':
	    case 'null':

	// If the value is a boolean or null, convert it to a string. Note:
	// typeof null does not produce 'null'. The case is included here in
	// the remote chance that this gets fixed someday.

	        return String(value);

	// If the type is 'object', we might be dealing with an object or an array or
	// null.

	    case 'object':

	// Due to a specification blunder in ECMAScript, typeof null is 'object',
	// so watch out for that case.

	        if (!value) {
	            return 'null';
	        }

	// Make an array to hold the partial results of stringifying this object value.

	        gap += indent;
	        partial = [];

	// Is the value an array?

	        if (Object.prototype.toString.apply(value) === '[object Array]') {

	// The value is an array. Stringify every element. Use null as a placeholder
	// for non-JSON values.

	            length = value.length;
	            for (i = 0; i < length; i += 1) {
	                partial[i] = str(i, value, limit) || 'null';
	            }

	// Join all of the elements together, separated with commas, and wrap them in
	// brackets.

	            v = partial.length === 0
	                ? '[]'
	                : gap
	                    ? (
	                      gap.length + partial.join(', ').length + 4 > limit ?
	                      '[\n' + gap + partial.join(',\n' + gap) + '\n' + mind + ']' :
	                      '[ ' + partial.join(', ') + ' ]'
	                    )
	                    : '[' + partial.join(',') + ']';
	            gap = mind;
	            return v;
	        }

	// If the replacer is an array, use it to select the members to be stringified.

	        if (rep && typeof rep === 'object') {
	            length = rep.length;
	            for (i = 0; i < length; i += 1) {
	                if (typeof rep[i] === 'string') {
	                    k = rep[i];
	                    v = str(k, value, limit);
	                    if (v) {
	                        partial.push(quote(k) + (
	                            gap
	                                ? ': '
	                                : ':'
	                        ) + v);
	                    }
	                }
	            }
	        } else {

	// Otherwise, iterate through all of the keys in the object.

	            for (k in value) {
	                if (Object.prototype.hasOwnProperty.call(value, k)) {
	                    v = str(k, value, limit);
	                    if (v) {
	                        partial.push(quote(k) + (
	                            gap
	                                ? ': '
	                                : ':'
	                        ) + v);
	                    }
	                }
	            }
	        }

	// Join all of the member texts together, separated with commas,
	// and wrap them in braces.

	        v = partial.length === 0
	            ? '{}'
	            : gap
	                ? (
	                  gap.length + partial.join(', ').length + 4 > limit ?
	                  '{\n' + gap + partial.join(',\n' + gap) + '\n' + mind + '}' :
	                  '{ ' + partial.join(', ') + ' }'
	                )
	                : '{' + partial.join(',') + '}';
	        gap = mind;
	        return v;
	    }
	}


	function beautify (value, replacer, space, limit) {

	// The stringify method takes a value and an optional replacer, and an optional
	// space parameter, and returns a JSON text. The replacer can be a function
	// that can replace values, or an array of strings that will select the keys.
	// A default replacer method can be provided. Use of the space parameter can
	// produce text that is more easily readable.

	  var i;
	  gap = '';
	  indent = '';

	  if (!limit) limit = 0;

	  if (typeof limit !== "number")
	    throw new Error("beaufifier: limit must be a number");

	// If the space parameter is a number, make an indent string containing that
	// many spaces.

	  if (typeof space === 'number') {
	      for (i = 0; i < space; i += 1) {
	          indent += ' ';
	      }

	// If the space parameter is a string, it will be used as the indent string.

	  } else if (typeof space === 'string') {
	      indent = space;
	  }

	// If there is a replacer, it must be a function or an array.
	// Otherwise, throw an error.

	  rep = replacer;
	  if (replacer && typeof replacer !== 'function' &&
	          (typeof replacer !== 'object' ||
	          typeof replacer.length !== 'number')) {
	      throw new Error('beautifier: wrong replacer parameter');
	  }

	// Make a fake root object containing our value under the key of ''.
	// Return the result of stringifying the value.

	  return str('', {'': value}, limit);
	}

	var jsonBeautify = beautify;

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
	  const index = register$1.call(this, plugin, event, callback, errorCallback);
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
	const register$1 = function(plugin, event, callback, errorCallback) {
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
	var thunderJS$1 = options => {
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

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	class Events
	{
	  constructor(tt = null, pkdId)
	  {
	    if(tt == null)
	    {
	      throw "No Thunder !";
	    }

	    this.thunderJS = tt;
	    this.pkdId     = pkdId;
	    this.events    = [];
	  }

	  async add(plugin, event, cb = null)
	  {
	    if( (plugin == null || plugin == "") &&
	        (event  == null || event  == "") )
	    {
	      throw "No plugin/event !";
	    }

	    this.events.push( await this.handleEvent(plugin, event, cb) );
	  }

	  disposeAll()
	  {
	    // console.log( "EVENTS >>>   destroyAll() ")

	    this.events.map( ee => { ee.dispose(); } );
	  }

	  async handleEvent(plugin, event, cb = null)
	  {
	    // console.log('EVENTS >> Listen for >> ['+plugin+'] -> '+event+' ...');

	    if(cb != null)
	    {
	      // console.log('Listen for ['+name+'] using CALLBACK ...');
	      return await this.thunderJS.on(plugin, event, cb);
	    }
	    else
	    {
	      return await thunderJS.on(plugin, event, (notification) =>
	      {
	          var str = " " + event + " ...  Event" + JSON.stringify(notification);
	          console.log('Handler GOT >> ' + str);
	      })
	    }
	  }
	}//CLASS

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	class IconButton extends lng.Component
	{
	  static _template( )
	  {
	    let RR = lng.shaders.RoundedRectangle;

	    return {
	      flexItem: { margin: 40 },
	      Button:
	      {
	        mount: 0.5,
	        w: 45, h: 45,

	        RRect:  {
	          w: 45, h: 45, rect: true, color: 0x88888888, pivot: 0.5, alpha: 0.5, shader: { radius: 8, type: RR, stroke: 1, strokeColor: 0xFF333333 }
	        },
	        Image: {
	          mount: 0.50,
	          scale: 0.45,
	          x: (45 * 0.5 ),
	          y: (45 * 0.5 ),

	          // alpha: 0.5,
	          // shader:  { type: lng.shaders.Grayscale}
	        },
	      },
	      }
	    };

	  _focus()
	  {
	    var bb  = this.tag("Button");
	    var bg  = this.tag("RRect");
	    var clr = this.clrFocus;

	    bb.setSmooth('alpha', 1.00, {duration: 0.3});
	    bg.setSmooth('alpha', 1.00, {duration: 0.3});

	    bb.setSmooth('scale', 1.18, {duration: 0.3});
	    bg.setSmooth('color',  clr, {duration: 0.3});
	  }

	  _unfocus()
	  {
	    var bb  = this.tag("Button");
	    var bg  = this.tag("RRect");
	    var clr = this.clrBlur;

	   // bb.setSmooth('alpha', 0.50, {duration: 0.3});
	    bg.setSmooth('alpha', 0.50, {duration: 0.3});

	    bb.setSmooth('scale', 1.00, {duration: 0.3});
	    bg.setSmooth('color',  clr, {duration: 0.3});
	  }

	  _init()
	  {
	    if(this.clrFocus == undefined) this.clrFocus = 0xFFaaaaaa; // #aaaaaaFF
	    if(this.clrBlur  == undefined) this.clrBlur  = 0xFF888888; // #888888FF

	    // var button = this.tag("Button");

	    // button.w = this.w;
	    // button.h = this.h;

	   var image = this.tag("Image");

	   image.patch( {src: this.src1 } );
	  }

	  setClrFocus(clr)      { this.clrFocus = clr;};
	  setClrBlur(clr)       { this.clrBlur  = clr;};

	}//CLASS

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	class Progress extends lng.Component
	{
	    static _template( )
	    {
	      let RR = lng.shaders.RoundedRectangle;

	      var  barClr1  = 0xFFcccccc;  // #ccccccFF  // Background
	      var  frameClr = 0xFF666666;  // #666666FF

	    return {
	        ProgressBar: {
	          Background: { x: -2, y: 0, w: 4, h: 12, rtt: true, rect: true, color: frameClr, shader: { radius: 3, type: RR} },
	          Progress:   { x:  0, y: 2, w: 0, h:  8, rtt: true, rect: true, color: barClr1,  shader: { radius: 3, type: RR} },
	        }
	      }
	    };

	    getProgress()
	    {
	      return this.value;
	    }

	    reset()
	    {
	      this.value = 0;
	      this.tag("Progress").w = 0;
	    }

	    setProgress(pc)
	    {
	      this.value = pc;

	      var ww = (this.w -4) * pc;

	      this.tag("Progress").setSmooth('w', ww, {duration: 1});
	    }

	    _init()
	    {
	      this.tag("Background").w = this.w;
	      this.reset();
	    }
	  }//CLASS

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	class StoreTile extends lng.Component
	{
	    static _template( )
	    {
	      var pts = 20;
	      var frameClr = 0xFF666666;  // #666666FF
	      var textClr  = 0xFFffffff;  // #ffffffFF

	      var stroke    = 2;
	      var strokeClr = 0xFF444444;

	      let RR = { radius: 20, type: lng.shaders.RoundedRectangle, stroke: stroke, strokeColor: strokeClr};

	      return {
	        flexItem: { margin: 40 },
	        alpha: 1.0,
	        pivot: 0.5,
	        w: 210, h: 150,
	          Button:
	          {
	            scale: 0.0,
	            pivotY: 1.0,

	            RRect:
	            {
	                w: 210, h: 150, rtt: true, rect: true, pivot: 0.5, alpha: 1.0, color: frameClr, shader: RR,

	                Image: {
	                  mount: 0.5,
	                  src: Utils.asset('images/crate2_80x80.png'),
	                  x: (w => 0.45 * w),
	                  y: 150 * 0.40 //(h => 0.40 * h)
	                },

	                Label:
	                { mountX: 0.5, mountY: 1.0,
	                  x: (w => 0.5 * w),
	                  y: 150 - 5, //(h => h - 5),
	                  text: {  text: "Label 11", fontFace: 'Regular', fontSize: pts, textColor: textClr,

	                  shadow: true,
	                  shadowColor: 0xFF000000,
	                  shadowOffsetX: 2,
	                  shadowOffsetY: 2,
	                  shadowBlur: 8,
	                  },
	                },

	                Icon: {
	                  alpha: 1.0,
	                  mountX: 1.0,
	                  scale: 0.52,
	                  x: (w => w + 4),
	                  y: (h => 0)
	                },

	                Buttons:
	                {
	                  flex: {direction: 'row'},
	                  alpha: 0.5,
	                  mountX: 0.5,
	                  x: 210 / 2,
	                  y: 150,

	                  Button1: { btnId: "CHECK", flexItem: { margin: 30 }, type: IconButton, src1: Utils.asset('images/RDKcheckmark80x80.png')  },
	                  Button2: { btnId: "KILL",  flexItem: { margin: 30 }, type: IconButton, src1: Utils.asset('images/x_mark.png')           },
	                  Button3: { btnId: "TRASH", flexItem: { margin: 30 }, type: IconButton, src1: Utils.asset('images/TrashCan80x80.png')    },
	                }
	              }, //RRect

	              Progress: { type: Progress, mountX: 0.0, x: 0, y: 190, w: 210, h: 8, alpha: 0.0 },

	          },//Button
	      }
	    };

	    setLabel(s)
	    {
	      var obj = this.tag("Label");
	      obj.text.text = s;
	    }

	    setIcon(s)
	    {
	      var btn = this.tag("Button");
	      var icn = btn.tag("Icon");

	      icn.patch( {src: s } );
	    }

	    _focus()
	    {
	      var btn  = this.tag("Button");
	      var tile =  btn.tag("RRect");

	      // console.log("BUTTON: focus() >> pkgId: " + this._info.pkgId);

	      tile.setSmooth('scale',  1.15, {duration: 0.3});
	    }

	    _unfocus()
	    {
	      var btn  = this.tag("Button");
	      var tile =  btn.tag("RRect");

	      // console.log("BUTTON: unfocus() >> pkgId: " + this._info.pkgId);

	      tile.setSmooth('scale', 1.0, {duration: 0.3});
	    }

	    hide()
	    {
	      if(this.tag("Button").scale == 1.0)
	      {
	        const anim = this.tag('Button').animation({
	          duration: 0.5,
	          actions: [
	              { p: 'scale', v: { 0: 1, 0.5: 0.50, 1: 0.0 } },
	          ]
	        });
	        anim.start();
	      }
	    }

	    show(d = 0)
	    {
	      if(this.tag("Button").scale == 0.0)
	      {
	        const anim = this.tag('Button').animation({
	          duration: 0.5,
	          delay: d,
	          actions: [
	              { p: 'scale', v: { 0: 0, 0.5: 0.50, 1: 1.0 } },
	          ]
	        });
	        anim.start();
	      }
	    }

	    _init()
	    {
	      this.buttonIndex = 0;

	      this._enabled = true;
	      this.tag("Button").scale = 0;

	      if(this.w && this.h)
	      {
	        let button = this.tag("Button");

	        button.w = this.w;
	        button.h = this.h;
	      }

	      this.info = this.tileInfo;

	      this._setState('PassiveState');
	    }

	    set info( ii )
	    {
	      this.setInfo(ii);  // allow set 'null'
	    }

	    get info()
	    {
	      return this._info;
	    }

	    setInfo(ii)
	    {
	      if(ii)
	      {
	        if(ii.id)
	        {
	          ii.pkgId = ii.id;
	        }

	        if(ii.name)  this.setLabel(ii.name);
	        else
	        if(ii.label) this.setLabel(ii.label);
	        else
	        if(ii.id)    this.setLabel(ii.id);
	        else
	        if(ii.pkgId) this.setLabel(ii.pkgId);
	        else         this.setLabel("unknown22");
	      }

	      this._info = ii; // allow 'null'
	    }

	    isEnabled()
	    {
	      return this._enabled;
	    }

	    enable()
	    {
	      //console.log("APP TILE  >> enable() - " + this.info.pkgId)

	      this.tag("Button").setSmooth('alpha', 1.0, {duration: 0.3});
	      this._enabled = true;
	    }

	    disable()
	    {
	      //console.log("APP TILE  >> disable() - " + this.info.pkgId)

	      this.tag("Button").setSmooth('alpha', 0.5, {duration: 0.3});
	      this._enabled = false;
	    }

	    clickAnim()
	    {
	      var anim = this.tag('Button').animation({
	        duration: 0.35,
	        repeat: 1,
	        actions: [
	          {
	            t: '',
	            p: 'scale', v: { 0: 1.0, 0.5: 1.2, 1: 1.0},
	          },
	        ],
	      });

	      anim.start();
	    }

	    //////////////////////////////////////////////////////////////////////////////////////////////////
	  }//CLASS

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	class StoreTile$1 extends StoreTile
	{
	    static _template( )
	    {
	      var pts = 20;
	      var frameClr = 0xFF666666;  // #666666FF
	      var textClr  = 0xFFffffff;  // #ffffffFF

	      var stroke    = 2;
	      var strokeClr = 0xFF444444;

	      let RR = { radius: 20, type: lng.shaders.RoundedRectangle, stroke: stroke, strokeColor: strokeClr};

	      return {
	        flexItem: { margin: 40 },
	        alpha: 1.0,
	        Button:
	        {
	          scale: 0.0,

	          RRect:
	          {
	              w: 210, h: 150, rtt: true, rect: true, pivot: 0.5, alpha: 1.0, color: frameClr, shader: RR,

	              Image: {
	                mountX: 0.5,
	                mountY: 0.5,
	                src: Utils.asset('images/crate2_80x80.png'),
	                x: (w => 0.45 * w),
	                y: (h => 0.4 * h)
	              },

	              Label:
	              { mountX: 0.5, mountY: 1.0, x: (w => 0.5 * w), y: (h => h - 5),
	                text: {  text: "Label 11", fontFace: 'Regular', fontSize: pts, textColor: textClr,

	                shadow: true,
	                shadowColor: 0xFF000000,
	                shadowOffsetX: 2,
	                shadowOffsetY: 2,
	                shadowBlur: 8,
	                },
	              },

	              Icon: {
	                alpha: 1.0,
	                mountX: 1.0,
	                scale: 0.52,
	                x: (w => w + 4),
	                y: (h => 0)
	              },
	            },

	            Progress: { type: Progress, mountX: 0.0, x: 0, y: 190, w: 210, h: 8, alpha: 0.0 },
	        }//Button
	      }
	    };

	    _init()
	    {
	      this._enabled = true;
	      this.tag("Button").scale = 0;

	      if(this.w && this.h)
	      {
	        let button = this.tag("Button");

	        button.w = this.w;
	        button.h = this.h;
	      }

	      this.info = this.tileInfo;

	      var download_PNG = Utils.asset('images/download3.png');
	      this.setIcon(download_PNG);
	    }

	    setInstalled(v)
	    {
	      if(this._info)
	      {
	        this._info.pkgInstalled = v;
	      }
	    }

	    isInstalled()
	    {
	      return this._info.pkgInstalled;
	    }

	    clickAnim()
	    {
	      var anim = this.tag('Button').animation({
	        duration: 0.35,
	        repeat: 1,
	        actions: [
	          {
	            t: '',
	            p: 'scale', v: { 0: 1.0, 0.5: 1.2, 1: 1.0},
	          },
	        ],
	      });

	      anim.start();
	    }

	  }//CLASS

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	class StoreTile$2 extends StoreTile
	{
	    static _template( )
	    {
	      var pts = 20;
	      var frameClr = 0xFF666666;  // #666666FF
	      var textClr  = 0xFFffffff;  // #ffffffFF

	      var stroke    = 2;
	      var strokeClr = 0xFF444444;

	      let RR = { radius: 20, type: lng.shaders.RoundedRectangle, stroke: stroke, strokeColor: strokeClr};

	      return {
	        flexItem: { margin: 40 },
	        alpha: 1.0,
	        pivot: 0.5,
	        w: 210, h: 150,
	          Button:
	          {
	            scale: 0.0,
	            pivotY: 1.0,

	            RRect:
	            {
	                w: 210, h: 150, rtt: true, rect: true, pivot: 0.5, alpha: 1.0, color: frameClr, shader: RR,

	                Image: {
	                  mount: 0.5,
	                  src: Utils.asset('images/crate2_80x80.png'),
	                  x: (w => 0.45 * w),
	                  y: 150 * 0.40 //(h => 0.40 * h)
	                },

	                Label:
	                { mountX: 0.5, mountY: 1.0,
	                  x: (w => 0.5 * w),
	                  y: 150 - 5, //(h => h - 5),
	                  text: {  text: "Label 11", fontFace: 'Regular', fontSize: pts, textColor: textClr,

	                  shadow: true,
	                  shadowColor: 0xFF000000,
	                  shadowOffsetX: 2,
	                  shadowOffsetY: 2,
	                  shadowBlur: 8,
	                  },
	                },

	                Icon: {
	                  alpha: 1.0,
	                  mountX: 1.0,
	                  scale: 0.52,
	                  x: (w => w + 4),
	                  y: (h => 0)
	                },

	                Buttons:
	                {
	                  flex: {direction: 'row'},
	                  alpha: 0.5,
	                  mountX: 0.5,
	                  x: 210 / 2,
	                  y: 150,

	                  Button1: { btnId: "RESUME", flexItem: { margin: 30 }, type: IconButton, src1: Utils.asset('images/RDKcheckmark80x80.png')  },
	                  Button2: { btnId: "KILL",   flexItem: { margin: 30 }, type: IconButton, src1: Utils.asset('images/x_mark.png')           },
	                  Button3: { btnId: "TRASH",  flexItem: { margin: 30 }, type: IconButton, src1: Utils.asset('images/TrashCan80x80.png')    },
	                }
	              }, //RRect

	              Progress: { type: Progress, mountX: 0.0, x: 0, y: 190, w: 210, h: 8, alpha: 0.0 },

	          },//Button
	      }
	    };

	    setSuspended()
	    {
	      var btn = this.tag("Button");
	      var icn = btn.tag("Icon");

	      icn.patch( {src: Utils.asset('images/sleep80x80.png')  } );

	      this.startSnooze();
	    }

	    stopSuspended()
	    {
	      this.stopSnooze();
	    }

	    startSnooze()
	    {
	        this.tag('Icon').setSmooth('alpha', 1.0, {duration: 0.3});

	        this.snoozeAnim = this.tag('Icon').animation({
	            duration: 1.75,
	            repeat: -1,
	            actions: [
	              {
	                t: '',
	                repeat: -1,
	                p: 'scale',
	                v: { 0.00: 0.30, 0.25: 0.60, 0.50: 0.65, 0.75: 0.60, 1.00: 0.30 },
	              },
	            ],
	          });

	        this.snoozeAnim.start();
	    }

	    stopSnooze()
	    {
	      if(this.snoozeAnim == null)
	      {
	        console.log("stopSnooze() ... Already stopped.  Null");
	        return;
	      }

	      this.snoozeAnim.on('stop', ()=>
	      {
	        this.tag('Icon').setSmooth('alpha', 0.0, {duration: 1.73});

	        this.snoozeAnim = null;
	      });

	      this.snoozeAnim.stop();
	    }

	    _init()
	    {
	      this.buttonIndex = 0;

	      this._enabled = true;
	      this.tag("Button").scale = 0;

	      if(this.w && this.h)
	      {
	        let button = this.tag("Button");

	        button.w = this.w;
	        button.h = this.h;
	      }

	      this.info = this.tileInfo;

	      this._setState('PassiveState');
	    }

	    startWiggle()
	    {
	        this.wiggleAnim = this.tag('Image').animation({
	            duration: 0.11*3,
	            repeat: -1,
	            actions: [
	              {
	                t: '',
	                repeat: -1,
	                p: 'rotation', v: { 0: 0.0, 0.25: -0.14, 0.5: 0.0, 0.75: 0.14, 1: 0.0},
	              },
	            ],
	          });

	        this.wiggleAnim.start();
	    }

	    stopWiggle()
	    {
	      if(this.wiggleAnim)
	      {
	        this.wiggleAnim.stop();
	        this.wiggleAnim = null;
	      }
	    }

	    setInstalled(v)
	    {
	      if(this._info)
	      {
	        this._info.pkgInstalled = v;
	      }
	    }

	    isInstalled()
	    {
	      return this._info.pkgInstalled;
	    }

	    clickAnim()
	    {
	      var anim = this.tag('Button').animation({
	        duration: 0.35,
	        repeat: 1,
	        actions: [
	          {
	            t: '',
	            p: 'scale', v: { 0: 1.0, 0.5: 1.2, 1: 1.0},
	          },
	        ],
	      });

	      anim.start();
	    }

	    //////////////////////////////////////////////////////////////////////////////////////////////////
	    //////////////////////////////////////////////////////////////////////////////////////////////////
	    //////////////////////////////////////////////////////////////////////////////////////////////////

	    static _states(){
	      return [
	            class PassiveState extends this
	            {
	              $enter()
	              {
	                // console.log(">>>>>>>>>>>>   STATE:  PassiveState");

	                var btn  = this.tag("Button");
	                var tile =  btn.tag("RRect");

	                tile.setSmooth('h', 150 , {duration: 0.3});

	                this._setState('PassiveState');
	              }

	              _handleDown()
	              {
	                this._setState('AppStateButtons');
	              }
	            },
	            class AppStateButtons extends this
	            {
	              $enter()
	              {
	                console.log(">>>>>>>>>>>>   STATE:  AppStateButtons");

	                var btn  = this.tag("Button");
	                var tile =  btn.tag("RRect");

	                tile.setSmooth('h', 210 , {duration: 0.3});
	              }

	              _handleUp()
	              {
	                this._setState('PassiveState');
	              }

	              _handleLeft()
	              {
	                // console.log(">>>>>>>>>>>>   LEFT:  this.buttonIndex: " + this.buttonIndex);

	                if(--this.buttonIndex < 0) this.buttonIndex = 0;
	              }

	              _handleRight()
	              {
	                var btns = this.tag("Buttons");

	                // console.log(">>>>>>>>>>>>   RIGHT:  this.buttonIndex: " + this.buttonIndex);
	                if(++this.buttonIndex >= btns.children.length) this.buttonIndex = btns.children.length - 1;
	              }

	              _handleEnter()
	              {
	                console.log(">>>>>>>>>>>>  _handleEnter()  " + this.buttonIndex );

	                var btns = this.tag("Buttons");
	                var button = btns.children[this.buttonIndex];

	                var fireThis = '$fire' + button.btnId;

	                button.fireAncestors(fireThis);

	                if(button.btnId == 'RESUME')
	                {
	                  this.setSuspended();
	                }
	                else
	                if(button.btnId == 'KILL')
	                {
	                  this.stopSuspended();
	                }
	              }

	              _getFocused()
	              {
	                // console.log(">>>>>>>>>>>>  _getFocused()  ");

	                var btns  = this.tag("Buttons");
	                return btns.children[this.buttonIndex]
	              }
	            },
	        ]
	      };
	  }//CLASS

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	class AppList extends lng.Component
	{
	    static _template() {
	      return {
	          flex: {direction: 'row', padding: 20, paddingTop: 50, wrap: false },
	          children:[]
	      }
	    }

	    _getFocused()
	    {
	      console.log('LIST >>> _getFocused() ... ENTER');
	      return this.children[0]//this.storeButtonIndex]
	    }

	    addTile(n, info)
	    {
	      // console.log('addTile() ... ENTER')

	      // console.log("LIST  addTile( n: "+n+",  info:  " + JSON.stringify(info, 2, null) )
	      // console.log("LIST  addTile( )    ... this.children.length " + this.children.length  )

	      this.children[n].setInfo( info );
	      this.children[n].show();
	    }

	    set storeTiles( list )
	    {
	        // console.log("SETTING >>> tiles: " + JSON.stringify(list, 2, null) )
	        this.children = list.map((tileInfo, index) =>
	        {
	            return {
	              w: 210, h: 150,
	              type: StoreTile$1,
	              tileInfo
	            }
	        });
	    }


	    set appTiles( list )
	    {
	        // console.log("SETTING >>> tiles: " + JSON.stringify(list, 2, null) )
	        this.children = list.map((tileInfo, index) =>
	        {
	            return {
	              w: 210, h: 150,
	              type: StoreTile$2,
	              tileInfo
	            }
	        });
	    }
	  }//CLASS

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	class Button extends lng.Component
	{
	  static _template( )
	  {
	    let RR = lng.shaders.RoundedRectangle;

	    return {
	      Button:
	      {
	        RRect: { w: 150, h: 40, rect: true, color: 0xFF888888, shader: { radius: 8, type: RR, stroke: 1, strokeColor: 0xFF333333 } },
	        Label: { mount: 0.5, x: (w => 0.5 * w), y: (h => 0.55 * h), text:{ text: '(unset)', fontSize: 20,  textColor: 0xFFffffff } },
	      },
	      }
	    };

	  setLabel(s)
	  {
	    var obj = this.tag("Label");
	    obj.text.text = s;
	  }

	  _focus()
	  {
	    var bb  = this.tag("Button");
	    var bg  = this.tag("RRect");
	    var clr = this.clrFocus;

	    bb.setSmooth('alpha', 1.00, {duration: 0.3});
	    bb.setSmooth('scale', 1.18, {duration: 0.3});
	    bg.setSmooth('color',  clr, {duration: 0.3});
	  }

	  _unfocus()
	  {
	    var bb  = this.tag("Button");
	    var bg  = this.tag("RRect");
	    var clr = this.clrBlur;

	    bb.setSmooth('alpha', 1.00, {duration: 0.3});
	    bb.setSmooth('scale', 1.00, {duration: 0.3});
	    bg.setSmooth('color',  clr, {duration: 0.3});
	  }

	  _init()
	  {
	    this.pts       = 20;
	    this.stroke    = 2;
	    this.strokeClr = 0xCC888888;

	    if(this.clrFrame == undefined) this.clrFrame = 0xFF666666; // #666666FF
	    if(this.clrText  == undefined) this.clrText  = 0xFFffffff; // #ffffffFF

	    if(this.clrFocus == undefined) this.clrFocus = 0xFFaaaaaa; // #aaaaaaFF
	    if(this.clrBlur  == undefined) this.clrBlur  = 0xFF888888; // #888888FF

	    var button = this.tag("Button");

	    button.w = this.w;
	    button.h = this.h;

	    if(this.label)
	    {
	      this.setLabel(this.label);
	    }
	  }

	  setClrBackground(clr) { this.clrBackground     = clr;};
	  setClrFocus(clr)      { this.clrFocus          = clr;};
	  setClrBlur(clr)       { this.clrBlur           = clr;};
	  setLabel(v)           { this.tag("Label").text = v; };

	}//CLASS

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	class OkCancel extends lng.Component
	{
	    static _template( )
	    {
	      let RR = lng.shaders.RoundedRectangle;

	      var  pts = 25;
	     // var buttonClr = 0xFF888888;  // #888888FF  // Background
	      var frameClr  = 0xFFaaaaaa;  // #aaaaaaFF
	      var textClr   = 0xFFffffff;  // #ffffffFF

	      var stroke    = 2;
	      var strokeClr = 0xFF444444;

	      return {
	        Dialog:
	        {
	            mount: 0.5, w: 600, h: 120, rtt: true, rect: true, color: frameClr, shader: { radius: 20, type: RR, stroke: stroke, strokeColor: strokeClr},

	            Label:
	            {
	              mountX: 0.5, mountY: 0.0, x: (w => 0.5 * w), y:  (h => 0.15 * h),
	              text: {  text: "Remove app ?", fontFace: 'Regular', fontSize: pts, textColor: textClr,
	                        shadow: true,
	                        shadowColor: 0xff444444,
	                        shadowOffsetX: 1,
	                        shadowOffsetY: 1,
	                        shadowBlur: 2,
	              },
	            },

	            OkButton:    { type: Button, label: "Ok",     clrFocus: 0xFF008800, mount: 0.5, x: (w => 0.25 * w), y: (h => h - 35), w: 150, h: 40 },
	            CancelButton:{ type: Button, label: "Cancel", clrFocus: 0xFF880000, mount: 0.5, x: (w => 0.75 * w), y: (h => h - 35), w: 150, h: 40 },
	        }//Dialog
	      }
	    };

	    setLabel(s)
	    {
	      this.tag("Label").text = s;
	    }

	    _init()
	    {
	      this.buttons = [
	                        this.tag("OkButton"),
	                        this.tag("CancelButton")
	                      ];

	      this.buttonIndex = 1;

	      this.tag("Dialog").w = this.w;
	      this.tag("Dialog").h = this.h;
	    }

	    static _states(){
	      return [
	            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	            class OKCState extends this
	            {
	              $enter()
	              {
	                this.tag("CancelButton").setFocus = true;
	              }

	              _handleLeft()
	              {
	                this.buttonIndex = 0;
	              }

	              _handleRight()
	              {
	                this.buttonIndex = 1;
	              }

	              handleEsc()
	              {
	                this.fireAncestors('$onRemoveCANCEL', this.pkgId, false);
	              }

	              _handleEnter() // could be OK or CANCEL button
	              {
	                var name = (this.buttonIndex == 0) ? '$onRemoveOK' : '$onRemoveCANCEL';

	                this.fireAncestors(name);
	              }

	              _handleKey(k)
	              {
	                switch( k.keyCode )
	                {
	                  case 27: this.handleEsc(); break; // ESC key
	                  default:
	                    console.log("OKC ... GOT key code: " + k.keyCode);
	                      break;
	                }

	                return true;
	              }

	              _getFocused()
	              {
	                return this.buttons[this.buttonIndex]
	              }
	            }
	        ]}//_states
	  }//CLASS

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	const DefaultApps =
	[
	  {
	    "pkgId": "TestApp1",
	    "type": "non-OCI",
	    "url": "http://10.0.2.15/testApp1.json",
	    "token": "TODO: Security",
	    "listener": "TODO: url or endpoint of event listener provided by caller"
	  },

	  {
	    "pkgId": "TestApp2",
	    "type": "non-OCI",
	    "url": "http://10.0.2.15/testApp2.tgz",
	    "token": "TODO: Security",
	    "listener": "TODO: url or endpoint of event listener provided by caller"
	  },

	  {
	    "pkgId": "TestApp3",
	    "type": "non-OCI",
	    "url": "http://10.0.2.15/testApp3.tgz",
	    "token": "TODO: Security",
	    "listener": "TODO: url or endpoint of event listener provided by caller"
	  }
	];

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	const HOME_KEY      = 77;
	const LIGHTNING_APP = "lightningapp";

	var AvailableApps   = [];
	var InstalledApps   = [];
	var InstalledAppMap = {};

	const thunder_cfg = {
	  host: '127.0.0.1',
	  port: 9999,
	  debug: false, // VERY USEFUL
	  versions: {
	    default: 1, // use version 5 if plugin not specified
	    Controller: 1,
	    Packager: 1,
	    // etc ..
	  }
	};

	var thunderJS$2 = null;

	class App extends Lightning.Component
	{
	  static getFonts() {
	    return [{ family: 'Regular', url: Utils.asset('fonts/Roboto-Regular.ttf') }]
	  }

	  static _template()
	  {
	    let RR = Lightning.shaders.RoundedRectangle;

	    var ui =
	    {
	      Blind1: {
	        x: 0, y: 0, w: 1920, h: 1080/2, rect: true, color: 0xff000000, zIndex: 998,

	        // Bg: {mountX: 0.5,
	        //   x: 1920/2, y: 1080/2, w: 600, h: 79, rect: true, color: 0xff000000, zIndex: 998,
	        // },

	        RDKlogo: {
	          mount: 0.5,
	          x: 1920/2,
	          y: 1080/2,
	          zIndex: 999,
	          src: Utils.asset('images/RDKLogo400x79.png'),
	        },
	      },
	      Blind2: {
	        x: 0, y: 1080/2, w: 1920, h: 1080/2, rect: true, color: 0xff000000, zIndex: 997
	      },


	      Background: {
	        w: 1920,
	        h: 1080,
	        color: 0xff8888aa,
	        src: Utils.asset('images/background.png'),
	      },

	      Title: {
	        mountX: 0.5,
	        mountY: 0,
	        x: 1920/2,
	        y: 20,
	        text: {
	          text: "Demo Store",
	          fontFace: 'Regular',
	          fontSize: 70,
	          textColor: 0xFFffffff,

	          shadow: true,
	          shadowColor: 0xff000000,
	          shadowOffsetX: 2,
	          shadowOffsetY: 2,
	          shadowBlur: 8,
	        },
	      },

	      Lists:
	      {
	        mountX: 0.5, x: 1920/2, y: 150, w: 1450, h: 900,
	        flex: {direction: 'column', alignItems: 'center'},
	      //	rect: false, //rtt: true, shader: { radius: 20, type: RR}, color: 0x4F888888,
	        // rect: true, color: 0x88ccccFF,

	        HelpText:
	        {
	          // rect: true, color: 0xff00ff00,

	          flex: {direction: 'row'},

	          HelpBox1:
	          {
	            w: 1450/2,
	            h: 10,
	            // rect: true, color: 0xffFF0000,

	            HelpTip1:
	            {
	              text:
	              {
	                w: 1450/2,
	                text: "Use  (A)ll or (I)nfo for package metadata",
	                textAlign: 'center',
	                fontFace: 'Regular',
	                fontSize: 16,
	                textColor: 0xFFffffff,

	                shadow: true,
	                shadowColor: 0xff000000,
	                shadowOffsetX: 2,
	                shadowOffsetY: 2,
	                shadowBlur: 8,
	              },
	            },
	          },//Box
	          HelpBox2:
	          {
	            w: 1450/2,
	            h: 10,
	            // rect: true, color: 0x88FF00FF,

	            HelpTip2:
	            {
	              text:
	              {
	                w: 1450/2,
	                // h: 50,
	                text: "Use  UP/DN  arrow keys for Console",
	                textAlign: 'center',
	                fontFace: 'Regular',
	                fontSize: 16,
	                textColor: 0xFFffffff,

	                shadow: true,
	                shadowColor: 0xff000000,
	                shadowOffsetX: 2,
	                shadowOffsetY: 2,
	                shadowBlur: 8,
	              },
	            },
	          },//Box
	        }, // HelpText

	        AvailableTXT: {
	          // rect: true, color: 0x88FF00FF,
	          x: 0,
	          y: 60,
	          w: 1450,
	          text: {
	            text: "AVAILABLE: ",

	            // highlight: true,
	            // highlightColor: 0xFF0000ff,

	            // textAlign: 'left',
	            fontFace: 'Regular',
	            fontSize: 30,
	            textColor: 0xFFffffff,

	            shadow: true,
	            shadowColor: 0xff000000,
	            shadowOffsetX: 2,
	            shadowOffsetY: 2,
	            shadowBlur: 8,
	          },
	        },

	        AvailableGroup:
	        {
	          mountX: 0.5, x: 1450/2, y: 0, w: 1450, h: 300, flex: {direction: 'row', padding: 15, wrap: false }, rect: true, rtt: true, shader: { radius: 20, type: RR}, color: 0x4F888888,

	          // Available PACKAGES from inventory ... injected here
	          AvailableList: { x: 0, type: AppList }
	        },


	        InstalledTXT: {
	          // rect: true, color: 0x88FF00FF,
	          x: 0,
	          y: 160,
	          w: 1450,
	          text: {
	            text: "INSTALLED: ",

	            // highlight: true,
	            // highlightColor: 0xFF0000ff,

	            // textAlign: 'left',
	            fontFace: 'Regular',
	            fontSize: 30,
	            textColor: 0xFFffffff,

	            shadow: true,
	            shadowColor: 0xff000000,
	            shadowOffsetX: 2,
	            shadowOffsetY: 2,
	            shadowBlur: 8,
	          },
	        },
	        InstalledGroup:
	        {
	          mountX: 0.5, x: 1450/2, y: 100, w: 1450, h: 300, flex: {direction: 'row', padding: 15, wrap: true}, rect: true, rtt: true, shader: { radius: 20, type: RR}, color: 0x4F888888,

	          InstalledList: { type: AppList }

	        }, // InstalledGroup
	      },//Lists

	      SpaceLeft:
	      {
	        x: 1400, y: 600,
	        text: {
	          text: "Space: 0 Kb",
	          textAlign: 'right',
	          fontFace: 'Regular',
	          fontSize: 22,
	          textColor: 0xaa00FF00,

	          shadow: true,
	          shadowColor: 0xFF000000,
	          shadowOffsetX: 2,
	          shadowOffsetY: 2,
	          shadowBlur: 8,
	        },

	      },

	      ConsoleBG:
	      {
	        mountX: 0.5,
	        x: 1920/2, y: 150, w: 1450,
	        h: 600, rect: true,
	        alpha: 0.0, shader: { radius: 20, type: RR },
	        color: 0xcc222222, // #222222ee
	        // colorTop: 0xFF636EFB, colorBottom: 0xFF1C27bC,

	        Console: {

	          x: 10, y: 10,
	          w: 1450,
	          //h: 500,
	          text: {
	            fontFace: 'Regular',
	            fontSize: 18,
	            textColor: 0xFFffffff,
	          },
	        },
	      }, // ConsoleBG

	      OkCancel: { type: OkCancel, x: 1920/2, y: 400, w: 600, h: 180, alpha: 0.0 },

	      // LineH: { mountY: 0.5, x: 0, y: 1080/2, w: 1920, h: 2, rect: true, color: 0xff00FF00 },
	      // LineV: { mountX: 0.5, y: 0, x: 1920/2, h: 1080, w: 2, rect: true, color: 0xff00FF00 },
	    };

	    return ui;
	  }

	  setConsole(str)
	  {
	    this.tag('Console').text.text = str;
	  }


	  $fireRESUME(pkgId)
	  {
	    // console.log(">>>>>>>>>>>>  fireRESUME() ");

	    let info = InstalledApps[this.installedButtonIndex];

	    // console.log(">>>>>>>>>>>>  fireRESUME() info: "+ beautify(info, null, 2, 100) );

	    if(info != null)
	    {
	      this.resumePkg(info.pkgId, info);
	    }
	  }

	  $fireKILL()
	  {
	    // console.log(">>>>>>>>>>>>  fireKILL() ");

	    let info = InstalledApps[this.installedButtonIndex];

	    // console.log(">>>>>>>>>>>>  fireKILL() info: "+ beautify(info, null, 2, 100) );

	    if(info != null)
	    {
	      this.killPkg(info.pkgId, info);
	    }
	  }

	  $fireTRASH(pkgId)
	  {
	    let info = InstalledApps[this.installedButtonIndex];

	    // console.log(">>>>>>>>>>>>  fireTRASH() info: "+ beautify(info, null, 2, 100) );

	    this._setState('OKCStateEnter');
	  }

	  findInstalledButton(pkgId)
	  {
	    var bb = this.tag('InstalledList').children.filter( (o) =>
	    {
	      if(o.info)
	      {
	        return o.info.pkgId == pkgId;
	      }
	      else
	      {
	        return false;
	      }
	    });

	    return bb.length == 0 ? null : bb[0];
	  }

	  findStoreButton(pkgId)
	  {
	    var bb = this.tag('AvailableList').children.filter( (o) =>
	    {
	      if(o.info)
	      {
	        return o.info.pkgId == pkgId;
	      }
	      else
	      {
	        return false;
	      }
	    });

	    return bb.length == 0 ? null : bb[0];
	  }

	  $onRemoveOK() // 'okButton = true' indicates the OK button was clicked
	  {
	    var dlg = this.tag("OkCancel");
	    var pkgId = dlg.pkgId;

	    console.log("onRemoveOK ENTER - ... pkgId: " + pkgId);

	    if(pkgId == undefined)
	    {
	      console.log("onRemoveOK() >>>  ERROR - ... pkgId: " + pkgId);
	      return;
	    }

	    let info = InstalledAppMap[pkgId];
	    if(info.appState == "SUSPENDED")
	    {
	      this.killPkg(pkgId);
	    }

	    // this.removePkg(pkgId);

	    dlg.setSmooth('alpha', 0, {duration: 0.3}); // HIDE

	    // Enable STORE button - as it's UNINSTALLED
	    let removeMe = this.tag('InstalledList').children[this.installedButtonIndex];
	    removeMe.stopWiggle();

	    //removeMe.hide();

	    if(removeMe.tag("Button").scale == 1.0)
	    {
	      const anim = removeMe.tag('Button').animation({
	        duration: 0.5,
	        actions: [
	            { p: 'scale', v: { 0: 1, 0.5: 0.50, 1: 0.0 } },
	        ]
	      });
	      anim.start();

	      anim.on('finish', () =>
	      {
	        this.tag('InstalledList').childList.remove(removeMe);
	        this.tag('InstalledList').childList.add(removeMe); // move to end

	        this.removePkg(pkgId);
	      });
	    }

	    var storeButton = this.findStoreButton(pkgId);
	    if(storeButton != null)
	    {
	      storeButton.enable();
	    }

	    this._setState('InstalledRowState');
	}

	  $onRemoveCANCEL()
	  {
	    var dlg = this.tag("OkCancel");

	    // console.log("onRemoveCANCEL ENTER - ... pkgId: " + pkgId);

	    dlg.setSmooth('alpha', 0, {duration: 0.3}); // HIDE

	    // console.log("onRemoveCANCEL ENTER - ... info: " + pkgId)

	    var dontRemoveMe = this.tag('InstalledList').children[this.installedButtonIndex];
	    dontRemoveMe.stopWiggle();

	    this._setState('InstalledRowState');
	  }

	  $fireINSTALL(pkgId)
	  {
	    // console.log("INSTALL >>  fireINSTALL() - ENTER .. pkgId: " + pkgId);

	    let button = this.tag('AvailableList').children[this.storeButtonIndex];

	    this.isInstalled(pkgId).then( (ans) =>
	    {
	      if( ans['available'] == false)
	      {
	        var progress = button.tag("Progress");

	        progress.reset(); // reset
	        progress.setSmooth('alpha', 1, {duration: .1});

	        var info = button.info;

	        this.installPkg(pkgId, info);
	        info.appState = "STOPPED";
	      }
	      else
	      {
	        console.log("CALL >> this.installPkg() ALREADY have ... pkgId: " + pkgId);
	      }
	    });
	  }

	  $LaunchClicked(pkgId)
	  {
	    // console.log("$LaunchClicked() >>>  ENTER - ... pkgId: " + pkgId)

	    let info = InstalledAppMap[pkgId];
	    if(info)
	    {
	      if(info.appState == "STOPPED" || info.appState == undefined)
	      {
	        this.launchPkg(pkgId, info);

	        this.setConsole("Launched: " + jsonBeautify(info, null, 2, 100) );

	      }
	      else if(info.appState == "SUSPENDED")
	      {
	        this.resumePkg(pkgId, info);

	        this.setConsole("Resumed: " + jsonBeautify(info, null, 2, 100) );
	      }
	      else
	      {
	        console.log("$LaunchClicked() >>> Error:  invalid app state: " + info);
	      }
	    }
	    else
	    {
	      console.log("$LaunchClicked() >>> Error:  NO  info: " + info);
	    }
	  }

	  async getAvailableSpace()
	  {
	    try
	    {
	      var result = await thunderJS$2.call('Packager', 'getAvailableSpace', null);

	      this.tag('SpaceLeft').text.text = ("Space Remaining: " + result.availableSpaceInKB + " Kb");

	      //this.setConsole( beautify(result, null, 2, 100) )
	    }
	    catch(e)
	    {
	      this.setConsole( 'getAvailableSpace() >>> CAUGHT:  e: ' + jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async getPackageInfo(pkgId)
	  {
	    try
	    {
	      let params = { "pkgId": pkgId };

	      var result = await thunderJS$2.call('Packager', 'getPackageInfo', params);

	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      this.setConsole( 'getPackageInfo() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async getInstalled()
	  {
	    // console.log("getInstalled() - ENTER ")

	    try
	    {
	      var result = await thunderJS$2.call('Packager', 'getInstalled', null);

	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      this.setConsole( 'getInstalled() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      return;
	    }

	    this.getAvailableSpace();

	    // Preserve App States ...

	    for( var i = 0; i < result.applications.length; i++)
	    {
	      let app   = result.applications[i];
	      let match = InstalledApps.filter( o => o.pkgId == app.id );

	      if(match.length > 0)
	      {
	        // NOTE: If objects have a property with the same name,
	        //       then the right-most object property overwrites the previous one.
	        //
	        var merged = { ...match[0], ...app }; // prefer new 'info' from getInstalled()

	        result.applications[i] = merged;
	      }
	    }

	    // Update Apps ...
	    InstalledAppMap = {};    // reset
	    InstalledApps   = null; // reset

	    //
	    // NOTE:  getInstalled() returns meta with 'id' -NOT- 'pkgId'
	    //
	    result.applications.map( (o) => InstalledAppMap[o.id] = o ); // populate info

	    InstalledApps = result.applications; // update INSTALLED array

	    //
	    // APP STORE >>> DISABLE apps that are already installed...
	    //
	    InstalledApps.map( have =>
	    {
	      let disable = AvailableApps.filter( o => o.pkgId == have.id );

	      var storeButton = this.findStoreButton(disable[0].pkgId);
	      if(storeButton != null)
	      {
	        storeButton.disable(); // DISABLE
	      }
	    });

	    // SHOW / HIDE tiles per installations
	    this.tag("InstalledList").children.map( (button, i) =>
	    {
	      if(i < InstalledApps.length)
	      {
	        InstalledApps[i].pkgInstalled = true;

	        button.info = InstalledApps[i];
	        button.show(i * 0.15);
	      }
	      else
	      {
	        button.info = null; //  RESET
	        // button.hide();
	      }
	    });

	    if(InstalledApps.length == 0)
	    {
	      this._setState('StoreRowState'); // No apps installed >>> BACK TO STORE 
	    }
	  }

	  async isInstalled(pkgId)
	  {
	    try
	    {
	      let params = { "pkgId": pkgId };

	      let result = await thunderJS$2.call('Packager', 'isInstalled', params);

	//      console.log( 'DEBUG:  IsInstalled  ' + beautify(result, null, 2, 100) )
	//      this.setConsole(     'IsInstalled  ' + beautify(result, null, 2, 100) )

	      return result;
	    }
	    catch(e)
	    {
	      console.log('DEBUG:  isInstalled() >>> CAUGHT:  e: ' + jsonBeautify(e, null, 2, 100) );
	      this.setConsole(    'isInstalled() >>> CAUGHT:  e: ' + jsonBeautify(e, null, 2, 100) );
	      return false;
	    }
	  }

	  async addKeyIntercept()  // v1
	  {
	    let params =
	    {
	      "keyCode": HOME_KEY,
	      "modifiers": ["ctrl"],
	      "client": LIGHTNING_APP
	    };

	    try
	    {
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'addKeyIntercept', params);
	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      console.log(     'addKeyIntercept() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      this.setConsole( 'addKeyIntercept() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async removeKeyIntercept() // v1
	  {
	    let params =
	    {
	      "client": LIGHTNING_APP
	    };

	    try
	    {
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'removeKeyIntercept', params);

	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      console.log(     'removeKeyIntercept() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      this.setConsole( 'removeKeyIntercept() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async setFocus(pkgId)
	  {
	    let params =
	    {
	        "client": pkgId
	    };

	    try
	    {
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'setFocus', params);

	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      console.log(     'setFocus() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      this.setConsole( 'setFocus() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async moveToFront(pkgId)
	  {
	    let params =
	    {
	        "client": pkgId
	    };

	    try
	    {
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'moveToFront', params);

	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      console.log(     'moveToFront() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      this.setConsole( 'moveToFront() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async moveToBack(pkgId)
	  {
	    let params =
	    {
	        "client": pkgId
	    };

	    try
	    {
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'moveToBack', params);
	      console.log(jsonBeautify(result, null, 2, 100));
	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      console.log(     'moveToBack() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      this.setConsole( 'moveToBack() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async suspendPkg(pkgId, info)
	  {
	    let params =
	    {
	        "client": pkgId
	    };

	    try
	    {
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'suspendApplication', params);

	      console.log(jsonBeautify(result, null, 2, 100));
	      this.setConsole( jsonBeautify(result, null, 2, 100) );

	      if(result.success)
	      {
	        this.moveToBack(pkgId);
	        this.setFocus(LIGHTNING_APP);

	        info.appState      = "SUSPENDED";
	        this.launchedPkgId = "";

	        // TODO: APP BUTTON - setSuspended()
	        var appButton = this.findInstalledButton(pkgId);
	        if(appButton != null)
	        {
	          appButton.setSuspended();
	        }
	        else
	        {
	          console.log("suspendPkg() >>> Cannot find App Button for pkgId: " + pkgId);
	        }
	      }
	      else
	      {
	        console.log( 'suspendPkg() failed!');
	      }
	    }
	    catch(e)
	    {
	      console.log(     'suspendPkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      this.setConsole( 'suspendPkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async resumePkg(pkgId, info)
	  {
	    let params =
	    {
	        "client": pkgId
	    };

	    try
	    {
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'resumeApplication', params);

	      console.log( jsonBeautify(result, null, 2, 100) );
	      this.setConsole( jsonBeautify(result, null, 2, 100) );

	      if(result.success)
	      {
	        this.moveToFront(pkgId);
	        this.setFocus(pkgId);

	        info.appState      = "LAUNCHED"; // RESUMED
	        this.launchedPkgId = pkgId;

	        // TODO: APP BUTTON - setLaunched()
	        var appButton = this.findInstalledButton(pkgId);
	        if(appButton != null)
	        {
	          appButton.stopSuspended();
	        }
	        else
	        {
	          console.log("resumePkg() >>> Cannot find App Button for pkgId: " + pkgId);
	        }
	      }
	      else
	      {
	        console.log( 'resumePkg() failed!');
	      }
	    }
	    catch(e)
	    {
	      console.log(     'resumePkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      this.setConsole( 'resumePkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async killPkg(pkgId, info)
	  {
	    let params =
	    {
	        "client": pkgId
	    };

	    try
	    {
	      // Need to resume before stopping a container app....
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'resumeApplication', params);

	      if(result.success)
	      {
	        info.appState = "STOPPED";
	      }
	      else
	      {
	        console.log( 'killPkg() >>> calling "resumeApplication" FAILED!');
	      }

	      // Next kill the App
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'kill', params);

	      if(result.success)
	      {
	        info.appState = "STOPPED";
	      }
	      else
	      {
	        console.log( 'killPkg() >>> calling "kill" FAILED!');
	      }

	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      console.log(     'killPkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      this.setConsole( 'killPkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }


	  async launchPkg(pkgId, info)
	  {
	    let params =
	    {
	        "client": pkgId,
	        "uri": pkgId, //TODO:  Unexpected... check why ?
	        // "uri": info.bundlePath,
	        "mimeType": "application/dac.native"
	    };

	    try
	    {
	      var result = await thunderJS$2.call('org.rdk.RDKShell.1', 'launchApplication', params);

	      console.log(jsonBeautify(result, null, 2, 100));
	      this.setConsole( jsonBeautify(result, null, 2, 100) );

	      if(result.success)
	      {
	        this.moveToFront(pkgId);
	        this.setFocus(pkgId);

	        info.appState      = "LAUNCHED"; // 1st LAUNCH
	        this.launchedPkgId = pkgId;

	        // TODO: APP BUTTON - setLaunched()
	      }
	      else
	      {
	        console.log( 'launchPkg() failed to launch app!!!');
	      }
	    }
	    catch(e)
	    {
	      console.log(     'launchPkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	      this.setConsole( 'launchPkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }
	  }

	  async installPkg(thisPkgId, info)
	  {
	    var myEvents = new Events(thunderJS$2, thisPkgId);

	    let buttons  = this.tag('AvailableList').children;
	    let button   = buttons[this.storeButtonIndex];
	    let progress = button.tag('Progress');

	    progress.reset(); // reset

	    let handleFailure = (notification, str) =>
	    {
	      console.log("FAILURE >> '"+str+"' ... notification = " + JSON.stringify(notification) );

	      if(thisPkgId == notification.pkgId)
	      {
	        button.setIcon(Utils.asset('images/x_mark.png'));

	        progress.setSmooth('alpha', 0, {duration: 1.3});

	        setTimeout( () =>
	        {
	          button.setIcon(Utils.asset('images/x_mark.png'));

	          progress.reset(); // reset

	          this.getAvailableSpace();

	        }, 1.2 * 1000); //ms

	        this.setConsole( jsonBeautify(notification, null, 2, 100) );
	      }
	    };

	    let handleFailureDownload     = (notification) => { handleFailure(notification,'FailureDownload');     };
	    let handleFailureDecryption   = (notification) => { handleFailure(notification,'FailureDecryption');   };
	    let handleFailureExtraction   = (notification) => { handleFailure(notification,'FailureExtraction');   };
	    let handleFailureVerification = (notification) => { handleFailure(notification,'FailureVerification'); };
	    let handleFailureInstall      = (notification) => { handleFailure(notification,'FailureInstall');      };

	    let handleProgress = (notification) =>
	    {
	      // console.log("HANDLER >> pkgId: "+thisPkgId+" ... notification = " + JSON.stringify(notification) );

	      if(thisPkgId == notification.pkgId)
	      {
	        let pc = notification.status / 8.0;
	        progress.setProgress(pc);

	        // console.log("HANDLER >> pkgId: "+thisPkgId+" ... progress = " + pc );

	        if(pc == 1.0)
	        {
	          progress.setSmooth('alpha', 0, {duration: 2.3});

	          var ans = AvailableApps.filter( (o) => { return o.pkgId == notification.pkgId; });

	          if(ans.length == 1) // IGNORE OTHER NOTTIFICATIONS
	          {
	            var info = ans[0];
	            this.onPkgInstalled(info, button);

	            if(info.events)
	            {
	              info.events.disposeAll(); // remove event handlers
	              info.events = null;
	            }
	          }
	        }//ENDIF - 100%
	      }
	    };

	    {
	      myEvents.add( 'Packager', 'onDownloadCommence', handleProgress);
	      myEvents.add( 'Packager', 'onDownloadComplete', handleProgress);

	      myEvents.add( 'Packager', 'onExtractCommence',  handleProgress);
	      myEvents.add( 'Packager', 'onExtractComplete',  handleProgress);

	      myEvents.add( 'Packager', 'onInstallCommence',  handleProgress);
	      myEvents.add( 'Packager', 'onInstallComplete',  handleProgress);

	      myEvents.add( 'Packager', 'onDownload_FAILED',     handleFailureDownload,) ;
	      myEvents.add( 'Packager', 'onDecryption_FAILED',   handleFailureDecryption) ;
	      myEvents.add( 'Packager', 'onExtraction_FAILED',   handleFailureExtraction) ;
	      myEvents.add( 'Packager', 'onVerification_FAILED', handleFailureVerification);
	      myEvents.add( 'Packager', 'onInstall_FAILED',      handleFailureInstall);
	    }

	    try
	    {
	      var result = await thunderJS$2.call('Packager', 'install', info);

	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      this.setConsole( 'installPkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }

	    info.events = myEvents;
	  }

	  async removePkg(pkgId)
	  {
	    console.log("removePkg() >>>    ENTER - ... pkgId: " + pkgId);

	    if(pkgId == undefined)
	    {
	      console.log("removePkg() >>>  ERROR - ... pkgId: " + pkgId);
	      return;
	    }

	    var params = {
	      "pkgId": pkgId
	    };

	    try
	    {
	      var result = await thunderJS$2.call('Packager', 'remove', params);

	      this.setConsole( jsonBeautify(result, null, 2, 100) );
	    }
	    catch(e)
	    {
	      this.setConsole( 'removePkg() >>> CAUGHT:  e: ' +  jsonBeautify(e, null, 2, 100) );
	    }

	    // Update the Installed
	    //
	    this.getAvailableSpace();
	    this.getInstalled();
	  }

	  onPkgInstalled(info, storeButton)
	  {
	    // console.log('onPkgInstalled() ... Installed >>> ' + info.pkgId)

	    info.pkgInstalled = true;

	    InstalledApps.push( info );
	    InstalledAppMap[info.pkgId] = info; // populate

	    this.tag('InstalledList').addTile(InstalledApps.length - 1, info);

	    // Disable STORE button - as it's UNINSTALLED
	    storeButton.disable();

	    this.getAvailableSpace();
	  }

	  _init()
	  {
	    this.storeButtonIndex     = 0;
	    this.installedButtonIndex = 0;
	    this.launchedPkgId        = "";

	    this.tag('Background').on('txLoaded', () =>
	    {
	      this._setState('IntroState');
	    });
	  }

	  handleToggleConsole()
	  {
	    let a = this.tag("ConsoleBG").alpha;
	    this.tag("ConsoleBG").setSmooth('alpha', (a == 1) ? 0 : 1, {duration: 0.3});
	  }

	  handleGetInfoALL()
	  {
	    this.getInstalled();
	  }

	  handleGetInfo()
	  {
	    let info = InstalledApps[this.installedButtonIndex];

	    this.getPackageInfo(info.pkgId || info.id);
	  }

	  // GLOBAL key handling
	  _handleKey(k)
	  {
	    switch( k.keyCode )
	    {
	      case HOME_KEY:
	      case 72: //'H' key
	        console.log("HOME code: " + k.keyCode);
	        this.setConsole( "HOME code: " + k.keyCode);

	        let info = InstalledAppMap[this.launchedPkgId];
	        if(info == "" || info == null)
	        {
	          console.log("Ignoring HOME key, no apps running");
	          break;
	        }

	        if(info.appState == "LAUNCHED")
	        {
	          console.log("Calling >>> this.suspendPkg()  pkgId: " + info.pkgId );
	          this.suspendPkg(info.pkgId, info);
	        }
	        else
	        {
	          console.log("Ignoring HOME key, no apps running");
	        }
	        break

	      case 65:  // 'A' key on keyboard
	      case 403: // 'A' key on remote
	          this.handleGetInfoALL();
	          break;

	      case 67:  // 'C' key on keyboard
	      case 405: // 'C' key on remote
	          this.handleToggleConsole();
	          break;

	      case 73:  // 'I' key on keyboard
	                // 'INFO' key on remote
	          this.handleGetInfo();
	          break;
	    }

	    return true;
	  }

	  static _states(){
	    return [
	          class IntroState extends this
	          {
	            $enter()
	            {
	              // console.log(">>>>>>>>>>>>   STATE:  IntroState");

	              var dlg = this.tag("OkCancel");
	              dlg.setSmooth('alpha', 0, {duration: 0.0});

	              let h1 =  (1080 + 79); // Move LOWER blind to below bottom (offscreen)
	              let h2 = -(h1/2 + 79); // Move UPPER blins to above top    (offscreen)

	              const anim = this.tag('RDKlogo').animation(
	              {
	                duration: 0.5,  delay: 1.5,
	                actions: [ { p: 'alpha', v: { 0: 1.0, 0.5: 0.75, 1: 0.0 } } ]
	              });

	              anim.on('finish', ()=>
	              {
	                this.tag('Blind1' ).setSmooth('y', h2, { delay: 0.25, duration: 0.75 });
	                this.tag('Blind2' ).setSmooth('y', h1, { delay: 0.25, duration: 0.75 });

	                this._setState('SetupState');
	              });

	              anim.start();
	            }
	          },
	          class SetupState extends this
	          {
	            fetchAppList(url)
	            {
	              // Fetch App List
	              //
	              fetch(url)
	              .then(res => res.json())
	              .then((apps) =>
	              {
	                apps.map( (o) => o.pkgInstalled = false); //default

	                AvailableApps = apps;
	                InstalledApps = apps;

	                this.tag("AvailableList").storeTiles   = AvailableApps;
	                this.tag("InstalledList").appTiles = InstalledApps;

	                this._setState('StoreRowState');
	              })
	              .catch(err =>
	              {
	                console.log("Failed to get URL: " + url);

	                AvailableApps = DefaultApps;

	                console.log("... using DefaultApps");

	                this.tag("AvailableList").tiles = AvailableApps;

	                this._setState('StoreRowState');
	              });
	            }

	            fetchThunderCfg(url)
	            {
	              // Fetch Thunder Cfg
	              //
	              fetch(url)
	              .then( res => res.json())
	              .then((cfg) =>
	              {
	                console.log(' >>> Creating CUSTOM ThunderJS ...');
	                thunderJS$2 = thunderJS$1(cfg);

	                this.getInstalled(); // <<< needs THUNDER
	              })
	              .catch(err =>
	              {
	                console.log("Failed to get URL: " + url);

	                console.log(' >>> Creating DEFAULT ThunderJS ...');
	                thunderJS$2 = thunderJS$1(thunder_cfg);

	                this.getInstalled();

	                console.log("... using default Thunder cfg.");
	              });
	            }

	            $enter()
	            {
	              // console.log(">>>>>>>>>>>>   STATE:  SetupState");

	              const URL_PARAMS = new window.URLSearchParams(window.location.search);
	              var appURL       = URL_PARAMS.get('appList');
	              var cfgURL       = URL_PARAMS.get('thunderCfg');

	              this.fetchThunderCfg(cfgURL);
	              this.fetchAppList(appURL);

	              this.setFocus(LIGHTNING_APP);
	              this.addKeyIntercept(); // Watch for HOME key

	              // State advanced within 'fetchAppList()' above.
	            }
	          },  //CLASS - SetupState
	          // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	          class StoreRowState extends this
	          {
	            $enter()
	            {
	              // console.log(">>>>>>>>>>>>   STATE:  StoreRowState");

	              // Set FOCUS to 1st package
	              //
	              var av_children = this.tag('AvailableList').children;
	              if(av_children.length >0)
	              {
	                av_children[this.storeButtonIndex].setFocus = true;
	              }

	              av_children.map( (o,n) => o.show(n * 0.15) );
	            }

	            _handleEnter()
	            {
	              let info   = AvailableApps[this.storeButtonIndex];
	              let button = this.tag('AvailableList').children[this.storeButtonIndex];

	              if(info == undefined)
	              {
	                console.log("FIRE >>> INSTALL  NO info !");
	                return // ignore
	              }

	              if(button.isEnabled() == false)
	              {
	                return // IGNORE CLICK
	              }

	              console.log("FIRE >>> INSTALL   pkgId:" + info.pkgId);

	              button.fireAncestors('$fireINSTALL', info.pkgId);
	            }

	            _handleDown()
	            {
	              if(InstalledApps.length > 0)
	              {
	                this._setState('InstalledRowState');
	              }
	            }

	            _handleLeft()
	            {
	              if(--this.storeButtonIndex < 0) this.storeButtonIndex = 0;
	            }

	            _handleRight()
	            {
	              if(++this.storeButtonIndex > AvailableApps.length) this.storeButtonIndex = AvailableApps.length - 1;
	            }

	            _getFocused()
	            {
	              return this.tag('AvailableList').children[this.storeButtonIndex]
	            }
	        }, //CLASS

	        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	        class InstalledRowState extends this
	        {
	          $enter()
	          {
	            // console.log(">>>>>>>>>>>>   STATE:  InstalledRowState");
	          }

	          _handleUp()
	          {
	            this._setState('StoreRowState');
	          }

	          _handleLeft()
	          {
	            if(--this.installedButtonIndex < 0) this.installedButtonIndex = 0;
	          }

	          _handleRight()
	          {
	            if(++this.installedButtonIndex >= InstalledApps.length) this.installedButtonIndex = InstalledApps.length - 1;
	          }

	          _handleEnter()
	          {
	            let info   = InstalledApps[this.installedButtonIndex];
	            let button = this.tag('InstalledList').children[this.installedButtonIndex];

	            console.log("FIRE >>> LAUNCH   pkgId:" + info.pkgId);

	            button.fireAncestors('$LaunchClicked', info.pkgId);
	            button.clickAnim();
	          }

	          _handleBack() // BACK key on the "Installed Row" - triggers the Delete OK/Cancel dialog
	          {
	            this._setState('OKCStateEnter');
	          }

	          _getFocused()
	          {
	            return this.tag('InstalledList').children[this.installedButtonIndex]
	          }
	        },//class
	        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	        class OKCStateEnter extends this
	        {
	          $enter()
	          {
	            // console.log(">>>>>>>>>>>>   STATE:  OKCStateEnter");

	            if(this.installedButtonIndex <0)
	            {
	              console.error(  'BUTTON index:' + this.installedButtonIndex +'  - INVLAID');
	              return;
	            }

	            var button = this.tag('InstalledList').children[this.installedButtonIndex];

	            if(button == undefined || button.info == undefined)
	            {
	              console.error(  'BUTTON index:' + this.installedButtonIndex +'  - NOT FOUND');
	              this.setConsole('BUTTON index:' + this.installedButtonIndex +'  - NOT FOUND');
	              return;
	            }
	            var pkgId = button.info.pkgId;

	            button.startWiggle();

	            var dlg    = this.tag("OkCancel");
	            dlg.pkgId  = pkgId; // needed later
	            dlg.button = button;

	            dlg.setLabel("Remove '" + pkgId + "' app ?");
	            dlg.setSmooth('alpha', 1, {duration: 0.3});

	            dlg._setState('OKCState');
	          }

	          _getFocused()
	          {
	            var dlg = this.tag("OkCancel");

	            return dlg;
	          }
	        },
	        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	      ]
	  }//_states
	}

	/*
	 * If not stated otherwise in this file or this component's LICENSE file the
	 * following copyright and licenses apply:
	 *
	 * Copyright 2020 RDK Management
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
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

	function index() {

	  return Launch(App, ...arguments)
	}

	return index;

}());
//# sourceMappingURL=appBundle.js.map
