
#pragma once				//Specifies that the compiler includes the header file only once when compiling a source code file.

#include "Module.h"                     //this file contain module details
#include "utils.h"			//this file contain build in common functionalities
#include "AbstractPlugin.h"		//it contain abstract plugin class with essential virtual methods that need to be implemented in the derived plugin class.
#include "libIBus.h"			// this file contain multiple IARM-Bus instances
#include "irMgr.h"			// this file related to IARM bus manager

namespace WPEFramework {

    namespace Plugin {

		// This is a server for a JSONRPC communication channel.
		// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
		// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
		// This realization of this interface implements, by default, the following methods on this plugin
		// - exists
		// - register
		// - unregister
		// Any other method to be handled by this plugin  can be added by using the
		// templated methods Register on the PluginHost::JSONRPC class.
		// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
		// this class exposes a public method called, Notify(), using this methods, all subscribed clients
		// will receive a JSONRPC message as a notification, in case this method is called.
        class PluginTemplate : public AbstractPlugin {
        private:
            typedef Core::JSON::String JString;
            typedef Core::JSON::ArrayType<JString> JStringArray;
            typedef Core::JSON::Boolean JBool;


            //Begin methods
            uint32_t getPluginTemplateStatus(const JsonObject& parameters, JsonObject& response);  // to retrieve plugin status(eg)
            uint32_t getPluginTemplateList(const JsonObject& parameters, JsonObject& response);    // to get coonected plugins(eg)
	    uint32_t getPluginTemplateInfo(const JsonObject& parameters, JsonObject& response);    // to retrive selected plugin info(eg)

	    uint32_t getConnectedVideoDisplays(const JsonObject& parameters, JsonObject& response); // to get information about connected display

	    //Begin event
	    void connectedVideoDisplaysUpdated(int hdmiHotPlugEvent);   //for event handling
	    //End events



        public:
            PluginTemplate();									// constructor
            virtual ~PluginTemplate();							// destructor
            virtual const string Initialize(PluginHost::IShell* service) override;	//initialize IARM plugin services
            virtual void Deinitialize(PluginHost::IShell* service) override;		//deinitialize IARM plugin services
        public:
            static PluginTemplate* _instance;

        private:
            void InitializeIARM();										// calling IARM services
            void DeinitializeIARM();   										// calling IARM services
            static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);  // display hdmi event handler
            void getConnectedVideoDisplaysHelper(std::vector<string>& connectedDisplays);			// display hdmi calls

        };
	} // namespace Plugin
} // namespace WPEFramework
