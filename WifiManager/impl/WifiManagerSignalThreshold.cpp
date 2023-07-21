/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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

#include "WifiManagerSignalThreshold.h"
#include "../WifiManager.h" // Need access to WifiManager::getInstance so can't use 'WifiManagerInterface.h'
#include "UtilsJsonRpc.h"
#include <wpa_ctrl.h>

#include <chrono>

using namespace WPEFramework::Plugin;

namespace {
    const float signalStrengthThresholdExcellent = -50.0f;
    const float signalStrengthThresholdGood = -60.0f;
    const float signalStrengthThresholdFair = -67.0f;

    #define BUFFER_SIZE 512
    #define Command "SIGNAL_POLL"

    std::string retrieveValues(const char *command, char *output_buffer, size_t output_buffer_size)
    {
        struct wpa_ctrl *wpa_ctrl= NULL;
        std::string key, value;
        std::string rssi = "";
        int ret = 0;

        if((NULL == command) || (NULL == output_buffer))
        {
            LOGERR("command/output_buffer is NULL\n");
        }
        else
        {
            wpa_ctrl = wpa_ctrl_open("/var/run/wpa_supplicant/wlan0");

            if (NULL == wpa_ctrl)
            {
                LOGERR("wpa_ctrl_open Failed  to open /var/run/wpa_supplicant/wlan0\n");
            }
            else if(-2 ==  (ret = wpa_ctrl_request(wpa_ctrl, command, strlen(command), output_buffer, &output_buffer_size, NULL)))
            {
                LOGERR("SIGNAL_POLL timed out\n");
            }
            else if (ret < 0)
            {
                LOGERR("SIGNAL_POLL failed\n");
            }
            else
            {
                std::istringstream mystream(output_buffer);

                if(std::getline(std::getline(mystream, key, '=') >> std::ws, value))
                {
                    if ("RSSI" == key)
                    {
                        rssi = value;
                    }
                }
            }

            if (NULL != wpa_ctrl)
            {
                wpa_ctrl_close(wpa_ctrl);
                wpa_ctrl = NULL;
            }
        }
        return rssi;
    }

    void getSignalData(float &signalStrengthOut, std::string &strengthOut) {
        JsonObject response;
        char buff[BUFFER_SIZE] = {'\0'};

        string signalStrength = retrieveValues(Command, buff, sizeof (buff));

        signalStrengthOut = 0.0f;
        if (!signalStrength.empty())
            signalStrengthOut = std::stof(signalStrength.c_str());
        else {
            LOGERR("signalStrength is empty\n");
            strengthOut = "Disconnected";
            return;
        }

        if (signalStrengthOut >= signalStrengthThresholdExcellent && signalStrengthOut < 0)
        {
            strengthOut = "Excellent";
        }
        else if (signalStrengthOut >= signalStrengthThresholdGood && signalStrengthOut < signalStrengthThresholdExcellent)
        {
            strengthOut = "Good";
        }
        else if (signalStrengthOut >= signalStrengthThresholdFair && signalStrengthOut < signalStrengthThresholdGood)
        {
            strengthOut = "Fair";
        }
        else
        {
            strengthOut = "Weak";
        };
    }
}

WifiManagerSignalThreshold::WifiManagerSignalThreshold():
    changeEnabled(false),
    running(false)
{
}


WifiManagerSignalThreshold::~WifiManagerSignalThreshold()
{
}

void WifiManagerSignalThreshold::stopSignalThresholdThread()
{
    stopThread();
}

uint32_t WifiManagerSignalThreshold::setSignalThresholdChangeEnabled(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();
    returnIfBooleanParamNotFound(parameters, "enabled");
    returnIfNumberParamNotFound(parameters, "interval");

    bool enabled = parameters["enabled"].Boolean();
    int interval = parameters["interval"].Number();

    setSignalThresholdChangeEnabled(enabled, interval);

    returnResponse(true);
}

uint32_t WifiManagerSignalThreshold::isSignalThresholdChangeEnabled(const JsonObject &parameters, JsonObject &response) const
{
    LOGINFOMETHOD();

    if(isSignalThresholdChangeEnabled())
    {
        response["result"] = 0;
    }
    else
    {
        response["result"] = 1;
    }

    returnResponse(true);
}

void WifiManagerSignalThreshold::setSignalThresholdChangeEnabled(bool enabled, int interval)
{
    LOGINFO("setSignalThresholdChangeEnabled: enabled %s, interval %d", enabled ? "true":"false", interval);

    stopThread();

    changeEnabled = enabled;
    JsonObject parameters, response;
    WifiState state;

    uint32_t result = WifiManager::getInstance().getCurrentState(parameters, response);
    if (result != 0)
    {
        LOGINFO("wifiManager.getCurrentState result = %d", result);
        return;
    }
    else if (response.HasLabel("state"))
    {
        int64_t number = std::stoi(response["state"].String());
        state = (WifiState) number;
        LOGINFO("wifi state = %d", static_cast<int>(state));
    }
    else
    {
        LOGINFO("no state attribute");
        return;
    }

    if(changeEnabled)
    {
        if (state == WifiState::CONNECTED)
            running = true;
        startThread(interval);
    }
}

bool WifiManagerSignalThreshold::isSignalThresholdChangeEnabled() const
{
    return changeEnabled;
}

void WifiManagerSignalThreshold::loop(int interval)
{
    std::unique_lock<std::mutex> lk(cv_mutex);
    std::string lastStrength = "";

    while(changeEnabled)
    {
        float signalStrength;
        std::string strength;
        if (running)
        {
            getSignalData(signalStrength, strength);

            if (strength != lastStrength)
            {
                LOGINFO("Triggering onWifiSignalThresholdChanged notification");
                WifiManager::getInstance().onWifiSignalThresholdChanged(signalStrength, strength);
                lastStrength = strength;
            }
        }
        cv.wait_for(lk, std::chrono::milliseconds(interval), [this](){ return changeEnabled == false; });
    }
}

void WifiManagerSignalThreshold::stopThread()
{
    changeEnabled = false;
    cv.notify_one();
    if(thread.joinable()) {
        thread.join();
    }
    running = false ;
}


void WifiManagerSignalThreshold::setSignalThresholdChangeEnabled(bool enable)
{
    LOGINFO("setSignalThresholdChangeEnabled: enable %s", enable ? "true":"false");
    running = enable;
}

void WifiManagerSignalThreshold::startThread(int interval)
{
    thread = std::thread([interval, this](){
        loop(interval);
    });
}
