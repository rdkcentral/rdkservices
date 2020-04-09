ContinueWatching
===============
***@mainpage ContinueWatching***

The Continue Watching Service will provide a method for applications on the STB to store a token for retrieval by XRE.Applications like netflix
will use this to store a token that XRE will retrieve and use the data to gather data from the OTT provider that can be used to populate a 
continue watching panel in the UI.This service will be enabled/disabled using an TR181 parameter.

**API's:**
- setApplicationToken
- getApplicationToken
- deleteApplicationToken

## Build:
```
bitbake thunder-plugins
```

## Test:
Before testing the continuewatching apis ,please ensure that OTT_token is enabled.Hence run the following commands:
```
export NETFLIX_ENABLE_COOKIE=true
tr181 -s -v true Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.OTT_Token.Enable
To verify  if OTT_Token is set: tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.OTT_Token.Enable
```

>Note: Below listed curl requests contains sample values for testing purposes only.

### setApplicationToken :
Will set the given token to the application.Returns CW_OK status if set,else returns a CW_ERROR status
```
Request:
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.ContinueWatching.1.setApplicationToken","params": {"applicationName":"netflix","token":"abcdefghijklmnopqrstuvwxyz"}}' http://127.0.0.1:9998/jsonrpc
Response:
{"jsonrpc":"2.0","id":3,"result":{"CW_STATUS":0,"success":true}
```

Check if the /opt/continuewatching.json file is created and ensure token is stored in an encrypted format as seen below.
```
{
        "tokens":       [{
                        "applicationName":      "netflix",
                        "encryptedData":        "OvABTqvfs1LnbgfQoNcT8QUNpotioUkek705w3AOEh/7e2j6uju5FUN1fjzgXJt7XSN9NvuYKe7AwD0gx0WBuFoMWup0VYWwVmmTscPStdxI1w3YzW7GsO"
                }]
}
```

### getApplicationToken :
Used to retrieve the token value of a particular application.Returns the token value on success,else it returns a CW_ERROR status.
```
Request:
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.ContinueWatching.1.getApplicationToken","params":{ "applicationName":"netflix"}}' http://127.0.0.1:9998/jsonrpc
Response:
{"jsonrpc":"2.0","id":3,"result":{"token":"abcdefghijklmnopqrstuvwxyz","CW_STATUS":0,"success":true}}
```

### deleteApplicationToken :
Deletes the token stored for a particular application.
```
Request:
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.ContinueWatching.1.deleteApplicationToken","params":{"applicationName":"netflix"}}' http://127.0.0.1:9998/jsonrpc
Response:
 {"jsonrpc":"2.0","id":3,"result":{"CW_STATUS":0,"success":true}}

```
check the /opt/continuewatching.json file to see if the application's token is deleted as seen below:
```
{
        "tokens":       []
}
```
## Return Values:
CW_STATUS is returned in the json response.It can take the following values:

CW_STATUS                      | Values |
-------------------------------|--------|
CW_OK                          |    0   |
CW_FAIL                        |    1   |


success value is also passed in the response .If its  true it indicates API executed correctly and if its false it indiactes API failure.



