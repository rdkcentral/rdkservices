/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2022 RDK Management
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
#include "LegacyPlugin_WiFiManagerAPIs.h"
#include "NetworkManagerLogger.h"


using namespace std;
using namespace WPEFramework::Plugin;
#define API_VERSION_NUMBER_MAJOR 2
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0
#define NETWORK_MANAGER_CALLSIGN    "org.rdk.NetworkManager.1"

#define LOGINFOMETHOD() { string json; parameters.ToString(json); NMLOG_TRACE("Legacy params=%s", json.c_str() ); }
#define LOGTRACEMETHODFIN() { string json; response.ToString(json); NMLOG_TRACE("Legacy response=%s", json.c_str() ); }

namespace WPEFramework
{
    class Job : public Core::IDispatch {
    public:
        Job(function<void()> work)
        : _work(work)
        {
        }
        void Dispatch() override
        {
            _work();
        }

    private:
        function<void()> _work;
    };

    static Plugin::Metadata<Plugin::WiFiManager> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
    );

    WiFiManager* _gWiFiInstance = nullptr;
    namespace Plugin
    {
        SERVICE_REGISTRATION(WiFiManager, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        WiFiManager::WiFiManager()
        : PluginHost::JSONRPC()
        , m_service(nullptr)
       {
           _gWiFiInstance = this;
           RegisterLegacyMethods();
       }

        WiFiManager::~WiFiManager()
        {
            _gWiFiInstance = nullptr;
        }

        void WiFiManager::activatePrimaryPlugin()
        {
            uint32_t result = Core::ERROR_ASYNC_FAILED;
            string callsign(NETWORK_MANAGER_CALLSIGN);
            Core::Event event(false, true);
            Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]() {
                auto interface = m_service->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    NMLOG_WARNING("no IShell for %s", callsign.c_str());
                } else {
                    NMLOG_INFO("Activating %s", callsign.c_str());
                    result = interface->Activate(PluginHost::IShell::reason::REQUESTED);
                    interface->Release();
                }
                event.SetEvent();
            })));
            event.Lock();

            return;
        }

        const string WiFiManager::Initialize(PluginHost::IShell*  service )
        {
            m_service = service;
            m_service->AddRef();

            string callsign(NETWORK_MANAGER_CALLSIGN);

            auto interface = m_service->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
            if (interface != nullptr)
            {
                PluginHost::IShell::state state = interface->State(); 
                NMLOG_INFO("Current status of the %s plugin is %d", callsign.c_str(), state);

                if((PluginHost::IShell::state::ACTIVATED  == state) || (PluginHost::IShell::state::ACTIVATION == state))
                {
                    NMLOG_INFO("Dependency Plugin '%s' Ready", callsign.c_str());
                }
                else
                {
                    activatePrimaryPlugin();
                }
                interface->Release();
            }
        
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            string query="token=";
            m_networkmanager = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >("org.rdk.NetworkManager", "");

            /* Wait for Proxy stuff to be established */
            sleep(3);
    
            if (Core::ERROR_NONE != subscribeToEvents())
                return string("Failed to Subscribe");
            else
                return string();
        }

        void WiFiManager::Deinitialize(PluginHost::IShell* /* service */)
        {
            UnregisterLegacyMethods();
            m_service->Release();
            m_service = nullptr;
            _gWiFiInstance = nullptr;
        }

        string WiFiManager::Information() const
        {
             return(string());
        }

        /**
         * Hook up all our JSON RPC methods
         *
         * Each method definition comprises of:
         *  * Input parameters
         *  * Output parameters
         *  * Method name
         *  * Function that implements that method
         */
        void WiFiManager::RegisterLegacyMethods()
        {
            CreateHandler({2});
            Register("cancelWPSPairing",                  &WiFiManager::cancelWPSPairing, this);
            Register("clearSSID",                         &WiFiManager::clearSSID, this);
            Register("connect",                           &WiFiManager::connect, this);
            Register("disconnect",                        &WiFiManager::disconnect, this);
            Register("getConnectedSSID",                  &WiFiManager::getConnectedSSID, this);
            Register("getCurrentState",                   &WiFiManager::getCurrentState, this);
            Register("getPairedSSID",                     &WiFiManager::getPairedSSID, this);
            Register("getPairedSSIDInfo",                 &WiFiManager::getPairedSSIDInfo, this);
            Register("getSupportedSecurityModes",         &WiFiManager::getSupportedSecurityModes, this);
            Register("initiateWPSPairing",                &WiFiManager::initiateWPSPairing, this);
            GetHandler(2)->Register<JsonObject, JsonObject>("initiateWPSPairing", &WiFiManager::initiateWPSPairing, this);
            Register("isPaired",                          &WiFiManager::isPaired, this);
            Register("saveSSID",                          &WiFiManager::saveSSID, this);
            Register("startScan",                         &WiFiManager::startScan, this);
            Register("stopScan",                          &WiFiManager::stopScan, this);
        }

        /**
         * Unregister all our JSON-RPC methods
         */
        void WiFiManager::UnregisterLegacyMethods()
        {
            Unregister("cancelWPSPairing");
            Unregister("clearSSID");
            Unregister("connect");
            Unregister("disconnect");
            Unregister("getConnectedSSID");
            Unregister("getCurrentState");
            Unregister("getPairedSSID");
            Unregister("getPairedSSIDInfo");
            Unregister("getSupportedSecurityModes");
            Unregister("initiateWPSPairing");
            Unregister("isPaired");
            Unregister("saveSSID");
            Unregister("startScan");
            Unregister("stopScan");
        }

        uint32_t WiFiManager::cancelWPSPairing (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;

            LOGINFOMETHOD();

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("StopWPS"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["result"] = string();
            }

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::clearSSID (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpParameters;

            LOGINFOMETHOD();

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("RemoveKnownSSID"), tmpParameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["result"] = 0;
            }

            LOGTRACEMETHODFIN();
            return rc;
        }
 
        uint32_t WiFiManager::connect(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("WiFiConnect"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::getConnectedSSID (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;

            LOGINFOMETHOD();

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetConnectedSSID"), parameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["ssid"] = tmpResponse["ssid"];
                response["bssid"] = tmpResponse["bssid"];
                response["rate"] = tmpResponse["rate"];
                response["noise"] = tmpResponse["noise"];
                response["security"] = tmpResponse["securityMode"];
                response["signalStrength"] = tmpResponse["signalStrength"];
                response["frequency"] = tmpResponse["frequency"];
                response["success"] = tmpResponse["success"];
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::getCurrentState(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetWifiState"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::getPairedSSID(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetKnownSSIDs"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::getPairedSSIDInfo(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetConnectedSSID"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::getSupportedSecurityModes(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetSupportedSecurityModes"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::isPaired (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            LOGINFOMETHOD();
            JsonObject tmpResponse;

            rc = getPairedSSID(parameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            {
                JsonArray array = tmpResponse["ssids"].Array();
                if (0 == array.Length())
                {
                    response["result"] = 1;
                }
                else
                {
                    response["result"] = 0;
                }
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::saveSSID (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;

            LOGINFOMETHOD();

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("AddToKnownSSIDs"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["result"] = 0;
            }

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::disconnect (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;

            LOGINFOMETHOD();

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("WiFiDisconnect"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["result"] = 0;
            }

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::initiateWPSPairing (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpParameters;
            LOGINFOMETHOD();
            if (parameters.HasLabel("method"))
            {
                string method = parameters["method"].String();
                if (method == "PBC")
                {
                    tmpParameters["method"] = 0;
                }
                else if (method == "PIN")
                {
                    tmpParameters["method"] = 1;
                    tmpParameters["wps_pin"] = parameters.HasLabel("wps_pin");
                }
                else if (method == "SERIALIZED_PIN")
                {
                    tmpParameters["method"] = 2;
                }
            }
            else
                tmpParameters["method"] = 0;

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("StartWPS"), tmpParameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["result"] = string();
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::startScan(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpParameters;

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("StartWiFiScan"), tmpParameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t WiFiManager::stopScan(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("StopWiFiScan"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        /** Private */
        uint32_t WiFiManager::subscribeToEvents(void)
        {
            uint32_t errCode = Core::ERROR_GENERAL;
            if (m_networkmanager)
            {
                errCode = m_networkmanager->Subscribe<JsonObject>(1000, _T("onWiFiStateChange"), &WiFiManager::onWiFiStateChange);
                if (errCode != Core::ERROR_NONE)
                {
                    NMLOG_ERROR ("Subscribe to onInterfaceStateChange failed, errCode: %u", errCode);
                }
                errCode = m_networkmanager->Subscribe<JsonObject>(1000, _T("onAvailableSSIDs"), &WiFiManager::onAvailableSSIDs);
                if (errCode != Core::ERROR_NONE)
                {
                    NMLOG_ERROR("Subscribe to onIPAddressChange failed, errCode: %u", errCode);
                }
                errCode = m_networkmanager->Subscribe<JsonObject>(1000, _T("onWiFiSignalStrengthChange"), &WiFiManager::onWiFiSignalStrengthChange);
                if (errCode != Core::ERROR_NONE)
                {
                    NMLOG_ERROR("Subscribe to onActiveInterfaceChange failed, errCode: %u", errCode);
                }
            }
            return errCode;
        }

        /** Event Handling and Publishing */
        void WiFiManager::onWiFiStateChange(const JsonObject& parameters)
        {
            LOGINFOMETHOD();
            JsonObject legacyResult;
            legacyResult["state"] = parameters["state"];
            legacyResult["isLNF"] = false;

            if(_gWiFiInstance)
                _gWiFiInstance->Notify("onWIFIStateChanged", legacyResult);

            return;
        }

        void WiFiManager::onAvailableSSIDs(const JsonObject& parameters)
        {
            LOGINFOMETHOD();

            if(_gWiFiInstance)
                _gWiFiInstance->Notify("onAvailableSSIDs", parameters);

            return;
        }

        void WiFiManager::onWiFiSignalStrengthChange(const JsonObject& parameters)
        {
            LOGINFOMETHOD();
            JsonObject legacyParams;
            legacyParams["signalStrength"] = parameters["signalQuality"];
            legacyParams["strength"] = parameters["signalLevel"];

            if (_gWiFiInstance)
                _gWiFiInstance->Notify("onWifiSignalThresholdChanged", legacyParams);

            return;
        }
    }
}
