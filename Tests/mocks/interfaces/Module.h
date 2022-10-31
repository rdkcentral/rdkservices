#pragma once

#ifndef MODULE_NAME
#define MODULE_NAME Interfaces
#endif

#include <core/core.h>

// These are interfaces offered by Thunder and used in the Plugins. Make them
// available on interfaces taht are exposed cross plugins.
#include <plugins/IPlugin.h>
#include <plugins/ISubSystem.h>
#include <plugins/IShell.h>
#include <plugins/IStateControl.h>

