/*
This class does contain all the definitions for the methods declared in the Plugin.h and those definitions
should be defined inside the below namespace.
*/

// include all the depended libraries
#include "PluginTemplate.h"
#include <algorithm>
#include "dsMgr.h"

// This files and libraries loaded and  added by target_include_directories(), target_link_libraries() functions in CMakeList.txt
#include "libIBusDaemon.h"
#include "host.hpp"
#include "exception.hpp"
#include "videoOutputPort.hpp"
#include "videoOutputPortType.hpp"
#include "videoOutputPortConfig.hpp"
#include "videoResolution.hpp"
#include "manager.hpp"
#include "dsUtl.h"
#include "dsError.h"
#include "list.hpp"
#include "libIBus.h"
#include "dsDisplay.h"
#include "rdk/iarmmgrs-hal/pwrMgr.h"

#include "tracing/Logging.h"
#include <syscall.h>
#include "utils.h"

#define HDMI_HOT_PLUG_EVENT_CONNECTED 0                 //for hdmi plugin event



using namespace std;


namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(PluginTemplate, 1, 0);		//service registration MACRO used for registering service

        PluginTemplate* PluginTemplate::_instance = nullptr;

        PluginTemplate::PluginTemplate()				//constructor
            : AbstractPlugin()
        {
            LOGINFO("ctor");
            PluginTemplate::_instance = this;
	    
	    // All the methods declared in Plugin.h should be registered here	
            registerMethod("getPluginTemplateStatus", &PluginTemplate::getPluginTemplateStatus, this);
            registerMethod("getPluginTemplateList", &PluginTemplate::getPluginTemplateList, this);
            registerMethod("getPluginTemplateInfo", &PluginTemplate::getPluginTemplateInfo, this);
            registerMethod("getConnectedVideoDisplays", &PluginTemplate::getConnectedVideoDisplays, this);

        }

        PluginTemplate::~PluginTemplate()				//destrucor
        {
            LOGINFO("dtor");
            PluginTemplate::_instance = nullptr;
        }


	//initialize and deinitialize or activate or deactivate handler for the plugin services:
        const string PluginTemplate::Initialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
	    // hdmi event occuring through IARM
	    InitializeIARM();

            return (string());
        }

        void PluginTemplate::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
	    // close IARM
	    DeinitializeIARM();

        }

        void PluginTemplate::InitializeIARM()
        {
            LOGINFO();

            if (Utils::IARM::init())
            {
                IARM_Result_t resi;
		//hdmi event handler through IARM bus
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
            }

            try
            {
                device::Manager::Initialize();
                LOGINFO("device::Manager::Initialize success");
            }
            catch(...)
            {
                LOGINFO("device::Manager::Initialize failed");
            }
        }


        void PluginTemplate::DeinitializeIARM()
        {
            LOGINFO();

            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG) );
            }

            try
            {
                device::Manager::DeInitialize();
                LOGINFO("device::Manager::DeInitialize success");
            }
            catch(...)
            {
                LOGINFO("device::Manager::DeInitialize failed");
            }
        }


	// hedmi event handler
        void PluginTemplate::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            LOGINFO();
            switch (eventId)
            {
            case IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG :
                {
		    //IARM event manger
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    int hdmi_hotplug_event = eventData->data.hdmi_hpd.event;
                    LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d ", hdmi_hotplug_event);
                    if(PluginTemplate::_instance)
                        PluginTemplate::_instance->connectedVideoDisplaysUpdated(hdmi_hotplug_event);
                }
                break;
            default:
                //do nothing
                break;
            }
        }


	// sending responce to json interface
        void setResponseArray(JsonObject& response, const char* key, const vector<string>& items)
        {
            JsonArray arr;
            for(auto& i : items) arr.Add(JsonValue(i));

            response[key] = arr;

            string json;
            response.ToString(json);
        }
       
	
	// to get connected video displays
        uint32_t PluginTemplate::getConnectedVideoDisplays(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            vector<string> connectedVideoDisplays;
            getConnectedVideoDisplaysHelper(connectedVideoDisplays);
            setResponseArray(response, "connectedVideoDisplays", connectedVideoDisplays);
            returnResponse(true);
        }


        void PluginTemplate::getConnectedVideoDisplaysHelper(vector<string>& connectedDisplays)
        {
            LOGINFO();
            try
            {
                device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                for (size_t i = 0; i < vPorts.size(); i++)
                {
                    device::VideoOutputPort &vPort = vPorts.at(i);
                    if (vPort.isDisplayConnected())
                    {
                        string displayName = vPort.getName();
                        if (strncasecmp(displayName.c_str(), "hdmi", 4)==0)
                        {
                            connectedDisplays.clear();
                            connectedDisplays.emplace_back(displayName);
                            break;
                        }
                        else
                        {
                            vectorSet(connectedDisplays, displayName);
                        }
                    }
                }
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }
        }




	// Begin methods
	uint32_t PluginTemplate::getPluginTemplateStatus(const JsonObject& parameters, JsonObject& response)
        {  
            LOGINFOMETHOD();

            vector<string> plug_status;
	    plug_status.push_back("CONNECTED");
            setResponseArray(response, "connection status from plugin", plug_status);
            returnResponse(true);
        }
 
 
 	uint32_t PluginTemplate::getPluginTemplateList(const JsonObject& parameters, JsonObject& response)
        {   
                LOGINFOMETHOD();
                vector<string> List_plugin;
		List_plugin.push_back ("plug-A");
		List_plugin.push_back ("plug-B");
		List_plugin.push_back ("plug-C");
		List_plugin.push_back ("plug-D");
		List_plugin.push_back ("plug-E");
		
		setResponseArray(response, "supportedSettopResolutions", List_plugin);
		returnResponse(true);
	}

	uint32_t PluginTemplate::getPluginTemplateInfo(const JsonObject& parameters, JsonObject& response)
	{
 	LOGINFOMETHOD();
  	string videoDisplay = parameters.HasLabel("plugin_name") ? parameters["plugin_name"].String() : "plug-A";
  	vector<string> plugin_info;
		plugin_info.push_back ("xyz-plugin");
		plugin_info.push_back ("no:430HT5");
            setResponseArray(response, "supportedTvResolutions", plugin_info);
	    returnResponse(true);
         }


        void PluginTemplate::connectedVideoDisplaysUpdated(int hdmiHotPlugEvent)
        {
            LOGINFO();
            static int previousStatus = HDMI_HOT_PLUG_EVENT_CONNECTED;
            static int firstTime = 1;

            if (firstTime || previousStatus != hdmiHotPlugEvent)
            {
                firstTime = 0;
                JsonArray connectedDisplays;
                if (HDMI_HOT_PLUG_EVENT_CONNECTED == hdmiHotPlugEvent)
                {
                    connectedDisplays.Add("HDMI0");
                }
                else
                {
                    /* notify Empty list on HDMI-output-disconnect hotplug */
                }

                JsonObject params;
                params["connectedVideoDisplays"] = connectedDisplays;
                sendNotify("connectedVideoDisplaysUpdated", params);
            }
            previousStatus = hdmiHotPlugEvent;
        }



    } // namespace Plugin
} // namespace WPEFramework
