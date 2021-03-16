
#pragma once                                  //Specifies that the compiler includes the header file only once, when compiling a source code file.

// if module name is not define then define module name as PluginTemplate
#ifndef MODULE_NAME
#define MODULE_NAME PluginTemplate
#endif

#include <plugins/plugins.h>                   // include libraries
#include <tracing/tracing.h>		       // include libraries

#undef EXTERNAL
#define EXTERNAL
