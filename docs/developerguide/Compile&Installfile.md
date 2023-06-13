# 8. Compilation & Install

## a) CMake file
Inside the FooPlugin directory, create a new `CMakeLists.txt` file.

This file contains a set of directives and instructions describing the project's source files and targets. This is used to compile the Plugin code to generate the plugin library (Shared library by default; “.so”). External dependencies can be included/linked to the target with the help of CMakeLists.txt configurations.

Example:
```C
set(PLUGIN_NAME FooPlugin)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_PERSISTENTSTORE_FOOPLUGIN"" CACHE STRING "To configure startup order of FooPlugin")

find_package(${NAMESPACE}Plugins REQUIRED)

find_package(PkgConfig)

add_library(${MODULE_NAME} SHARED
        FooPlugin.cpp
        Module.cpp
        )

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)


target_include_directories(${MODULE_NAME} PRIVATE
        ../helpers)

target_link_libraries(${MODULE_NAME} PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins)

install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
```

## b) Compilation & Install

- To include `PluginName` plugin in build sequence, Open rdkservices recipe file and add below line. By default; its configured to be disabled while building rdkservices.

    ```git command
    $ vi meta-rdk-video/recipes-extended/rdkservices/rdkservices_git.bb
    ```

    PACKAGECONFIG[PluginName]  = " -DPLUGIN_PLUGINNAME=OFF,-DPLUGIN_PLUGINNAME=ON, "
- To include the plugin in rdkservises build; add the same in packageconfig in rdkservices recipe:
    ```git
        PACKAGECONFIG += " PluginName"
    ```
- To compile and install in build directory :
    ```git
    $ bitbake -c compile -f rdkservices
    ```
- Once build complete copy `.json`, `.so` files into RDK Set top box.

    - Copy the PluginName.json (Ex: FooPlugin.json) file to `/etc/WPEFramework/plugins` in RDK Set top box.

    - Copy the plugin library `libWPEFrameworkPluginName.so` to `/usr/lib/wpeframework/plugins`.

- So that the controller plugin identify it and list it in the `WebUI (controller UI)`.

