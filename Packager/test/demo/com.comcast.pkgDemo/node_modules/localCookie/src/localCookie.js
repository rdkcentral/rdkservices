// LocalStorage and Cookie storage library
class localCookie {
	constructor(options) {
		options = options || {}
		this.forceCookies = options.forceCookies || false

		if (this._checkIfLocalStorageWorks() === true && options.forceCookies !== true)
			return {
				getItem : this._getItemLocalStorage,
				setItem : this._setItemLocalStorage,
				removeItem : this._removeItemLocalStorage,
				clear : this._clearLocalStorage
			}
		else
			return {
				getItem : this._getItemCookie,
				setItem : this._setItemCookie,
				removeItem : this._removeItemCookie,
				clear : this._clearCookies
			}
	}

	// Checks if local storage is available and works
	_checkIfLocalStorageWorks() {
		if (typeof localStorage !== 'undefined') {
			try {
				localStorage.setItem('feature_test', 'yes')
				if (localStorage.getItem('feature_test') === 'yes') {
					localStorage.removeItem('feature_test')
					// localStorage is enabled
					return true
				} else {
					// localStorage is disabled
					return false
				}
			} catch(e) {
				// localStorage is disabled
				return false
			}
		} else {
			// localStorage is not available
			return false
		}
	}

	/*
	 * Local Storage APIs
	 */
	 _getItemLocalStorage(key) {
	 	return window.localStorage.getItem(key)
	 }

	 _setItemLocalStorage(key, value) {
	 	return window.localStorage.setItem(key, value)
	 }

	 _removeItemLocalStorage(key) {
	 	return window.localStorage.removeItem(key)
	 }

	 _clearLocalStorage() {
	 	return window.localStorage.clear()
	 }

	 /*
	  * Cookie APIs
	  */
	_getItemCookie(key) {
		function escape(s) { return s.replace(/([.*+?\^${}()|\[\]\/\\])/g, '\\$1'); };
		var match = document.cookie.match(RegExp('(?:^|;\\s*)' + escape(key) + '=([^;]*)'));

		//if value in key of the entire cookie is empty, its deleted
		if (match && match[1] === '')
			match[1] = null

		return match ? match[1] : null;
	}

	_setItemCookie(key, value) {
		document.cookie = `${key}=${value}`
		return //return undefined just like localStorage
	}

	_removeItemCookie(key) {
		// to delete an k/v set the key to empty
		document.cookie = `${key}=;Max-Age=-99999999;`
		return //return undefined just like localStorage
	}

	_clearCookies() {
		// empty all key/values in cookies
		// What about cookies we got from the server? ¯\_(ツ)_/¯
		// They should have the HttpOnly flag, so we shouldn't be able to delete them
		document.cookie.split(";").forEach( (c) => {
			document.cookie = c.replace(/^ +/, "").replace(/=.*/, "=;expires=Max-Age=-99999999");
		});

		return //return undefined
	}
}

export default localCookie