# **How to add a new RDK service/Thunder plugin**

- There are two types of plugins in RDK, In-Process plugins, and Out-of-Process plugins.
- In-Process plugins run within the same process as the main application and can access the application's memory directly.
- Out-of-Process plugins run in a separate process from the main application and connect to main WPEFramework application through COM RPC (Unix sockets).
- In summary, the choice between In-Process and Out-of-Process plugins depends on the desired trade-off between stability, security, and performance. Out-of-Process plugins are created primarily for stability (any crashes won't bring down the main WPEFramework process) and for resource intensive plugins (browser plugins like WebKitBrowser) that would fit better as a separate process, whereas In-Process plugins are created mainly for performance.

## **Please follow the below steps to create a new RDK service/ Thunder plugin - In process plugin.**
1. Create a clone repository of [rdkcentral/ rdkservices](https://github.com/rdkcentral/rdkservices).

2. Navigate to the cloned repository.

3. Create a new folder with a unique name for the plugin. Please refer to link [SecurityAgent Plugin](https://github.com/rdkcentral/rdkservices/tree/sprint/23Q1/SecurityAgent) for sample.

4.	Inside the plugin directory, create a new `CMakeLists.txt` file.
-	This file contains a set of directives and instructions describing the project's source files and targets. This is used to compile the Plugin code to generate the plugin library (Shared library by default; `“.so”`). External dependencies can be included/linked to the target with the help of `CMakeLists.txt` configurations.

    For example, please refer to link [ SecurityAgent CMakelist](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/CMakeLists.txt).

5. Create a new JSON file with a unique name `PluginName.json`.
-	This file contains the plugin's information like schema, information and interface json file.
    - **Schema**: A schema defines the structure of a JSON document, including the type of data that can be stored in it, the names of fields and their types, and constraints on the values that can be stored in those fields. The purpose of a schema is to provide a blueprint for how the data in a JSON document should be organized, so that applications can validate the data before processing it.

    - **Information**: Information refers to the actual data stored in a JSON document. This data can be in the form of key-value pairs, arrays, or nested structures. The information stored in a JSON document can be used to represent a wide range of data, including configuration settings, metadata, and other types of structured data.

    - **Interface**: An interface defines the way in which a JSON document can be accessed and processed by an application. It specifies the methods and properties that are available for accessing and manipulating the data stored in the document, as well as any constraints or requirements for using those methods and properties. Interfaces are often used to define the API (Application Programming Interface) for a JSON document, making it easier for developers to interact with the data stored in the document

    For example, please refer to link [SecurityAgent JSON file](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/SecurityAgent.json).

**About Thunder Interfaces** - A thunder interface is `JSONRPC` and `COMRPC` interfaces which provides the communication between the `thunder (WPE Framework)` and `thunder plugins (RDK Services)`. If thunder plugin is a out of process plugin, then it's having their own thunder interface which will be saved and maintained in a separate repo [Thunder interfaces](https://github.com/rdkcentral/ThunderInterfaces).

For example, `PersistentStore` Plugin has their own Thunder interface called `IStore` and located in [https://github.com/rdkcentral/ThunderInterfaces/blob/master/interfaces/IStore.h](https://github.com/rdkcentral/ThunderInterfaces/blob/master/interfaces/IStore.h).

6. Create a `module.h` header file as metioned below with the required PluginName. 
 ```C++
#pragma once
#ifndef MODULE_NAME
#define MODULE_NAME Plugin_PLuginName
#endif

#include <plugins/plugins.h>
#include <tracing/tracing.h>

#undef EXTERNAL
#define EXTERNAL

```
For example, Please refer to link [SecurityAgent/Module.h](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/Module.h).

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

## New Plugin test and validation
- Each RDK Service can be validated through JSON RPC Commands through HTTP. It has a request and response in JSON format.

| Function | Request | Response |
| :-------- | :-------- | :-------- |
| Activate controller | curl http://localhost:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{ "jsonrpc":"2.0", "id":3, "method":"Controller.1.activate", "params":{"callsign":"org.rdk.PluginName"} }'  | {"jsonrpc":"2.0","id":3,"result":{"success":true}} |
|Deactivate controller|curl http://localhost:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{ "jsonrpc":"2.0", "id":3, "method":"Controller.1.deactivate", "params":{"callsign":"org.rdk.PluginName"} }'|{"jsonrpc":"2.0","id":3,"result":{"success":true}}|
|getPluginStatus|curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getPluginNameStatus"}'|{"jsonrpc":"2.0","id":3,"result":{"connection status from plugin":["CONNECTED"],"success":true}}|
| getPluginNameList |	curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getPluginNameList"}'|{"jsonrpc":"2.0","id":3,"result":{"Supported plugin list":["plug-A","plug-B","plug-C","plug-D","plug-E"],"success":true}}|	
|getPluginNameInfo|	curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getPluginNameInfo", "params":{"plugin_name":"plug-A"}}'| {"jsonrpc":"2.0","id":3,"result":{"supportedTvResolutions":["xyz-plugin","no:430HT5"],"success":true}}root@raspberrypi-rdk-mc:~#|
| event API when hdmi connected|  curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getConnectedVideoDisplays"}'| {"jsonrpc":"2.0","id":3,"result":{"connectedVideoDisplays":["HDMI0"],"success":true}}root@raspberrypi-rdk-mc:~#|
|event API when hdmi not connected| curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getConnectedVideoDisplays"}'| {"jsonrpc":"2.0","id":3,"result":{"connectedVideoDisplays":[],"success":true}}root@raspberrypi-rdk-mc:~#|

- Please refer the link for [Out-of-process plugin](https://github.com/Pavankumar-HM/rdkservices/blob/main/docs/Developer_guide/new_plugin_outofprocess.md).


