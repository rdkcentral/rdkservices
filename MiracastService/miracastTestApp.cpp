#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <signal.h>
#include "rdk_debug.h"
#include <iarmUtil.h>
#include <libIBus.h>

#include <miracast.h>

#define LOG_MIRACAST_APP "LOG.RDK.MIRACAST"

using namespace std;

#if 0
enum
{
    Exit=0,
    Start_WiFi_Display=1
};
#endif

bool job = true, endMain = true;
std::string ongoingJob;
pthread_t threadID;

void *searchThread(void *vargp)
{
    while(job)
    {
        sleep(1); 
        printf("%s ....!\n", (char*)vargp);
    }
}

class Callback_impl : public MiracastCallback
{
    public:

    Callback_impl()
    {
    }

    ~Callback_impl()
    {
    }

    virtual void onMiracastDisabled()
    {
    }
    virtual void onDeviceDiscovery(DeviceInfo* deviceInfo)
    {
        printf("App: New Device found\n");
        printf("Name/MAC - %s/%s\n", deviceInfo->modelName.c_str(), deviceInfo->deviceMAC.c_str());
    } 
    virtual void onDeviceLost(std::string MAC)
    {
        printf("Device lost - %s \n", MAC.c_str()); 
    }
    virtual void onGroupRemoved(std::string reason)
    {
        printf("[%s]Group Removed\n", __FUNCTION__);
    }
    virtual void onGoNegReq(std::string MAC)
    {
        job = false;
        int input = -1;
        //pthread_join(threadID, NULL);
        printf("[%s]Group Neg Request\n", __FUNCTION__);
        printf("Connect request received from %s", MAC.c_str());

        printf("\n==================================================================\n");
        printf("*****	Miracast Test Application to demo screencasting		****** 	  ");
        printf("\n==================================================================\n");
        printf("App:1. Authorize incoming request\n" );
        printf("App:0. Exit." );
        printf("App:\n==================================================================\n");

        printf("App: Selection: " );
        std::cin >> input;
        if(input == 1)
        {
            m_serviceObj->connectDevice(MAC);
            ongoingJob = "Connecting";
            //pthread_create(&threadID, NULL, searchThread, (void*)ongoingJob.c_str()); 
        }
        else
        {
            job = false;
            endMain = false;
        }
    } 
    virtual void onProvisionReq(std::string authType, std::string MAC)
    {
        printf("[%s]Provision Request\n", __FUNCTION__);
    } 
    virtual void onGroupFormationSuccess()
    {
        printf("[%s]Group Formation Success\n", __FUNCTION__);
    } 
    virtual void onGroupStarted()
    {
        printf("[%s]Group Started\n", __FUNCTION__);
        job = false;
        int input = -1;
        //pthread_join(threadID, NULL);
        printf("[%s]P2P Device Connected\n", __FUNCTION__);
        printf("\n==================================================================\n");
        printf("*****	Miracast Test Application to demo screencasting		****** 	  ");
        printf("\n==================================================================\n");
        printf("App:1. Start streaming\n" );
        printf("App:0. Exit." );
        printf("App:\n==================================================================\n");

        printf("App: Selection: " );
        std::cin >> input;
        if(input == 1)
        {
            m_serviceObj->startStreaming();
        }
        else
        {
            job = false;
            endMain = false;
        }
    } 
    virtual void onConnected()
    {
        printf("onConnect event received\n");
    }
    virtual void onStreamingStarted()
    {
        printf("[%s]Starting to Stream\n", __FUNCTION__);
    }
    virtual void onSessionEstablished()
    {
        printf("[%s]Session established\n", __FUNCTION__);
    }
    virtual void onWFDTimeout()
    {
    }
    virtual void onDeviceOff()
    {
    }
    virtual void HDCPStatus(bool status)
    {
    }
    void setServiceobject(MiracastService* obj)
    {
        m_serviceObj = obj;
    }
    virtual void stopService( void )
    {
	    endMain = false;
    }
    private:
        MiracastService* m_serviceObj;
};

void signalHandler(int sig)
{
    printf("Got signal %d before terminating\n", sig);
    system("pkill udhcpd");
    printf("Terminating\n");
    endMain = false;
    return;
}

int main(int argc,char **argv)
{
    int ret = -1, input = -1;
    signal(SIGINT, signalHandler);
/*
//Not required if IARM not used
    if(IARM_Bus_Init("MIRACAST_SRV") != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_MIRACAST_APP, "error initializing IARM Bus!\n");
    }

    if(IARM_Bus_Connect()!= IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_MIRACAST_APP, "error connecting to IARM Bus!\n");
        return 0;
    }   
    printf("IARM initialisations done. Connected to IARM Bus.\n");
*/
    Callback_impl* callback = new Callback_impl();
    MiracastService* obj = MiracastService::create(callback);
    callback->setServiceobject(obj);
#if 0
    printf("\n==================================================================\n");
    printf("*****	Miracast Test Application to demo screencasting		****** 	  ");
    printf("\n==================================================================\n");
    printf("App:1. Start WiFi-Display\n" );
    printf("App:0. Exit." );
    printf("App:\n==================================================================\n");
    printf("App: Selection: " );
    std::cin >> input;

    switch (input)
    {
        case Start_WiFi_Display:
            ret = obj->discoverDevices();
            printf("App: Started Device discovery\n");
            ongoingJob = "Searching";
            //pthread_create(&threadID, NULL, searchThread, (void*)ongoingJob.c_str()); 
            break;
        case Exit:
            printf("App:Exiting..!\n" );
            job = false;
            endMain = false;
            break;
        default:
            printf("App:Invalid input..!\n" );
            break;
    }
/*
    if(ret == -1)
    {
        IARM_Bus_Disconnect();
        IARM_Bus_Term();
        return 0;
    }
*/
#endif
    while( true )
    {
	    printf("\n==================================================================\n");
	    printf("*****       Miracast Test Application to demo screencasting         ******    ");
	    printf("\n==================================================================\n");
	    printf("App:1. Exit.\n" );
	    printf("App:2. Start WiFi-Display\n" );
	    printf("App:3. Stop WiFi-Display\n" );
	    printf("App:\n==================================================================\n");
	    printf("App: Selection: \n" );
	    std::cin >> input;

	    switch (input)
	    {
		    case Start_WiFi_Display:
		    case Stop_WiFi_Display:
		    case Exit:
		    {
			    obj->SendMessage( input );
		    }
		    break;
		    default:
		    {
			    printf("App:Invalid input..!\n" );
		    }
		    break;
	    }

	    if ( Exit == input ){
		    break;
	    }
    }
    
    MiracastService::Destroy(obj);
    callback->setServiceobject(NULL);
    delete callback;

    return 0;
}
