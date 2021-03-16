#
# this file contains a set of directives and instructions describing the project's source files and targets (executable, library, or both).
#


set(PLUGIN_NAME PluginTemplate)                                 # to set a environment variable set(<variable> <value>)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)                # to Finds and loads settings from an external project.


#Adds a library target called <name> to be built from the source files listed in the command invocation. The <name> corresponds to the logical target name and must be globally unique within a project.

add_library(${MODULE_NAME} SHARED
        PluginTemplate.cpp
        Module.cpp
        ../helpers/utils.cpp)

#The syntax for the command is to list all the targets you want to change, and then provide the values you want to set next. You can use any prop value pair you want and extract it later with the get_property() or get_target_property() command.

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

#Specifies include directories to use when compiling a given target.

target_include_directories(${MODULE_NAME} PRIVATE ../helpers)


find_package(DS)
if (DS_FOUND)
    find_package(IARMBus)
    add_definitions(-DDS_FOUND)
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_include_directories(${MODULE_NAME} PRIVATE ${DS_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES} ${DS_LIBRARIES})
else (DS_FOUND)
   #Specify libraries or flags to use when linking a given target and/or its dependents.
   target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)
endif(DS_FOUND)

#his command generates installation rules for a project. Install rules specified by calls to the install() command within a source directory are executed in order during installation.
install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
