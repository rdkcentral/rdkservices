# **Follow the below steps to create a new RDK service/ Thunder plugin - In-process plugin.**

![Graphical Represenation of steps to create a new In-Process Thunder Plugin/ RDK Services](./In-process-plugin.png)

1. [**Create Project Folder**](createfolder.md)
2. [**Create a "CMake" file**](CMakefile.md)
3. [**Plugin Json API Spec**](PluginJSONfile.md)
4. [**Headers Files**](headerfiles.md)
5. [**Source Files**]()
6. [**Plugin Config File**]()
7. [**Compilation & Install**]()
8. [**Controller UI**]()

7. Create a header file with name `pluginName.h`.
- This header file includes the support for JSON request, response, logging etc.

- Declare the plugin class in this which should contain all the structures, variables, and methods which are needed for plugin implementation.

- A plugin should derive from the `PluginHost::JSONRPC` public class if it intends to use the JSON-RPC protocol. By doing this, the plugin can use the base class to access the JSON-RPC protocol's capabilities and functionalities. This enables the plugin to interact via the JSON-RPC interface with other programmes or systems. Thunder Plugin must adhere to the `PluginHost::IPlugin` interface.

- Create the required objects of its plugin class type using `constructor` and start `initialization` for the created objects of its Plugin class type within `IPlugin Initialize()`.

- Return a non-empty string containing helpful error information if there was any `initialization error`. The plugin won't activate as a result of this, and the developer or caller will also receive this error information.

- Make sure that `IPlugin Deinitialize()`, which is called when the plugin is deactivated, cleans up the initialization carried out with `IPlugin Initialize()` using `deconstructor`.

```C++
namespace WPEFramework {
    namespace Plugin {  
         SERVICE_REGISTRATION(Plugin, 1, 0);
                 
          //registration
          //All the methods declared in Plugin.h should be registered here
                 
          //initialize and deinitialize the handlers for the plug-in service
                 
          //All the methods declared in Plugin.h should be defined here
```

 For example, please refer to link [SecurityAgent.h file](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/SecurityAgent.h).

8. Create a `module.cpp` C++ file as mentioned below.
```C++
#include "Module.h"

MODULE_NAME_DECLARATION(BUILD_REFERENCE)
```
 For example, Please refer to link [SecurityAgent/Module.cpp](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/Module.cpp).

9.	Create a new C++ file with `PluginName.cpp` name which is used to declare the module name for the Plugin.
- The plugin should register using service registration MACRO as declared below :
```C++
 namespace WPEFramework {
   namespace Plugin { 
    SERVICE_REGISTRATION(Plugin, 1, 0);
    ---------------------------------------
    ---------------------------------------
    ---------------------------------------
   }
  }
  ```

- The files `PluginName.h` already include `Initialization` and `Deinitialization` code. Here, identical files may be utilised.

     For example, please refer to link [SecurityAgent.cpp file](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/SecurityAgent.cpp).

  **Proxy/Stub generator:**
  - **Proxy** - object in one process space representing the ’real’ object in another process space; proxy takes care of marshalling the parameters.
  - **Marshalling** - process of transforming the memory representation of an object into a data format suitable for storage or transmission. It is typically used when data must be moved between different parts of a computer program or from one program to another. 
  - **Stub** - object in the process space that contains the actual object; takes care of unmarshalling the request from Proxy and on behave of the Proxy object executes the call on the real object.

    Create JSON header and C++ file using the [proxy/stub generator](https://github.com/rdkcentral/Thunder/tree/master/Tools/ProxyStubGenerator).

10. Create a C++ file with name `PluginNameJsonRpc.cpp` which contains the registration for methods and properties which are declared in `PluginName.h`.

- Complete `RegisterAll()` and `UnregisterAll()` for the all the required classes, objects, or Variables.

```C++
#include "PluginName.h"

namespace WPEFramework {
namespace Plugin {

void PluginName::RegisterAll()
{
    Register<JsonObject, JsonObject>(_T("----"), &PluginName::endpoint_----, this);
    -------------------------------------
    -------------------------------------
    
}

void PluginName::UnregisterAll()
{
    Unregister(_T("----"));
    ----------------------
    ----------------------
}
}
}
```
 
For example, please refer to link [SecurityAgentJsonRpc file](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/SecurityAgentJsonRpc.cpp).


11.	Create a configuration file `PluginName.config` which is used to set configurations of the Plugin. 

    Ex: set (autostart true) is used to make the Plugin to start automatically along with WPEframework daemon.

     For example, please refer to link [SecurityAgent/configuration file](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/SecurityAgent.config).

12. Create a `CHANGELOG.md` file where you can list all of the modifications you have made to the plugin over time with version updates.
- VERSION_NUMBER_MAJOR
- VERSION_NUMBER_MINOR
- VERSION_NUMBER_PATCH

    For example, Please refer to link [SecurityAgent/Changelog.md](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/CHANGELOG.md).

13. To add new plugin in rdkservices, Compile and execute the `CMakeLists.txt` which contains a set of directives and instructions describing the project's source files and targets (executable, library, or both).
- Using the `CMake` utility, this file contains the tasks needed to be done to make a plugin. Additionally, it contains packages, libraries needed to compile, as well as other plugin configuration options.

- Adds a library target called `name` to be built from the source files listed in the command invocation. The `name` corresponds to the logical target name and must be globally unique within a project.
```C++
add_library(${MODULE_NAME} SHARED
        PluginName.cpp
        Module.cpp
        ../helpers/utils.cpp)
```
- This file is compiled and generate `.so` file.
- Enable or disable the plugin flag in the recipe file.
- Add this flag into the main CMakeLists.txt file present in the rdkservice.
- Add these lines ( by default its disabled ) :
```python
    if(PLUGIN_PLUGINNAME)
        add_subdirectory(PluginName)
    endif()
```
- It will invoke CMakeLists file present in the `pluginName.json`.

## Compilation and Install:
- To include `PluginName` plugin in build sequence, Open rdkservices recipe file and add below line. By default; its configured to be disabled while building rdkservices.

```git command
$ vi meta-rdk-video/recipes-extended/rdkservices/rdkservices_git.bb
```
    PACKAGECONFIG[PluginName]  = " -DPLUGIN_PLUGINNAME=OFF,-DPLUGIN_PLUGINNAME=ON, "
- To include the plugin in rdkservises build; add the same in packageconfig in rdkservices recipe:
    PACKAGECONFIG += " PluginName"

- To compile and install in build directory :
    $ bitbake -c compile -f rdkservices

- Once build complete copy `.json`, `.so` files into `raspberrypi`.

    - Copy the PluginName.json (Ex: SecurityAgent.json) file to `/etc/WPEFramework/plugins` in `raspberrypi`

    - Copy the plugin library `libWPEFrameworkPluginName.so` to `/usr/lib/wpeframework/plugins`

- So that the controller plugin identify it and list it in the `WebUI (controller UI)`.

## Controller UI:
- Controller UI is a web UI that can be launched from a host machine's (machine under the same network where RPi resides) browser. This UI can be loaded with the RPi box's IP address with Thunder's port number configured (here). RDKServices uses **9998** as port.

    URL: http://<Rpi's IP address>:9998

![Controller UI](./Controller_UI_In-process.PNG)


## New Plugin test and validation for In-process Plugin:
- Each RDK Service can be validated through JSON RPC Commands through HTTP. It has a request and response in JSON format.

callsign":"org.rdk.PluginName

| Function | Request | Response |
| :-------- | :-------- | :-------- |
| Activate controller | curl http://localhost:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{ "jsonrpc":"2.0", "id":3, "method":"Controller.1.activate", "params":{"callsign":"org.rdk.PluginName"} }'  | {"jsonrpc":"2.0","id":3,"result":{"success":true}} |
|Deactivate controller|curl http://localhost:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{ "jsonrpc":"2.0", "id":3, "method":"Controller.1.deactivate", "params":{"callsign":"org.rdk.PluginName"} }'|{"jsonrpc":"2.0","id":3,"result":{"success":true}}|
|getPluginStatus|curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getPluginNameStatus"}'|{"jsonrpc":"2.0","id":3,"result":{"connection status from plugin":["CONNECTED"],"success":true}}|
| getPluginNameList |	curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getPluginNameList"}'|{"jsonrpc":"2.0","id":3,"result":{"Supported plugin list":["plug-A","plug-B","plug-C","plug-D","plug-E"],"success":true}}|	
|getPluginNameInfo|	curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getPluginNameInfo", "params":{"plugin_name":"plug-A"}}'| {"jsonrpc":"2.0","id":3,"result":{"supportedTvResolutions":["xyz-plugin","no:430HT5"],"success":true}}root@raspberrypi-rdk-mc:~#|
| event API when hdmi connected|  curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getConnectedVideoDisplays"}'| {"jsonrpc":"2.0","id":3,"result":{"connectedVideoDisplays":["HDMI0"],"success":true}}root@raspberrypi-rdk-mc:~#|
|event API when hdmi not connected| curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getConnectedVideoDisplays"}'| {"jsonrpc":"2.0","id":3,"result":{"connectedVideoDisplays":[],"success":true}}root@raspberrypi-rdk-mc:~#|

- Please refer the link for [Out-of-process plugin](https://github.com/Pavankumar-HM/rdkservices/blob/main/docs/developerguide/new_plugin_outofprocess.md).


