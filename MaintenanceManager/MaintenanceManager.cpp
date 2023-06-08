/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2021 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

/**
 * @file  MaintenanceManager.cpp
 * @author Livin Sunny
 * @brief Thunder Plugin based Implementation for RDK MaintenanceManager service API's.
 * @reference RDK-29959.
 */
#include <stdlib.h>
#include <errno.h>
#include <cstdio>
#include <regex>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <bits/stdc++.h>
#include <algorithm>

#include "MaintenanceManager.h"

#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilscRunScript.h"
#include "UtilsfileExists.h"

enum eRetval { E_NOK = -1,
    E_OK };

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIARM.h"

#endif /* USE_IARMBUS || USE_IARM_BUS */

#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
#include "mfrMgr.h"
#endif

#ifdef ENABLE_DEEP_SLEEP
#include "deepSleepMgr.h"
#endif

#include "maintenanceMGR.h"

using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 19
#define SERVER_DETAILS  "127.0.0.1:9998"


#define PROC_DIR "/proc"
#define MAINTENANCE_MANAGER_RFC_CALLER_ID "MaintenanceManager"
#define TR181_AUTOREBOOT_ENABLE "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"
#define TR181_STOP_MAINTENANCE  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.StopMaintenance.Enable"
#define TR181_RDKVFWUPGRADER  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKFirmwareUpgrader.Enable"
#define TR181_PARTNER_ID "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Bootstrap.PartnerName"
#define TR181_TARGET_PROPOSITION "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Bootstrap.TargetProposition"
#define TR181_XCONFURL "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Bootstrap.XconfUrl"

string notifyStatusToString(Maint_notify_status_t &status)
{
    string ret_status="";
    switch(status){
        case MAINTENANCE_IDLE:
            ret_status="MAINTENANCE_IDLE";
            break;
        case MAINTENANCE_STARTED:
            ret_status="MAINTENANCE_STARTED";
            break;
        case MAINTENANCE_ERROR:
            ret_status="MAINTENANCE_ERROR";
            break;
        case MAINTENANCE_COMPLETE:
            ret_status="MAINTENANCE_COMPLETE";
            break;
        case MAINTENANCE_INCOMPLETE:
            ret_status="MAINTENANCE_INCOMPLETE";
            break;
        default:
            ret_status="MAINTENANCE_ERROR";
    }
    return ret_status;
}

bool checkValidOptOutModes(string OptoutModes){
    vector<string> modes{
        "ENFORCE_OPTOUT",
        "BYPASS_OPTOUT",
        "IGNORE_UPDATE",
        "NONE"
    };

    if ( find( modes.begin(), modes.end(), OptoutModes) != modes.end() ){
        return true;
    }
    else {
        return false;
    }
}

string moduleStatusToString(IARM_Maint_module_status_t &status)
{
    string ret_status="";
    switch(status){
        case MAINT_DCM_COMPLETE:
            ret_status="MAINTENANCE_DCM_COMPLETE";
            break;
        case MAINT_DCM_ERROR:
            ret_status="MAINTENANCE_DCM_ERROR";
            break;
        case MAINT_RFC_COMPLETE:
            ret_status="MAINTENANCE_RFC_COMPLETE";
            break;
        case MAINT_RFC_ERROR:
            ret_status="MAINTENANCE_RFC_ERROR";
            break;
        case MAINT_LOGUPLOAD_COMPLETE:
            ret_status="MAINTENANCE_LOGUPLOAD_COMPLETE";
            break;
        case MAINT_LOGUPLOAD_ERROR:
            ret_status="MAINTENANCE_LOGUPLOAD_ERROR";
            break;
        case MAINT_PINGTELEMETRY_COMPLETE:
            ret_status="MAINTENANCE_PINGTELEMETRY_COMPLETE";
            break;
        case MAINT_PINGTELEMETRY_ERROR:
            ret_status="MAINTENANCE_PINGTELEMETRY_ERROR";
            break;
        case MAINT_FWDOWNLOAD_COMPLETE:
            ret_status="MAINTENANCE_FWDOWNLOAD_COMPLETE";
            break;
        case MAINT_FWDOWNLOAD_ERROR:
            ret_status="MAINTENANCE_FWDOWNLOAD_ERROR";
            break;
        case MAINT_REBOOT_REQUIRED:
            ret_status="MAINTENANCE_REBOOT_REQUIRED";
            break;
        case MAINT_FWDOWNLOAD_ABORTED:
            ret_status="MAINTENANCE_FWDOWNLOAD_ABORTED";
            break;
        case MAINT_CRITICAL_UPDATE:
            ret_status="MAINTENANCE_CRITICAL_UPDATE";
            break;
        default:
            ret_status="MAINTENANCE_EMPTY";
    }
    return ret_status;
}
/**
 * @brief WPEFramework class for Maintenance Manager
 */
namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::MaintenanceManager> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin {

        namespace {
            // MaintenanceManager should use interfaces

            uint32_t getServiceState(PluginHost::IShell* shell, const string& callsign, PluginHost::IShell::state& state)
            {
                uint32_t result;
                auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IShell for " << callsign << std::endl;
                } else {
                    result = Core::ERROR_NONE;
                    state = interface->State();
                    std::cout << "IShell state " << state << " for " << callsign << std::endl;
                    interface->Release();
                }
                return result;
            }
        }

        //Prototypes
        SERVICE_REGISTRATION(MaintenanceManager, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);
        /* Global time variable */
        MaintenanceManager* MaintenanceManager::_instance = nullptr;

        cSettings MaintenanceManager::m_setting(MAINTENANCE_MGR_RECORD_FILE);

        string task_names_foreground[]={
            "/lib/rdk/StartDCM_maintaince.sh",
            "/lib/rdk/RFCbase.sh",
            "/lib/rdk/swupdate_utility.sh >> /opt/logs/swupdate.log",
            "/lib/rdk/Start_uploadSTBLogs.sh"
        };

        vector<string> tasks;

        string script_names[]={
            "DCMscript_maintaince.sh",
            "RFCbase.sh",
            "swupdate_utility.sh",
            "uploadSTBLogs.sh"
        };

        string deviceInitializationContext[] = {
            "partnerId",
            "targetProposition",
            "regionalConfigService"
        };

        /**
         * Register MaintenanceManager module as wpeframework plugin
         */
        MaintenanceManager::MaintenanceManager()
            :PluginHost::JSONRPC()
        {
            MaintenanceManager::_instance = this;

            /**
             * @brief Invoking Plugin API register to WPEFRAMEWORK.
             */
#ifdef DEBUG
            Register("sampleMaintenanceManagerAPI", &MaintenanceManager::sampleAPI, this);
#endif /* DEBUG */
            Register("getMaintenanceActivityStatus", &MaintenanceManager::getMaintenanceActivityStatus,this);
            Register("getMaintenanceStartTime", &MaintenanceManager::getMaintenanceStartTime,this);
            Register("setMaintenanceMode", &MaintenanceManager::setMaintenanceMode,this);
            Register("startMaintenance", &MaintenanceManager::startMaintenance,this);
            Register("stopMaintenance", &MaintenanceManager::stopMaintenance,this);
            Register("getMaintenanceMode", &MaintenanceManager::getMaintenanceMode,this);


            MaintenanceManager::m_task_map[task_names_foreground[0].c_str()]=false;
            MaintenanceManager::m_task_map[task_names_foreground[1].c_str()]=false;
            MaintenanceManager::m_task_map[task_names_foreground[2].c_str()]=false;
            MaintenanceManager::m_task_map[task_names_foreground[3].c_str()]=false;

            MaintenanceManager::m_param_map[deviceInitializationContext[0].c_str()] = TR181_PARTNER_ID;
            MaintenanceManager::m_param_map[deviceInitializationContext[1].c_str()] = TR181_TARGET_PROPOSITION;
            MaintenanceManager::m_param_map[deviceInitializationContext[2].c_str()] = TR181_XCONFURL;

            MaintenanceManager::m_paramType_map[deviceInitializationContext[0].c_str()] = DATA_TYPE::WDMP_STRING;
            MaintenanceManager::m_paramType_map[deviceInitializationContext[1].c_str()] = DATA_TYPE::WDMP_STRING;
            MaintenanceManager::m_paramType_map[deviceInitializationContext[2].c_str()] = DATA_TYPE::WDMP_STRING;
         }

        void MaintenanceManager::task_execution_thread(){
            uint8_t i=0;
            string cmd="";
            bool internetConnectStatus=false;

            LOGINFO("Executing Maintenance tasks");
            m_statusMutex.lock();
            MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_STARTED);
            m_statusMutex.unlock();

            /* cleanup if not empty */
            if(!tasks.empty()){
                tasks.erase (tasks.begin(),tasks.end());
            }

            /* Controlled by CFLAGS */
#if defined(SUPPRESS_MAINTENANCE)
            bool activationStatus=false;
            bool skipFirmwareCheck=false;

            /* Activation check */
            activationStatus = getActivatedStatus(skipFirmwareCheck);

            /* we proceed with network check only if
             * "activation-connect", "activation-ready"
             * "not-activated", "activated" */
            if(activationStatus){
                /* Network check */
                internetConnectStatus = isDeviceOnline();
            }
#else
            internetConnectStatus = isDeviceOnline();
#endif

#if defined(ENABLE_WHOAMI)
    if (UNSOLICITED_MAINTENANCE == g_maintenance_type) {
        /* WhoAmI check*/
        bool whoAmIStatus = knowWhoAmI();
        if (whoAmIStatus) {
            LOGINFO("knowWhoAmI() returned successfully");
        }
    }
#endif

            if ( false == internetConnectStatus ) {
                m_statusMutex.lock();
                MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_ERROR);
                m_statusMutex.unlock();
                LOGINFO("Maintenance is exiting as device is not connected to internet.");
                if (UNSOLICITED_MAINTENANCE == g_maintenance_type && !g_unsolicited_complete){
                    g_unsolicited_complete = true;
                }
                return;
            }

            LOGINFO("Reboot_Pending :%s",g_is_reboot_pending.c_str());

            if (UNSOLICITED_MAINTENANCE == g_maintenance_type){
                LOGINFO("---------------UNSOLICITED_MAINTENANCE--------------");
#ifndef ENABLE_WHOAMI
                tasks.push_back(task_names_foreground[0].c_str());
#endif
            }
            else if( SOLICITED_MAINTENANCE == g_maintenance_type){
                LOGINFO("=============SOLICITED_MAINTENANCE===============");
            }

#if defined(ENABLE_WHOAMI)
            if (UNSOLICITED_MAINTENANCE == g_maintenance_type) {
                tasks.push_back(task_names_foreground[1].c_str());
                tasks.push_back(task_names_foreground[2].c_str());
                tasks.push_back(task_names_foreground[0].c_str());
                tasks.push_back(task_names_foreground[3].c_str());
            } else {
                tasks.push_back(task_names_foreground[1].c_str());
                tasks.push_back(task_names_foreground[2].c_str());
                tasks.push_back(task_names_foreground[3].c_str());
            }
#elif defined(SUPPRESS_MAINTENANCE)
            /* decide which all tasks are needed based on the activation status */
            if (activationStatus){
                if(skipFirmwareCheck){
                    /* set the task status of swupdate */
                    SET_STATUS(g_task_status,DIFD_SUCCESS);
                    SET_STATUS(g_task_status,DIFD_COMPLETE);

                    /* Add tasks */
                    tasks.push_back(task_names_foreground[1].c_str());
                    tasks.push_back(task_names_foreground[3].c_str());
                }else{
                    tasks.push_back(task_names_foreground[1].c_str());
                    tasks.push_back(task_names_foreground[2].c_str());
                    tasks.push_back(task_names_foreground[3].c_str());
                }
            }
#else
            tasks.push_back(task_names_foreground[1].c_str());
            tasks.push_back(task_names_foreground[2].c_str());
            tasks.push_back(task_names_foreground[3].c_str());
#endif
            std::unique_lock<std::mutex> lck(m_callMutex);

            for( i = 0; i < tasks.size() && !m_abort_flag; i++) {
                cmd = tasks[i];
                cmd += " &";
                cmd += "\0";
                m_task_map[tasks[i]] = true;

                if ( !m_abort_flag ){
                    LOGINFO("Starting Script (SM) :  %s \n",cmd.c_str());
                    system(cmd.c_str());

                    LOGINFO("Waiting to unlock.. [%d/%d]",i+1,tasks.size());
                    task_thread.wait(lck);
                }
            }

	    m_abort_flag=false;
            LOGINFO("Worker Thread Completed");
        }

        bool MaintenanceManager::knowWhoAmI()
        {
            bool success = false;
            int retryDelay = 10;
            int retryCount = 0;
            const char* secMgr_callsign = "org.rdk.SecManager";
            const char* secMgr_callsign_ver = "org.rdk.SecManager.1";
            PluginHost::IShell::state state;
            WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>* thunder_client = nullptr;

            do {

                if ((getServiceState(m_service, secMgr_callsign, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED)) {
                    LOGINFO("%s is active", secMgr_callsign);

                    thunder_client=getThunderPluginHandle(secMgr_callsign_ver);
                    if (thunder_client == nullptr) {
                        LOGINFO("Failed to get plugin handle");
                    } else {
                        JsonObject params;
                        JsonObject joGetResult;

                        thunder_client->Invoke<JsonObject, JsonObject>(5000, "getDeviceInitializationContext", params, joGetResult);
                        if (joGetResult.HasLabel("success") && joGetResult["success"].Boolean()) {
                            if (joGetResult.HasLabel("partnerProvisioningContext")) {
                                JsonObject getProvisioningContext = joGetResult["partnerProvisioningContext"].Object();
                                int size = (int)(sizeof(deviceInitializationContext)/sizeof(deviceInitializationContext[0]));
                                for (int idx=0; idx < size; idx++) {
                                    const char* key = deviceInitializationContext[idx].c_str();

                                    // Retrive partnerProvisioningContext Value
                                    string paramValue = getProvisioningContext[key].String();

                                    if (!paramValue.empty()) {
                                        if (strcmp(key, "regionalConfigService") == 0) {
                                            paramValue = "https://" + paramValue;
                                        }
                                        LOGINFO("[partnerProvisioningContext] %s : %s", key, paramValue.c_str());

                                        // Retrieve tr181 parameter from m_param_map
                                        string rfc_parameter = m_param_map[key];

                                        //  Retrieve parameter data type from m_paramType_map
                                        DATA_TYPE rfc_dataType = m_paramType_map[key];

                                        // Set the RFC values for partnerProvisioningContext parameters
                                        setRFC(rfc_parameter.c_str(), paramValue.c_str(), rfc_dataType);
                                    } else {
                                        LOGINFO("Not able to fetch %s value from partnerProvisioningContext", key);
                                    }
                                }
                                success = true;
                            } else {
                                LOGINFO("partnerProvisioningContext is not available in the response");
                            }
                        } else {
                            // Get retryDelay value and sleep for that much seconds
                            if (joGetResult.HasLabel("retryDelay")) {
                                retryDelay = joGetResult["retryDelay"].Number();
                            }
                            LOGINFO("getDeviceInitializationContext failed");
                        }
                    }
                } else {
                    LOGINFO("%s is not active", secMgr_callsign);
                }

		retryCount++;
                if (retryCount == 4 && !success) {
                    if (checkActivatedStatus() == "activated") {
                        LOGINFO("Device is already activated. Exiting from knowWhoAmI()");
                        success = true;
                    }
                }

		if (!success) {
                    LOGINFO("Retrying in %d seconds", retryDelay);
                    sleep(retryDelay);
                }

            } while (!success);
            return success;
        }

        // Thunder plugin communication
        WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>* MaintenanceManager::getThunderPluginHandle(const char* callsign)
        {
            string token;
            WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>* thunder_client = nullptr;

            auto security = m_service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr) {
                string payload = "http://localhost";
                if (security->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t*>(payload.c_str()),
                        token)
                    == Core::ERROR_NONE) {
                    std::cout << "MaintenanceManager got security token" << std::endl;
                } else {
                    std::cout << "MaintenanceManager failed to get security token" << std::endl;
                }
                security->Release();
            } else {
                std::cout << "No security agent" << std::endl;
            }

            string query = "token=" + token;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
            thunder_client = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(callsign, "", false, query);
            return thunder_client;
        }

        bool MaintenanceManager::setRFC(const char* rfc, const char* value, DATA_TYPE dataType)
        {
            bool result = false;
            WDMP_STATUS status;
            status = setRFCParameter((char *)MAINTENANCE_MANAGER_RFC_CALLER_ID, rfc, value, dataType);

            if ( WDMP_SUCCESS == status ){
                LOGINFO("Successfuly set the tr181 parameter %s with value %s", rfc, value);
                result = true;
            } else {
                LOGINFO("Failed setting %s parameter", rfc);
            }
            return result;
        }

        const string MaintenanceManager::checkActivatedStatus()
        {
            JsonObject joGetParams;
            JsonObject joGetResult;
            std::string callsign = "org.rdk.AuthService.1";
            uint8_t i = 0;
            std::string ret_status("invalid");

            /* check if plugin active */
            PluginHost::IShell::state state = PluginHost::IShell::state::UNAVAILABLE;
            if ((getServiceState(m_service, "org.rdk.AuthService", state) != Core::ERROR_NONE) || (state != PluginHost::IShell::state::ACTIVATED)) {
                LOGINFO("AuthService plugin is not activated.Retrying.. \n");
                //if plugin is not activated we need to retry
                do{
                    if ((getServiceState(m_service, "org.rdk.AuthService", state) != Core::ERROR_NONE) || (state != PluginHost::IShell::state::ACTIVATED)) {
                        sleep(10);
                        i++;
                        LOGINFO("AuthService retries [%d/4] \n",i);
                    }
                    else{
                        break;
                    }
                }while( i < MAX_ACTIVATION_RETRIES );

                if (state != PluginHost::IShell::state::ACTIVATED){
                    LOGINFO("AuthService plugin is Still not active");
                    return ret_status;
                }
                else{
                    LOGINFO("AuthService plugin is Now active");
                }
            }
            if (state == PluginHost::IShell::state::ACTIVATED){
                LOGINFO("AuthService is active");
            }

            string token;

            // TODO: use interfaces and remove token
            auto security = m_service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr) {
                string payload = "http://localhost";
                if (security->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t*>(payload.c_str()),
                        token)
                    == Core::ERROR_NONE) {
                    std::cout << "MaintenanceManager got security token" << std::endl;
                } else {
                    std::cout << "MaintenanceManager failed to get security token" << std::endl;
                }
                security->Release();
            } else {
                std::cout << "No security agent" << std::endl;
            }

            string query = "token=" + token;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
            auto thunder_client = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(callsign.c_str(), "", false, query);
            if (thunder_client != nullptr) {
                uint32_t status = thunder_client->Invoke<JsonObject, JsonObject>(5000, "getActivationStatus", joGetParams, joGetResult);
                LOGINFO("Invoke status : %d",status);
                if (status > 0) {
                    LOGINFO("%s call failed %d", callsign.c_str(), status);
                    ret_status = "invalid";
                    LOGINFO("Setting Default to [%s]",ret_status.c_str());
                } else if (joGetResult.HasLabel("status")) {
                    ret_status = joGetResult["status"].String();
                    LOGINFO("Activation Value [%s]",ret_status.c_str());
                }
                else {
                    LOGINFO("Failed to read the ActivationStatus");
                    ret_status = "invalid";
                }

                return ret_status;
            }

            LOGINFO("thunder client failed");
            return ret_status;
        }

        bool MaintenanceManager::getActivatedStatus(bool &skipFirmwareCheck)
        {
            /* activation-connect, activation ready, not-activated - execute all except DIFD
             * activation disconnect - dont run maintenance
             * activated - run normal */
            bool ret_result=false;
            string activationStatus;
            Auth_activation_status_t result;
            const std::unordered_map<std::string,std::function<void()>> act{
                {"activation-connect",   [&](){ result = ACTIVATION_CONNECT; }},
                    {"activation-ready",   [&](){ result = ACTIVATION_READY; }},
                    {"not-activated",   [&](){ result = NOT_ACTIVATED; }},
                    {"activation-disconnect", [&](){ result = ACTIVATION_DISCONNECT; }},
                    {"activated", [&](){ result = ACTIVATED; }},
            };

            activationStatus = checkActivatedStatus();
            LOGINFO("activation status : [ %s ]",activationStatus.c_str());
            const auto end = act.end();
            auto search = act.find(activationStatus);
            if ( search != end ){
                search->second();
            }
            else{
                result = INVALID_ACTIVATION;
                LOGINFO("result: invalid Activation");
            }

            switch(result){
                case ACTIVATED:
                    ret_result = true;
                    break;
                case ACTIVATION_DISCONNECT:
                    ret_result = false;
                    break;
                case NOT_ACTIVATED:
                case ACTIVATION_READY:
                case ACTIVATION_CONNECT:
                    ret_result = true;
                    skipFirmwareCheck = true;
                default:
                    ret_result = true;
            }

            LOGINFO("ret_result: [%s] skipFirmwareCheck:[%s]"
                    ,(ret_result)? "true":"false",(skipFirmwareCheck)?"true":"false");
            return ret_result;
        }

        bool MaintenanceManager::checkNetwork()
        {
            JsonObject joGetParams;
            JsonObject joGetResult;
            std::string callsign = "org.rdk.Network.1";

            string token;

            // TODO: use interfaces and remove token
            auto security = m_service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr) {
                string payload = "http://localhost";
                if (security->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t*>(payload.c_str()),
                        token)
                    == Core::ERROR_NONE) {
                    std::cout << "MaintenanceManager got security token" << std::endl;
                } else {
                    std::cout << "MaintenanceManager failed to get security token" << std::endl;
                }
                security->Release();
            } else {
                std::cout << "No security agent" << std::endl;
            }

            string query = "token=" + token;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
            auto thunder_client = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(callsign.c_str(), "", false, query);
            if (thunder_client != nullptr) {
                uint32_t status = thunder_client->Invoke<JsonObject, JsonObject>(5000, "isConnectedToInternet", joGetParams, joGetResult);
                if (status > 0) {
                    LOGINFO("%s call failed %d", callsign.c_str(), status);
                    return false;
                } else if (joGetResult.HasLabel("connectedToInternet")) {
                    LOGINFO("connectedToInternet status %s",(joGetResult["connectedToInternet"].Boolean())? "true":"false");
                    return joGetResult["connectedToInternet"].Boolean();
                } else {
                    return false;
                }
            }

            LOGINFO("thunder client failed");
            return false;
        }

        bool MaintenanceManager::isDeviceOnline()
        {
            bool network_available =  false;
            LOGINFO("Checking device has network connectivity\n");

            /* add 4 checks every 30 seconds */
            int i=0;
            do{
                network_available = checkNetwork();
                if ( !network_available ){
                    sleep(30);
                    i++;
                    LOGINFO("Network retries [%d/4] \n",i);
                }else{
                    break;
                }
            }while( i < MAX_NETWORK_RETRIES );

            if ( network_available ){
                return true;
            }else {
                return false;
            }
        }

        MaintenanceManager::~MaintenanceManager()
        {
            MaintenanceManager::_instance = nullptr;
        }

        const string MaintenanceManager::Initialize(PluginHost::IShell* service)
        {
            ASSERT(service != nullptr);
            ASSERT(m_service == nullptr);

            m_service = service;
            m_service->AddRef();

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            InitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

            /* On Success; return empty to indicate no error text. */
            return (string());
        }

        void MaintenanceManager::Deinitialize(PluginHost::IShell* service)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            stopMaintenanceTasks();
            DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

            ASSERT(service == m_service);

            m_service->Release();
            m_service = nullptr;
        }

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        void MaintenanceManager::InitializeIARM()
        {
            if (Utils::IARM::init()) {
                IARM_Result_t res;
                // Register for the Maintenance Notification Events
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, _MaintenanceMgrEventHandler));
                //Register for setMaintenanceStartTime
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_DCM_NEW_START_TIME_EVENT,_MaintenanceMgrEventHandler));

                maintenanceManagerOnBootup();
            }
        }

        void MaintenanceManager::maintenanceManagerOnBootup() {
            /* on boot up we set these things */
            MaintenanceManager::g_currentMode = FOREGROUND_MODE;

            MaintenanceManager::m_notify_status=MAINTENANCE_IDLE;
            MaintenanceManager::g_epoch_time="";

            /* to know the maintenance is solicited or unsolicited */
            g_maintenance_type=UNSOLICITED_MAINTENANCE;
            LOGINFO("Triggering Maintenance on bootup");

            /* On bootup we check for opt-out value
             * if empty set the value to none */
            string OptOutmode = "NONE";
            OptOutmode = m_setting.getValue("softwareoptout").String();
            if(!checkValidOptOutModes(OptOutmode)){
                LOGINFO("OptOut Value is not Set. Setting to NONE \n");
                m_setting.remove("softwareoptout");
                OptOutmode = "NONE";
                m_setting.setValue("softwareoptout",OptOutmode);
            }
            else {
                LOGINFO("OptOut Value Found as: %s \n", OptOutmode.c_str());
            }

            MaintenanceManager::g_is_critical_maintenance="false";
            MaintenanceManager::g_is_reboot_pending="false";
            MaintenanceManager::g_lastSuccessful_maint_time="";
            MaintenanceManager::g_task_status=0;
            MaintenanceManager::m_abort_flag=false;
            MaintenanceManager::g_unsolicited_complete = false;

            /* we post just to tell that we are in idle at this moment */
            m_statusMutex.lock();
            MaintenanceManager::_instance->onMaintenanceStatusChange(m_notify_status);
            m_statusMutex.unlock();

            m_thread = std::thread(&MaintenanceManager::task_execution_thread, _instance);
        }

        void MaintenanceManager::_MaintenanceMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (MaintenanceManager::_instance){
                LOGWARN("IARM event Received with %d !", eventId);
                MaintenanceManager::_instance->iarmEventHandler(owner, eventId, data, len);
            }
            else
                LOGWARN("WARNING - cannot handle IARM events without MaintenanceManager plugin instance!");
        }

        void MaintenanceManager::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
          m_statusMutex.lock();
          if ( !m_abort_flag ){
            Maint_notify_status_t notify_status=MAINTENANCE_STARTED;
            IARM_Bus_MaintMGR_EventData_t *module_event_data=(IARM_Bus_MaintMGR_EventData_t*)data;
            IARM_Maint_module_status_t module_status;
            time_t successfulTime;
            string str_successfulTime="";
            auto task_status_DCM=m_task_map.find(task_names_foreground[0].c_str());
            auto task_status_RFC=m_task_map.find(task_names_foreground[1].c_str());
            auto task_status_FWDLD=m_task_map.find(task_names_foreground[2].c_str());
            auto task_status_LOGUPLD=m_task_map.find(task_names_foreground[3].c_str());

            IARM_Bus_MaintMGR_EventId_t event = (IARM_Bus_MaintMGR_EventId_t)eventId;
            LOGINFO("Maintenance Event-ID = %d \n",event);

            if (!strcmp(owner, IARM_BUS_MAINTENANCE_MGR_NAME)) {
                if ( IARM_BUS_DCM_NEW_START_TIME_EVENT == eventId ) {
                    /* we got a new start time from DCM script */
                    string l_time(module_event_data->data.startTimeUpdate.start_time);
                    LOGINFO("DCM_NEW_START_TIME_EVENT Start Time %s \n", l_time.c_str());
                    /* Store it in a Global structure */
                    g_epoch_time=l_time;
                }
                else if (( IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE == eventId ) &&
                    ( MAINTENANCE_STARTED == m_notify_status )) {
                    module_status = module_event_data->data.maintenance_module_status.status;
                    LOGINFO("MaintMGR Status %d \n",module_status);
                    string status_string=moduleStatusToString(module_status);
                    LOGINFO("MaintMGR Status %s \n", status_string.c_str());
                    switch (module_status) {
                        case MAINT_RFC_COMPLETE :
                            if(task_status_RFC->second != true) {
                                 LOGINFO("Ignoring Event RFC_COMPLETE");
                                 break;
                            }
                            else {
                                 SET_STATUS(g_task_status,RFC_SUCCESS);
                                 SET_STATUS(g_task_status,RFC_COMPLETE);
                                 task_thread.notify_one();
                                 m_task_map[task_names_foreground[1].c_str()]=false;
                            }
                            break;
                        case MAINT_DCM_COMPLETE :
                            if(task_status_DCM->second != true) {
                                 LOGINFO("Ignoring Event DCM_COMPLETE");
                                 break;
                            }
                            else {
                                SET_STATUS(g_task_status,DCM_SUCCESS);
                                SET_STATUS(g_task_status,DCM_COMPLETE);
                                task_thread.notify_one();
                                m_task_map[task_names_foreground[0].c_str()]=false;
                            }
                            break;
                        case MAINT_FWDOWNLOAD_COMPLETE :
                            if(task_status_FWDLD->second != true) {
                                 LOGINFO("Ignoring Event MAINT_FWDOWNLOAD_COMPLETE");
                                 break;
                            }
                            else {
                                SET_STATUS(g_task_status,DIFD_SUCCESS);
                                SET_STATUS(g_task_status,DIFD_COMPLETE);
                                task_thread.notify_one();
                                m_task_map[task_names_foreground[2].c_str()]=false;
                            }
                            break;
                       case MAINT_LOGUPLOAD_COMPLETE :
                            if(task_status_LOGUPLD->second != true) {
                                 LOGINFO("Ignoring Event MAINT_LOGUPLOAD_COMPLETE");
                                 break;
                            }
                            else {
                                SET_STATUS(g_task_status,LOGUPLOAD_SUCCESS);
                                SET_STATUS(g_task_status,LOGUPLOAD_COMPLETE);
                                task_thread.notify_one();
                                m_task_map[task_names_foreground[3].c_str()]=false;
                            }

                            break;
                        case MAINT_REBOOT_REQUIRED :
                            SET_STATUS(g_task_status,REBOOT_REQUIRED);
                            g_is_reboot_pending="true";
                            break;
                        case MAINT_CRITICAL_UPDATE:
                            g_is_critical_maintenance="true";
                            break;
                        case MAINT_FWDOWNLOAD_ABORTED:
                            SET_STATUS(g_task_status,TASK_SKIPPED);
                            /* we say FW update task complete */
                            SET_STATUS(g_task_status,DIFD_COMPLETE);
                            task_thread.notify_one();
                            m_task_map[task_names_foreground[2].c_str()]=false;
                            LOGINFO("FW Download task aborted \n");
                            break;
                        case MAINT_DCM_ERROR:
                            if(task_status_DCM->second != true) {
                                 LOGINFO("Ignoring Event DCM_ERROR");
                                 break;
                            }
                            else {
                                SET_STATUS(g_task_status,DCM_COMPLETE);
                                task_thread.notify_one();
                                LOGINFO("Error encountered in DCM script task \n");
                                m_task_map[task_names_foreground[0].c_str()]=false;
                            }
                            break;
                        case MAINT_RFC_ERROR:
                            if(task_status_RFC->second != true) {
                                 LOGINFO("Ignoring Event RFC_ERROR");
                                 break;
                            }
                            else {
                                 SET_STATUS(g_task_status,RFC_COMPLETE);
                                 task_thread.notify_one();
                                 LOGINFO("Error encountered in RFC script task \n");
                                 m_task_map[task_names_foreground[1].c_str()]=false;
                            }

                            break;
                        case MAINT_LOGUPLOAD_ERROR:
                            if(task_status_LOGUPLD->second != true) {
                                  LOGINFO("Ignoring Event MAINT_LOGUPLOAD_ERROR");
                                  break;
                            }
                            else {
                                SET_STATUS(g_task_status,LOGUPLOAD_COMPLETE);
                                task_thread.notify_one();
                                LOGINFO("Error encountered in LOGUPLOAD script task \n");
                                m_task_map[task_names_foreground[3].c_str()]=false;
                            }

                            break;
                       case MAINT_FWDOWNLOAD_ERROR:
                            if(task_status_FWDLD->second != true) {
                                 LOGINFO("Ignoring Event MAINT_FWDOWNLOAD_ERROR");
                                 break;
                            }
                            else {
                                SET_STATUS(g_task_status,DIFD_COMPLETE);
                                task_thread.notify_one();
                                LOGINFO("Error encountered in SWUPDATE script task \n");
                                m_task_map[task_names_foreground[2].c_str()]=false;
                            }
                            break;
                       case MAINT_DCM_INPROGRESS:
                            m_task_map[task_names_foreground[0].c_str()]=true;
                            /*will be set to false once COMEPLETE/ERROR received for DCM*/
                            LOGINFO(" DCM already IN PROGRESS -> setting m_task_map of DCM to true \n");
                            break;
                       case MAINT_RFC_INPROGRESS:
                            m_task_map[task_names_foreground[1].c_str()]=true;
                            /*will be set to false once COMEPLETE/ERROR received for RFC*/
                            LOGINFO(" RFC already IN PROGRESS -> setting m_task_map of RFC to true \n");
                            break;
                       case MAINT_FWDOWNLOAD_INPROGRESS:
                            m_task_map[task_names_foreground[2].c_str()]=true;
                            /*will be set to false once COMEPLETE/ERROR received for FWDOWNLOAD*/
                            LOGINFO(" FWDOWNLOAD already IN PROGRESS -> setting m_task_map of FWDOWNLOAD to true \n");
                            break;
                       case MAINT_LOGUPLOAD_INPROGRESS:
                            m_task_map[task_names_foreground[3].c_str()]=true;
                            /*will be set to false once COMEPLETE/ERROR received for LOGUPLOAD*/
                            LOGINFO(" LOGUPLOAD already IN PROGRESS -> setting m_task_map of LOGUPLOAD to true \n");
                            break;
                        default:
                            break;
                    }
                }
                else{
                    LOGINFO("Ignoring/Unknown Maintenance Status!!");
                    m_statusMutex.unlock();
                    return;
                }

                LOGINFO(" BITFIELD Status : %x",g_task_status);
                /* Send the updated status only if all task completes execution
                 * until that we say maintenance started */
                if ( (g_task_status & TASKS_COMPLETED ) == TASKS_COMPLETED ){
                    if ( (g_task_status & ALL_TASKS_SUCCESS) == ALL_TASKS_SUCCESS ){ // all tasks success
                        LOGINFO("DBG:Maintenance Successfully Completed!!");
                        notify_status=MAINTENANCE_COMPLETE;
                        /*  we store the time in persistant location */
                        successfulTime=time(nullptr);
                        tm ltime=*localtime(&successfulTime);
                        time_t epoch_time=mktime(&ltime);
                        str_successfulTime=to_string(epoch_time);
                        LOGINFO("last succesful time is :%s", str_successfulTime.c_str());
                        /* Remove any old completion time */
                        m_setting.remove("LastSuccessfulCompletionTime");
                        m_setting.setValue("LastSuccessfulCompletionTime",str_successfulTime);

                    }
                    /* Check other than all success case which means we have errors */
                    else if ((g_task_status & ALL_TASKS_SUCCESS)!= ALL_TASKS_SUCCESS) {
                        if ((g_task_status & MAINTENANCE_TASK_SKIPPED ) == MAINTENANCE_TASK_SKIPPED ){
                            LOGINFO("DBG:There are Skipped Task. Maintenance Incomplete");
                            notify_status=MAINTENANCE_INCOMPLETE;
                        }
                        else {
                            LOGINFO("DBG:Maintenance Ended with Errors");
                            notify_status=MAINTENANCE_ERROR;
                        }

                    }

                    LOGINFO("ENDING MAINTENANCE CYCLE");
                    if(m_thread.joinable()){
                        m_thread.join();
                        LOGINFO("Thread joined successfully\n");
                    }

                    if ( g_maintenance_type == UNSOLICITED_MAINTENANCE && !g_unsolicited_complete) {
                        g_unsolicited_complete = true;
                    }
                    MaintenanceManager::_instance->onMaintenanceStatusChange(notify_status);
                }
                else {
                    LOGINFO("Tasks are not completed!!!!");

                }
            }
            else {
                LOGWARN("Ignoring unexpected event - owner: %s, eventId: %d!!", owner, eventId);
            }
          }
          else {
              LOGINFO("Maintenance has been aborted. Hence ignoring the event");
          }
          m_statusMutex.unlock();
        }
        void MaintenanceManager::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected()){
                IARM_Result_t res;
                IARM_CHECK(IARM_Bus_RemoveEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, _MaintenanceMgrEventHandler));
                IARM_CHECK(IARM_Bus_RemoveEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_DCM_NEW_START_TIME_EVENT, _MaintenanceMgrEventHandler));
                MaintenanceManager::_instance = nullptr;
            }
        }
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

#ifdef DEBUG
        /**
         * @brief : sampleAPI
         */
        uint32_t MaintenanceManager::sampleAPI(const JsonObject& parameters,
                JsonObject& response)
        {
            response["sampleAPI"] = "Success";
            /* Kept for debug purpose/future reference. */
            sendNotify(EVT_ONMAINTMGRSAMPLEEVENT, parameters);
            returnResponse(true);
        }
#endif /* DEBUG */

        /*
         * @brief This function returns the status of the current
         * or previous maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceActivityStatus",
         *                  "params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"status":MAINTENANCE_IDLE,"LastSuccessfulCompletionTime":
                              -1,"isCriticalMaintenance":true,"isRebootPending":true}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::getMaintenanceActivityStatus(const JsonObject& parameters,
                JsonObject& response)
                {
                    bool result = false;
                    string isCriticalMaintenance = "false";
                    string isRebootPending = "false";
                    string LastSuccessfulCompletionTime = "NA"; /* TODO : check max size to hold this */
                    string getMaintenanceStatusString = "\0";
                    bool b_criticalMaintenace=false;
                    bool b_rebootPending=false;

                    std::lock_guard<std::mutex> guard(m_callMutex);

                    /* Check if we have a critical maintenance */
                    if (!g_is_critical_maintenance.empty()){
                        isCriticalMaintenance=g_is_critical_maintenance;
                    }

                    if (!g_is_reboot_pending.empty()){
                        isRebootPending=g_is_reboot_pending;
                    }

                    /* Get the last SuccessfulCompletion time from Persistant location */
                    if (m_setting.contains("LastSuccessfulCompletionTime")){
                        LastSuccessfulCompletionTime=m_setting.getValue("LastSuccessfulCompletionTime").String();
                    }

                    if (!isCriticalMaintenance.compare("true")){
                        b_criticalMaintenace=true;
                    }

                    if(!isRebootPending.compare("true")){
                        b_rebootPending=true;
                    }

                    response["maintenanceStatus"] = notifyStatusToString(m_notify_status);
                    if(strcmp("NA",LastSuccessfulCompletionTime.c_str())==0)
                    {
                       response["LastSuccessfulCompletionTime"] = 0;  // stoi is not able handle "NA"
                    }
                    else
                    {
                       try{
                               response["LastSuccessfulCompletionTime"] = stoi(LastSuccessfulCompletionTime.c_str());
                          }
                       catch(exception &err)
                          {
                              //exception caught with stoi -- So making "LastSuccessfulCompletionTime" as 0
                              response["LastSuccessfulCompletionTime"] = 0;
                          }
                    }
                    response["isCriticalMaintenance"] = b_criticalMaintenace;
                    response["isRebootPending"] = b_rebootPending;
                    result = true;

                    returnResponse(result);
                }
        /*
         * @brief This function returns the start time of the maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceStartTime","params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"maintenanceStartTime":12345678,"success":true}}
         * @return: Core::<StatusCode>
         */
        uint32_t MaintenanceManager::getMaintenanceStartTime (const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            string starttime="";

            starttime = Utils::cRunScript("/lib/rdk/getMaintenanceStartTime.sh &");
            if (!starttime.empty()){
                  response["maintenanceStartTime"]=stoi(starttime.c_str());
                  result=true;
            }

            returnResponse(result);
        }

        /***
        * @brief	: Used to read file contents into a vector
        * @param1[in]	: Complete file name with path
        * @param2[in]	: Destination vector buffer to be filled with file contents
        * @return	: <bool>; TRUE if operation success; else FALSE.
        */
        bool getFileContent(std::string fileName, std::vector<std::string> & vecOfStrs)
        {
            bool retStatus = false;
            std::ifstream inFile(fileName.c_str(), ios::in);

            if (!inFile.is_open())
                return retStatus;

            std::string line;
            retStatus = true;
            while (std::getline(inFile, line)) {
                if (line.size() > 0) {
                    vecOfStrs.push_back(line);
                }
            }
            inFile.close();
            return retStatus;
        }

        /* Utility API for parsing the  DCM/Device properties file */
        bool parseConfigFile(const char* filename, string findkey, string &value)
        {
            vector<std::string> lines;
            bool found=false;
            getFileContent(filename,lines);
            for (vector<std::string>::const_iterator i = lines.begin();
                 i != lines.end(); ++i){
                string line = *i;
                size_t eq = line.find_first_of("=");
                if (std::string::npos != eq) {
                    std::string key = line.substr(0, eq);
                    if (key == findkey) {
                        value = line.substr(eq + 1);
                        found=true;
                        break;
                    }
                }
            }

            if(found){
                return true;
            }
            else{
                return false;
            }
        }

        /*
         * @brief This function returns Mode of the maintenance.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceMode","params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"maintenanceMode":"FOREGROUND","optOut":"IGNORE_UPDATE","success":true}}
         * @return: Core::<StatusCode>
         */
        uint32_t MaintenanceManager::getMaintenanceMode(const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            string softwareOptOutmode = "NONE";
            if ( BACKGROUND_MODE != g_currentMode && FOREGROUND_MODE != g_currentMode )  {
                LOGERR("Didnt get a valid Mode. Failed\n");
                returnResponse(false);
            }
            else {
                response["maintenanceMode"] = g_currentMode;

                if ( Utils::fileExists(MAINTENANCE_MGR_RECORD_FILE) ){
                    if ( parseConfigFile(MAINTENANCE_MGR_RECORD_FILE,"softwareoptout",softwareOptOutmode)){
                        /* check if the value is valid */
                        if(!checkValidOptOutModes(softwareOptOutmode)){
                            LOGERR("OptOut Value Corrupted. Failed\n");
                            returnResponse(false);
                        }
                        else {
                            LOGINFO("OptOut Value = %s",softwareOptOutmode.c_str());
                        }
                    }
                    else {
                        LOGERR("OptOut Value Not Found. Failed\n");
                        returnResponse(false);
                    }
                }
                else {
                    LOGERR("OptOut Config File Not Found. Failed\n");
                    returnResponse(false);
                }
                response["optOut"] = softwareOptOutmode.c_str();
                result = true;
            }
            returnResponse(result);
        }


        /*
         * @brief This function returns the current status of the current
         * or previous maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.setMaintenanceMode",
         *                  "params":{"maintenanceMode":FOREGROUND}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::setMaintenanceMode(const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            string new_mode = "";
            string old_mode = g_currentMode;
            string bg_flag = "false";
            string new_optout_state = "";
	    bool rdkvfwrfc=false;
	    // 1 = Foreground and 0 = background
	    int mode = 1;

            rdkvfwrfc = readRFC(TR181_RDKVFWUPGRADER);
            /* Label should have maintenance mode and softwareOptout field */
            if ( parameters.HasLabel("maintenanceMode") && parameters.HasLabel("optOut") ){

                new_mode = parameters["maintenanceMode"].String();

                if ( BACKGROUND_MODE != new_mode && FOREGROUND_MODE != new_mode )  {
                    LOGERR("value of new mode is incorrect, therefore \
                            current mode '%s' not changed.\n", old_mode.c_str());
                        returnResponse(false);
                }

                std::lock_guard<std::mutex> guard(m_callMutex);

                /* check if maintenance is on progress or not */
                /* if in progress restrict the same */
                if ( MAINTENANCE_STARTED != m_notify_status ){

                    LOGINFO("SetMaintenanceMode new_mode = %s\n",new_mode.c_str());

                    /* remove any older one */
                    m_setting.remove("background_flag");
                    if ( BACKGROUND_MODE == new_mode ) {
                        bg_flag = "true";
                    }
                    else {
                        /* foreground */
                        bg_flag = "false";
                    }
                    g_currentMode = new_mode;
                    m_setting.setValue("background_flag", bg_flag);
		}else {
                     /*If firmware rfc is true and IARM bus component present allow to change maintenance mode*/
	            if (rdkvfwrfc == true) {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
                    LOGINFO("SetMaintenanceMode new_mode = %s\n",new_mode.c_str());
		    /* Sending IARM Event to application for mode change */
		    (new_mode != BACKGROUND_MODE) ? mode = 1 : mode = 0;
                    LOGINFO("setMaintenanceMode rfc is true and mode:%d\n", mode);
                    IARM_Result_t ret_code = IARM_RESULT_SUCCESS;
	            ret_code = IARM_Bus_BroadcastEvent("RdkvFWupgrader", (IARM_EventId_t) 0, (void *)&mode, sizeof(mode));
	            if (ret_code == IARM_RESULT_SUCCESS) {
                        LOGINFO("IARM_Bus_BroadcastEvent is success and value=%d\n", mode);
                        g_currentMode = new_mode;
                        /* remove any older one */
                        m_setting.remove("background_flag");
                        if ( BACKGROUND_MODE == new_mode ) {
                            bg_flag = "true";
                        }
                        else {
                            /* foreground */
                            bg_flag = "false";
                        }
		        m_setting.setValue("background_flag", bg_flag);
	            }else{
                        LOGINFO("IARM_Bus_BroadcastEvent is fail Mode change not allowed and value=%d\n", mode);
	            }
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
		    }else {
                        LOGERR("Maintenance is in Progress, Mode change not allowed");
		    }
                    result =true;
		}

                /* OptOut changes here */
                new_optout_state = parameters["optOut"].String();

                LOGINFO("SetMaintenanceMode optOut = %s\n",new_optout_state.c_str());

                /* check if we have a valid state from user */
                if(checkValidOptOutModes(new_optout_state)){
                    /* we got a valid state; Now store it in persistant location */
                    m_setting.setValue("softwareoptout",new_optout_state);
                }
                else{
                    LOGINFO("Invalid optOut = %s\n",new_optout_state.c_str());
                    returnResponse(false);
                }

                /* Set the result as true */
                result = true;

            }
            else {
                /* havent got the correct label */
                LOGERR("SetMaintenanceMode Missing Key Values\n");
            }

            returnResponse(result);
        }

        /*
         * @brief This function starts the maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.startMaintenance",
         *                  "params":{}}''
         * @param2[out]:{"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::startMaintenance(const JsonObject& parameters,
                JsonObject& response)
                {
                    bool result = false;
                    /* check what mode we currently have */
                    string current_mode="";

                    LOGINFO("Triggering scheduled maintenance ");
                    /* only one maintenance at a time */
                    /* Lock so that m_notify_status will not be updated  further */
                    m_statusMutex.lock();
                    if ( MAINTENANCE_STARTED != m_notify_status && g_unsolicited_complete ){

                        /*reset the status to 0*/
                        g_task_status=0;
                        g_maintenance_type=SOLICITED_MAINTENANCE;

                        m_abort_flag=false;

                        /* we dont touch the dcm so
                         * we say DCM is success and complete */
                        SET_STATUS(g_task_status,DCM_SUCCESS);
                        SET_STATUS(g_task_status,DCM_COMPLETE);

                        /* isRebootPending will be set to true
                         * irrespective of XConf configuration */
                        g_is_reboot_pending="true";

                        /* we set this to false */
                        g_is_critical_maintenance="false";

                        /* if there is any active thread, join it before executing the tasks from startMaintenance
                        * especially when device is in offline mode*/
                        if(m_thread.joinable()){
                            m_thread.join();
                            LOGINFO("Thread joined successfully\n");
                        }

                        m_thread = std::thread(&MaintenanceManager::task_execution_thread, _instance);

                        result=true;
                    }
                    else {
                        LOGINFO("Already a maintenance is in Progress. Please wait for it to complete !!");
                    }
                    m_statusMutex.unlock();
                    returnResponse(result);
                }

        /*
         * @brief This function stops the maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.stopMaintenance",
         *                  "params":{}}''
         * @param2[out]:{"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::stopMaintenance(const JsonObject& parameters,
                JsonObject& response){

                bool result=false;
                if( readRFC(TR181_STOP_MAINTENANCE) ) {
                    result=stopMaintenanceTasks();
                }
                else {
                    LOGINFO("Failed to initiate stopMaintenance, RFC is set as False\n");
                }
                returnResponse(result);
        }

        bool MaintenanceManager::stopMaintenanceTasks(){
	        string codeDLtask;
            int k_ret=EINVAL;
            int i=0;
            bool task_status[4]={false};
            bool result=false;

            LOGINFO("Stopping maintenance activities");
            /* run only when the maintenance status is MAINTENANCE_STARTED */
            m_statusMutex.lock();
            if ( MAINTENANCE_STARTED == m_notify_status  ){

                // Set the condition flag m_abort_flag to true
                m_abort_flag = true;

                auto task_status_DCM=m_task_map.find(task_names_foreground[0].c_str());
                auto task_status_RFC=m_task_map.find(task_names_foreground[1].c_str());
                auto task_status_FWDLD=m_task_map.find(task_names_foreground[2].c_str());
                auto task_status_LOGUPLD=m_task_map.find(task_names_foreground[3].c_str());

                task_status[0] = task_status_DCM->second;
                task_status[1] = task_status_RFC->second;
                task_status[2] = task_status_FWDLD->second;
                task_status[3] = task_status_LOGUPLD->second;

                for (i=0;i<4;i++) {
                    LOGINFO("task status [%d]  = %s ScriptName %s",i,(task_status[i])? "true":"false",script_names[i].c_str());
                }
                for (i=0;i<4;i++){
                    if(task_status[i]){

                        k_ret = abortTask( script_names[i].c_str() );        // default signal is SIGABRT

                        if( k_ret == 0 ) {                                      // if task(s) was(were) killed successfully ...                    
                            m_task_map[task_names_foreground[i].c_str()]=false; // set it to false 
                        }
                        /* No need to loop again */
                        break;
                    }
                    else{
                        LOGINFO("Task[%d] is false \n",i);
                    }
                }

                result=true;
            }
            else {
                LOGERR("Failed to stopMaintenance without starting maintenance \n");
            }
            task_thread.notify_one();

            if(m_thread.joinable()){
                m_thread.join();
                LOGINFO("Thread joined successfully\n");
            }

            if (UNSOLICITED_MAINTENANCE == g_maintenance_type && !g_unsolicited_complete){
                g_unsolicited_complete = true;
	    }

            LOGINFO("Maintenance has been stopped. Hence setting maintenance status to MAINTENANCE_ERROR\n");
            MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_ERROR);
            m_statusMutex.unlock();

            return result;
        }

        bool MaintenanceManager::readRFC(const char *rfc){
            bool ret=false;
            RFC_ParamData_t param;
	    if (rfc == NULL) {
                return ret;
	    }
            WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("MaintenanceManager"),rfc, &param);
            if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE){
	        LOGINFO("rfc read success");
                if( param.type == WDMP_BOOLEAN ){
	            LOGINFO("rfc type is boolean");
                    if(strncasecmp(param.value,"true",4) == 0 ){
	                LOGINFO("rfc value=%s", param.value);
                        ret=true;
                    }
                }
            }
            LOGINFO(" %s = %s , call value %d ", rfc, (ret == true)?"true":"false", wdmpStatus);
            return ret;
        }


        int MaintenanceManager::abortTask(const char* taskname, int sig_to_send){
            int k_ret=EINVAL;
            pid_t pid_num;

            pid_num=getTaskPID( taskname );
            LOGINFO("PID of %s is %d \n", taskname , (int)pid_num);
            if( pid_num != -1){
                /* send the signal to task to terminate */
                k_ret = kill( pid_num, sig_to_send );
                LOGINFO(" %s sent signal %d\n", taskname, sig_to_send );
                if (k_ret == 0){
                   LOGINFO(" %s Terminated\n", taskname );
                }
                else{
                    LOGINFO("Failed to terminate with error %s - %d \n", taskname, k_ret);
                }
            }
            else {
                LOGINFO("Didnt find PID for %s\n", taskname);
            }
            return k_ret;
        }

        /* Helper function to find the Script/Task PID*/
        pid_t MaintenanceManager::getTaskPID(const char* taskname){

            DIR* dir=opendir(PROC_DIR);
            struct dirent* ent;
            char* endptr;
            char buf[512];

            while((ent = readdir(dir)) != NULL) {
                long lpid = strtol(ent->d_name, &endptr, 10);
                if (*endptr != '\0') {
                    continue;
                }
                /* Get the PID */
                snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);

                /* Open the cmdline and read */
                FILE* fp = fopen(buf, "r");
                if (fp) {
                    char *arg = 0;
                    size_t size = 0;
                    while(getdelim(&arg, &size, 0,fp) != -1){
                        printf("%s\n",arg);
                        char* first = strstr(arg, taskname);
                        if (first != NULL){
                            free(arg);
                            fclose(fp);
                            closedir(dir);
                            return (pid_t)lpid;
                        }
                    }
                    free(arg);
                    fclose(fp);
                }
            }
            closedir(dir);
            return (pid_t)-1;
        }

        void MaintenanceManager::onMaintenanceStatusChange(Maint_notify_status_t status) {
            JsonObject params;
            /* we store the updated value as well */
            m_notify_status=status;
            params["maintenanceStatus"]=notifyStatusToString(status);
            sendNotify(EVT_ONMAINTENANCSTATUSCHANGE, params);
        }

   } /* namespace Plugin */
} /* namespace WPEFramework */
