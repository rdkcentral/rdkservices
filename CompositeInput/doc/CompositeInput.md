
Versions
org.rdk.CompositeInput.1 - initial version of Composite Input API.

------------------------------

Methods

-----------------------------

getCompositeInputDevices (v1)
Description: returns an array of composite input devices.

Arguments: none

Returns:

devices : array - an array of compositeInputDevice object (based on number of Composite Input ports on the device), where compositeInputDevice is:

{
    id: number,
    location: string,
    connected: boolean
}



Request : {"jsonrpc":"2.0", "id":3, "method":"org.rdk.CompositeInput.1.getCompositeInputDevices"}
 
Response: {"jsonrpc":"2.0","id":3,"result":{"devices":[{"id":0,"locator":"cvbsin://localhost/deviceid/0","connected":"true"},{"id":1,"locator":"cvbsin://localhost/deviceid/1","connected":"false"}],"success":true}}


---------------------------------

startCompositeInput (v1)
Description: Activates selected Composite In Port as primary video source.  

Arguments

portId : number - an id of Composite In source.   (getCompositeInputDevices (v1) will list available Composite Input devices and portIds)
Returns: 

success : bool - result of activating Composite Input  
Request : {"jsonrpc":"2.0", "id":3, "method":"org.rdk.CompositeInput.1.startCompositeInput", "params": {"portId":"1"}}
 
Response: {"jsonrpc":"2.0","id":3,"result":{"success":true}}

---------------------------------

stopCompositeInput (v1)
Description: Deactivates current Composite In Port selected as primary video source. 

Returns:

success : bool - result of deactivating Composite Input 
Request : {"jsonrpc":"2.0", "id":3, "method":"org.rdk.CompositeInput.1.stopCompositeInput"}
 
Response: {"jsonrpc":"2.0","id":3,"result":{"success":true}}

--------------------------------

setVideoRectangle (v1)
Description: Sets Composite Input video window

Arguments

x: int - The x-coordinate of the video rectangle.
y: int - The y-coordinate of the video rectangle.
w: int - The width of the video rectangle.
h: int - The height of the video rectangle.
Returns: 

success : bool - result of setting Composite Input  video window
Request : {"jsonrpc":"2.0","id":"3","method": "org.rdk.CompositeInput.1.setVideoRectangle", "params":{"x":"0","y":"0","w":"1920","h":"1080"}}
 
Response: {"jsonrpc":"2.0","id":3,"result":{"success":true}}

--------------------------------


