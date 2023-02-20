2. Create a "CMake" file

- Inside the plugin directory, create a new `CMakeLists.txt` file.
- This file contains a set of directives and instructions describing the project's source files and targets.
- This is used to compile the Plugin code to generate the plugin library (Shared library by default; `“.so”`). External dependencies can be included/linked to the target with the help of `CMakeLists.txt` configurations.

    For example, please refer to link [ SecurityAgent CMakelist](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/CMakeLists.txt).