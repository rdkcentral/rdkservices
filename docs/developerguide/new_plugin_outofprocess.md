# **How to add a new RDK service/Thunder plugin**
- Here the plugin is developed as an out-of-process that runs as a separate thread from WPEFrameworkServices. The particular service can either be COMRPC (for communication between plugins) or JSONRPC (for external communication). It has a web-based controller UI.

## **Please follow the below steps to create a new RDK service/ Thunder plugin - Out-of-process plugin.**
1. Create a clone repository of [rdkcentral/ rdkservices](https://github.com/rdkcentral/rdkservices).

2. Navigate to the cloned repository.

3. Create a new folder with a unique name for the plugin. Please refer to link [PersistentStore Plugin](https://github.com/rdkcentral/rdkservices/tree/sprint/23Q1/PersistentStore).

4. Inside the plugin directory, create a new `CMakeLists.txt` file.
-	This file contains a set of directives and instructions describing the project's source files and targets. This is used to compile the Plugin code to generate the plugin library (Shared library by default; `“.so”`). External dependencies can be included/linked to the target with the help of `CMakeLists.txt` configurations.

    For example, please refer to link [ PersistentStore CMakelist](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/PersistentStore/CMakeLists.txt).
