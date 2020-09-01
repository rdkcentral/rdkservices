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

#include "utils.h"

#include <chrono>

using namespace WPEFramework::Plugin;

namespace {
    const float signalStrengthThresholdExcellent = -50.0f;
    const float signalStrengthThresholdGood = -60.0f;
    const float signalStrengthThresholdFair = -67.0f;

    void getSignalData(WifiManagerInterface &wifiManager, float &signalStrengthOut, std::string &strengthOut) {
        JsonObject response;
        wifiManager.getConnectedSSID(JsonObject(), response);

        signalStrengthOut = 0.0f;
        if (response.HasLabel("signalStrength")) {
            signalStrengthOut = std::stof(response["signalStrength"].String());
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

WifiManagerSignalThreshold::WifiManagerSignalThreshold(WifiManagerInterface &wifiManager):
    changeEnabled(false),
    wifiManager(wifiManager)
{
}

WifiManagerSignalThreshold::~WifiManagerSignalThreshold()
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
    stopThread();

    changeEnabled = enabled;
    if(changeEnabled) {
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
    while(changeEnabled) {
        LOGINFO("WifiManagerSignalThreashold::loop");

        float signalStrength;
        std::string strength;
        getSignalData(wifiManager, signalStrength, strength);

        wifiManager.onWifiSignalThresholdChanged(signalStrength, strength);

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
}

void WifiManagerSignalThreshold::startThread(int interval)
{
    thread = std::thread([interval, this](){
        loop(interval);
    });
}
