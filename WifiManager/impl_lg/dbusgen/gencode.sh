#!/bin/bash

# WE ARE NOT DIRECTLY USING THESE GENERATED FILES!
# dbus-codegen generated code creates static objects & this doesn't play well with wpeframework & dynamically
# loaded & unloaded modules (generally, not possible to disable and reenable the plugin then)
# these files are still useful - part of the implementation can be reused, with slight changes
gdbus-codegen --generate-c-code networkconfig1.autogen ../../../../../../../../../../onemw-src/onemw-src/networking/om-netconfig/api/dbus/com.lgi.rdk.utils.networkconfig1.xml
gdbus-codegen --generate-c-code wifimanagement1.autogen ../../../../../../../../../../onemw-src/onemw-src/networking/om-netconfig/api/dbus/com.lgi.rdk.utils.wifimanagement1.xml



