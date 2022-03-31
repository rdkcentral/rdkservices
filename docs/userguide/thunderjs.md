# Creating Thunder JS Clients

The samples discussed here are taken from the ThunderJS project. For complete code examples and documentation, see [ThunderJS](https://github.com/rdkcentral/ThunderJS).

The thunderJS.js library is a very flexible client library that supports making service calls using multiple programming styles. A common approach when building an application is to call RDK services in an object oriented manner. The following example initializes the library and configures the device host address. It then creates a `getSystemInfo` function for calling the `systeminfo` method on the DeviceInfo service and uses promises to capture a successful result or error.

```javascript
var thunderJS
var defaultHost = localStorage.getItem('host')
var host = prompt('Please inform the IP address of your STB', defaultHost || '192.168.')
localStorage.setItem('host', host)
thunderJS = ThunderJS({
  host: host,
})
function getSystemInfo() {
  log('Calling: DeviceInfo.systeminfo')
  thunderJS.DeviceInfo.systeminfo()
    .then(function(result) {
      log('Success', result)
    })
    .catch(function(error) {
      log('Error', error)
    })
}
function log(msg, content) {
  var el = document.getElementById('log')
  var entry = '<p class="font-bold">' + msg + '</p>'
  if (content) {
    entry += '<pre class="border mt-4 mb-8 text-sm">' + JSON.stringify(content, null, 2) + '</pre>'
  }
  entry += '<hr class="border-b" />'
  el.innerHTML += entry
}
```

A simple HTML application can then reference the implementation and the thunderJS.js library and call the `getSystemInfo` function as follows:

```html
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>ThunderJS - Browser example</title>
  </head>
  <body>
    <script src="thunderJS.js"></script>
    <script src="myImpl.js"></script>
    <button>onclick="getSystemInfo()">Get SystemInfo</button>
  </body>
</html>
```

The `systeminfo` method does not take any parameters; however, many RDK service methods do take parameters. In this case, you can pass parameters as an argument to the method. For example:

```javascript
const params = {
  videoDisplay: 'HDMI0'
}
thunderJS.DisplaySettings.getSupportedResolutions(params)
```
