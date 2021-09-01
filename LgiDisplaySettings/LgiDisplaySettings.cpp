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

#include "LgiDisplaySettings.h"
#include <algorithm>
#include "host.hpp"
#include "exception.hpp"
#include "videoOutputPort.hpp"
#include "videoOutputPortType.hpp"
#include "videoOutputPortConfig.hpp"
#include "videoResolution.hpp"
#include "audioOutputPort.hpp"
#include "audioOutputPortType.hpp"
#include "audioOutputPortConfig.hpp"
#include "dsError.h"

using namespace std;

namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(LgiDisplaySettings, 1, 0);

	extern void setResponseArray(JsonObject& response, const char* key, const vector<string>& items);

        static bool parseQBool(const std::string& str)
        {
            // https://doc.qt.io/qt-5/qvariant.html#toBool:
            // 'Returns true if (...) lower-case content is not one of the following: empty, "0" or "false"; otherwise returns false.'
            string lowercase_string{};
            std::transform(str.begin(),
                           str.end(),
                           std::back_inserter(lowercase_string),
                           ::tolower);
            return !(lowercase_string.empty() || lowercase_string == "false" || lowercase_string == "0");
        }

        LgiDisplaySettings::LgiDisplaySettings() : DisplaySettings()
        {
            registerMethod("setOutputFrameRatePreference", &LgiDisplaySettings::setOutputFrameRatePreference, this);
            registerMethod("setAudioProcessingHint", &LgiDisplaySettings::setAudioProcessingHint, this);
            registerMethod("getAudioOutputEncoding", &LgiDisplaySettings::getAudioOutputEncoding, this);
            registerMethod("getFollowColorSpace", &LgiDisplaySettings::getFollowColorSpace, this);
            registerMethod("setFollowColorSpace", &LgiDisplaySettings::setFollowColorSpace, this);
            registerMethod("getPreferredOutputColorSpace", &LgiDisplaySettings::getPreferredOutputColorSpace, this);
            registerMethod("setPreferredOutputColorSpace", &LgiDisplaySettings::setPreferredOutputColorSpace, this);
            registerMethod("getHDRGfxColorSpace", &LgiDisplaySettings::getHDRGfxColorSpace, this);
            registerMethod("setHDRGfxColorSpace", &LgiDisplaySettings::setHDRGfxColorSpace, this);
        }

        uint32_t LgiDisplaySettings::setOutputFrameRatePreference(const JsonObject& parameters, JsonObject& response)
        {
            // servicemanager params: (const bool followContent);
            LOGINFOMETHOD();
            bool success = false;
            returnIfParamNotFound(parameters, "followContent");
            const string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : "HDMI0";
            const bool followContent = parseQBool(parameters["followContent"].String());

            try
            {
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                vPort.setOutputFrameRatePreference(followContent);
                success = true;
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            returnResponse(success);
        }

        uint32_t LgiDisplaySettings::setAudioProcessingHint(const JsonObject& parameters, JsonObject& response)
        {
            // servicemanager params: (QString audioPort, QString audioMode, QString audioDelayMs);
            LOGINFOMETHOD();
            bool success = false;
            returnIfParamNotFound(parameters, "audioMode");
            const string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";

            int64_t _delayMs = -1;
            getDefaultNumberParameter("audioDelayMs", _delayMs, -1);
            if (_delayMs >= 0 && _delayMs <= std::numeric_limits<uint32_t>::max())
            {
                const string audioMode = parameters["audioMode"].String();
                const uint32_t delayMs = _delayMs;
                try
                {
                    device::AudioOutputPort &aPort = device::AudioOutputPortConfig::getInstance().getPort(audioPort);
                    aPort.setAudioDelayHint(delayMs, audioMode);
                    success = true;
                }
                catch(const device::Exception& err)
                {
                    LOG_DEVICE_EXCEPTION0();
                }
            }
            else
            {
                LOGWARN("setAudioProcessingHint: audioDelayMs value %lld out of uint32_t bounds; not executed", _delayMs);
            }

            returnResponse(success);
        }

        uint32_t LgiDisplaySettings::getAudioOutputEncoding(const JsonObject& parameters, JsonObject& response)
        {
            // servicemanager params: (QString audioPort);
            LOGINFOMETHOD();
            bool success = false;

            const string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";

            try
            {
                device::AudioOutputPort &aPort = device::AudioOutputPortConfig::getInstance().getPort(audioPort);
                const device::AudioEncoding &aEnc = aPort.getEncoding();
                response["encoding"] = aEnc.getName();
                success = true;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            returnResponse(success);
        }

        uint32_t LgiDisplaySettings::getFollowColorSpace(const JsonObject& parameters, JsonObject& response)
        {
            // servicemanager params: (QString videoDisplay) const;
            LOGINFOMETHOD();
            bool success = false;
            const string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : "HDMI0";

            try
            {
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                response["followColorSpace"] = vPort.getFollowColorSpace();
                success = true;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            returnResponse(success);
        }

        uint32_t LgiDisplaySettings::setFollowColorSpace(const JsonObject& parameters, JsonObject& response)
        {
            // servicemanager params: (QString videoDisplay, bool followCOlorSpace);
            LOGINFOMETHOD();
            bool success = false;
            const string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : "HDMI0";
            returnIfParamNotFound(parameters, "followColorSpace");

            const bool followColorSpace = parseQBool(parameters["followColorSpace"].String());

            try
            {
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                vPort.setFollowColorSpace(followColorSpace);
                success = true;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            returnResponse(success);
        }

        uint32_t LgiDisplaySettings::getPreferredOutputColorSpace(const JsonObject& parameters, JsonObject& response)
        {
            // servicemanager params: (const QString videoDisplay);
            LOGINFOMETHOD();
            bool success = false;
            const string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : "HDMI0";

            try
            {
                std::string result;
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                if (vPort.getPreferredOutputColorSpace(result))
                {
                    // the result is comma-separated list of values, like 'BT2020_NCL,BT2020_CL,BT709'
                    string colorSpace;
                    stringstream ss {result};
                    vector<string> colorSpaces;
                    while (getline(ss, colorSpace, ',')) {
                        colorSpaces.push_back(colorSpace);
                    }
                    setResponseArray(response, "preferredOutputColorSpaces", colorSpaces);
                    success = true;
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            returnResponse(success);
        }

        uint32_t LgiDisplaySettings::setPreferredOutputColorSpace(const JsonObject& parameters, JsonObject& response)
        {
            // servicemanager params: (const QString videoDisplay, const QString colorSpaces);
            // here, we expect colorSpaces to be array
            LOGINFOMETHOD();
            bool success = false;
            const string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : "HDMI0";

            returnIfParamNotFound(parameters, "colorSpaces");
            auto colorSpacesArr = parameters["colorSpaces"].Array();
            auto colorSpaces = colorSpacesArr.Elements();

            try
            {
                stringstream strColorSpaces;
                bool first = true;
                while (colorSpaces.Next())
                {
                    if (!first) strColorSpaces << ",";
                    first = false;
                    strColorSpaces << colorSpaces.Current().String();
                }
                device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                success = vPort.setPreferredOutputColorSpace(strColorSpaces.str());
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            returnResponse(success);
        }

        uint32_t LgiDisplaySettings::getHDRGfxColorSpace(const JsonObject& parameters, JsonObject& response)
        {
            // servicemanager params: (QString videoPort, int &y, int &cr, int &cb);
            LOGINFOMETHOD();
            bool success = false;
            const string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : "HDMI0";

            try {
                int16_t y, cr, cb;
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);

                vPort.getType().getInstance(device::VideoOutputPortType::kHDMI).GetHDRGfxColorSpace(&y, &cr, &cb);

                // without some cast, JSON module will end up serializing weird incorrect stuff
                // there is probably some Thunder bug regarding handling int16_t in JSON.h
                response["y"] = int32_t(y);
                response["cr"] = int32_t(cr);
                response["cb"] = int32_t(cb);

                success = true;
            }
            catch(const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION0();
            }

            returnResponse(success);
        }

        uint32_t LgiDisplaySettings::setHDRGfxColorSpace(const JsonObject& parameters, JsonObject& response)
        {
            // servicemanager params: (QString videoPort, int y, int cr, int cb);
            LOGINFOMETHOD();
            bool success = false;
            const string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : "HDMI0";

            returnIfParamNotFound(parameters, "y");
            returnIfParamNotFound(parameters, "cr");
            returnIfParamNotFound(parameters, "cb");

            int64_t y, cr, cb;
            getDefaultNumberParameter("y", y, std::numeric_limits<int64_t>::max());
            getDefaultNumberParameter("cr", cr, std::numeric_limits<int64_t>::max());
            getDefaultNumberParameter("cb", cb, std::numeric_limits<int64_t>::max());

            if (y < std::numeric_limits<int16_t>::min() || y > std::numeric_limits<int16_t>::max()
                || cr < std::numeric_limits<int16_t>::min() || cr > std::numeric_limits<int16_t>::max()
                || cb < std::numeric_limits<int16_t>::min() || cb > std::numeric_limits<int16_t>::max())
            {
                LOGWARN("setHDRGfxColorSpace: some of values: y=%lld cr=%lld cb=%lld out of int16_t bounds; not executed", y,cr,cb);
            }
            else
            {
                int16_t _y = y, _cr = cr, _cb = cb;
                try
                {
                    device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                    vPort.getType().getInstance(device::VideoOutputPortType::kHDMI).SetHDRGfxColorSpace(&_y, &_cr, &_cb);
                    success = true;
                }
                catch(const device::Exception& err)
                {
                    LOG_DEVICE_EXCEPTION0();
                }
            }

            returnResponse(success);
        }

        uint32_t LgiDisplaySettings::getBassEnhancer(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success = true;
            string audioPort = parameters.HasLabel("audioPort") ? parameters["audioPort"].String() : "HDMI0";
            dsSurroundVirtualizer_t virtualizer;
            try
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                if (aPort.isConnected())
                {
                    virtualizer = aPort.getSurroundVirtualizer();
                    response["enable"] = virtualizer.mode ? true : false ;
                    response["boost"] = virtualizer.boost;

                }
                else
                {
                    LOGERR("aport is not connected!");
                    success = false;
                }
            }
            catch (const device::Exception& err)
            {
                LOG_DEVICE_EXCEPTION1(audioPort);
                success = false;
                response["enable"] = false;
            }
            returnResponse(success);
        }

    } // namespace Plugin
} // namespace WPEFramework
