/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#include "../../Module.h"
#include "Dolby.h"
#include "audio_if.h"
#include <string.h>
#include "hardware/audio.h"

using namespace WPEFramework::Exchange;

/**
 * @brief Resource wrapper for audio_hw_device.
 * 
 */
class AudioDevice
{
public:
    AudioDevice()
        : _audioDev(nullptr)
        , _initialized(false)
    {
        int error = 0;
        error = audio_hw_load_interface(&_audioDev);
        if (error == 0)
        {
            error = _audioDev->init_check(_audioDev);
            _initialized = (error == 0);
        }
    }

    ~AudioDevice()
    {
        if (IsInitialized())
        {
            audio_hw_unload_interface(_audioDev);
        }
    }

    AudioDevice(const AudioDevice &) = delete;
    AudioDevice &operator=(const AudioDevice &) = delete;

    bool IsInitialized() const { return _initialized; }

    int HdmiFormat(int hdmiMode)
    {
        std::string command("hdmi_format=");
        command += std::to_string(hdmiMode);
        return _audioDev->set_parameters(_audioDev, command.c_str());
    }

    int HdmiFormat() const
    {
        char *value = _audioDev->get_parameters(_audioDev, "hdmi_format");
        int code = value[strlen("hdmi_format=")] - '0';
        delete value;
        return code;
    }

private:
    audio_hw_device *_audioDev;
    bool _initialized;
};

uint32_t set_audio_output_type(const Dolby::IOutput::Type type)
{
    uint32_t result = WPEFramework::Core::ERROR_GENERAL;
    AudioDevice audioDev;
    if (audioDev.IsInitialized() && type != Dolby::IOutput::Type::DIGITAL_PLUS)
    {
        if (audioDev.HdmiFormat(static_cast<int>(type)) == 0)
            result = WPEFramework::Core::ERROR_NONE;
    }
    return result;
}

Dolby::IOutput::Type ToEnum(int code, uint32_t &error)
{
    Dolby::IOutput::Type result;
    switch (code)
    {
    case Dolby::IOutput::Type::AUTO:
    {
        result = Dolby::IOutput::Type::AUTO;
        error = WPEFramework::Core::ERROR_NONE;
        break;
    }
    case Dolby::IOutput::Type::DIGITAL_AC3:
    {
        result = Dolby::IOutput::Type::DIGITAL_AC3;
        error = WPEFramework::Core::ERROR_NONE;
        break;
    }
    case Dolby::IOutput::Type::DIGITAL_PCM:
    {
        result = Dolby::IOutput::Type::DIGITAL_PCM;
        error = WPEFramework::Core::ERROR_NONE;
        break;
    }
    case Dolby::IOutput::Type::DIGITAL_PASSTHROUGH:
    {
        result = Dolby::IOutput::Type::DIGITAL_PASSTHROUGH;
        error = WPEFramework::Core::ERROR_NONE;
        break;
    }
    default:
    {
        result = Dolby::IOutput::Type::AUTO;
        error = WPEFramework::Core::ERROR_GENERAL;
        TRACE_GLOBAL(WPEFramework::Trace::Information, (_T("Could not map the provided dolby output: %d to Dolby::IOutput::Type enumeration."), code));
        break;
    }
    }
    return result;
}

uint32_t get_audio_output_type(WPEFramework::Exchange::Dolby::IOutput::Type *type)
{
    uint32_t error = WPEFramework::Core::ERROR_GENERAL;

    AudioDevice audioDev;
    if (audioDev.IsInitialized())
    {
        int audioType = audioDev.HdmiFormat();
        *type = ToEnum(audioType, error);
    }
    return error;
}
