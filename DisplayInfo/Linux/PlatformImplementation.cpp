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

#include "../Module.h"
#include "DRMConnector.h"
#include <core/Portability.h>
#include <interfaces/IDRM.h>
#include <interfaces/IDisplayInfo.h>
#include <interfaces/IConfiguration.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <fstream>
#include <functional>
#include <mutex>
#include <string>

namespace WPEFramework {
namespace Plugin {

    std::string getLine(const std::string& filepath)
    {
        std::string line;
        std::ifstream statusFile(filepath);

        if (statusFile.is_open()) {
            getline(statusFile, line);
            statusFile.close();
        } else {
            TRACE_GLOBAL(Trace::Error, (_T("Could not open file: %s"), filepath.c_str()));
        }
        return line;
    }

    class UdevObserverType : public Core::SocketDatagram {
        using CallbackDescriptor = std::pair<std::function<void(const std::string&)>, std::string>;
    public:
        UdevObserverType(const UdevObserverType&) = delete;
        const UdevObserverType& operator=(const UdevObserverType&) = delete;

        UdevObserverType()
            /** The group value of "2" stands for GROUP_UDEV, which filters out kernel messages,
             * occuring before the device is initialized.
             * https://insujang.github.io/2018-11-27/udev-device-manager-for-the-linux-kernel-in-userspace/
             */
            : Core::SocketDatagram(false, Core::NodeId(NETLINK_KOBJECT_UEVENT, 0, 2), Core::NodeId(), 512, 1024)
        {
            Core::SocketDatagram::Open(Core::infinite);
        }

        /**
         * @brief Registers a callback per unique devtype value. The specified callback will
         * be invoked if a UDEV device with an attribute DEVTYPE=<devtype> has changed.
         * DEVTYPE attribute can be retrieved via CLI if the user knows the path
         * to the device:
         *      udevadm info --path=<path_to_device>.
         * 
         * @param callback - Callback to be invoked on status change of a device.
         * @param devtype - DEVTYPE attribute of a udev device.
         * @return bool - False if a callback for a given devtype already exists.  
         */
        bool Register(std::function<void(const std::string&)>& callback, const std::string& devtype)
        {
            auto found = std::find_if(_callbacks.begin(), _callbacks.end(), [&](CallbackDescriptor& pair){
                return pair.second == devtype;
            });

            if (found == _callbacks.end()) {
                _callbacks.push_back({callback, devtype});
                return true;
            } else {
                return false;
            }
        }

        void Unregister(const std::string& devtype) 
        {
            _callbacks.erase(std::remove_if(_callbacks.begin(), _callbacks.end(), [&](CallbackDescriptor& pair) {
                return pair.second == devtype;
            }));
        }

        ~UdevObserverType() override
        {
            if (Core::SocketDatagram::IsOpen()) {
                Core::SocketDatagram::Close(Core::infinite);
            }
        }

        void StateChange() override {}

        uint16_t SendData(VARIABLE_IS_NOT_USED uint8_t* dataFrame, VARIABLE_IS_NOT_USED const uint16_t maxSendSize) override
        {
            return 0;
        }

        uint16_t ReceiveData(uint8_t* dataFrame, const uint16_t receivedSize) override
        {
            udev_monitor_netlink_header* header = reinterpret_cast<udev_monitor_netlink_header*>(dataFrame);

            if (header->filter_tag_bloom_hi == 0 && header->filter_tag_bloom_lo == 0) {

                int data_index = header->properties_off / sizeof(uint8_t);
                auto data_ptr = reinterpret_cast<char*>(&(dataFrame[data_index]));
                auto values = GetMessageValues(data_ptr, header->properties_len);

                for (auto& callback : _callbacks) {
                    if (std::find(values.begin(), values.end(), "DEVTYPE=" + callback.second) != values.end()) {
                        callback.first(callback.second);
                    }
                }
            }

            return receivedSize;
        }

    private:
        /**
         * See https://github.com/systemd/systemd/blob/master/src/libsystemd/sd-device/device-monitor.c
         * for details of the udev_monitor_netlink_header structure, which is needed
         * in order to properly parse messages coming in through the socket connection
         * (UdevObserver class).
         */
        static constexpr auto UDEV_MONITOR_MAGIC = 0xfeedcafe;
        struct udev_monitor_netlink_header {
            char prefix[8];
            unsigned int magic;
            unsigned int header_size;
            unsigned int properties_off;
            unsigned int properties_len;
            unsigned int filter_subsystem_hash;
            unsigned int filter_devtype_hash;
            unsigned int filter_tag_bloom_hi;
            unsigned int filter_tag_bloom_lo;
        };

        std::vector<std::string> GetMessageValues(char* values, uint32_t size)
        {
            std::vector<std::string> output;
            for (uint32_t i = 0, output_index = 0; i < size; ++output_index) {
                char* data = &values[i];
                output.push_back(std::string(data));
                i += (strlen(data) + 1);
            }
            return output;
        }
        std::vector<CallbackDescriptor> _callbacks;
    };

    class GraphicsProperties : public Exchange::IGraphicsProperties {
    public:
        GraphicsProperties(const std::string& memoryStatsFilepath, const std::string& memoryFreeKey, const std::string& memoryTotalKey, uint32_t multiplier)
            : _memoryStatsFile(memoryStatsFilepath)
            , _memoryFreeKey(memoryFreeKey)
            , _memoryTotalKey(memoryTotalKey)
            , _unitMultiplier(multiplier)
        {
        }

        GraphicsProperties(const GraphicsProperties&) = delete;
        const GraphicsProperties& operator=(const GraphicsProperties&) = delete;

        uint32_t TotalGpuRam(uint64_t& total) const override
        {
            total = GetMemory(_memoryTotalKey) * _unitMultiplier;
            return Core::ERROR_NONE;
        }

        uint32_t FreeGpuRam(uint64_t& free) const override
        {
            free = GetMemory(_memoryFreeKey) * _unitMultiplier;
            return Core::ERROR_NONE;
        }

        BEGIN_INTERFACE_MAP(GraphicsProperties)
        INTERFACE_ENTRY(Exchange::IGraphicsProperties)
        END_INTERFACE_MAP

    private:
        uint64_t GetMemory(const std::string& key) const
        {
            auto extractNumbers = [](const std::string& str) {
                string value;
                if (!str.empty()) {
                    auto first = str.find_first_of("0123456789");
                    auto last = str.find_last_of("0123456789");
                    value = str.substr(first, last - first + 1);
                }
                return std::stoul(!value.empty() ? value : "0");
            };

            auto value = GetValueForKey(_memoryStatsFile, key);
            return extractNumbers(value);
        }

        std::string GetValueForKey(const std::string& filepath, const std::string& key) const
        {
            std::ifstream statusFile(filepath);
            std::string result, line;

            if (statusFile.is_open()) {
                while (result.empty() && getline(statusFile, line)) {
                    if (line.find(key) != std::string::npos)
                        result = line;
                }
                statusFile.close();
            } else {
                TRACE(Trace::Error, (_T("Could not open GPU stats file: %s"), filepath.c_str()));
            }
            return result;
        }

        std::string _memoryStatsFile;
        std::string _memoryFreeKey;
        std::string _memoryTotalKey;
        uint32_t _unitMultiplier;
    };

    class HDRProperties : public Exchange::IHDRProperties {
    public:
        HDRProperties(const std::string& hdrLevelFilepath)
            : _hdrLevelFilepath(hdrLevelFilepath)
        {
        }

        HDRProperties(const HDRProperties&) = delete;
        const HDRProperties& operator=(const HDRProperties&) = delete;

        uint32_t TVCapabilities(VARIABLE_IS_NOT_USED IHDRIterator*& type) const override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t STBCapabilities(VARIABLE_IS_NOT_USED IHDRIterator*& type) const override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t HDRSetting(HDRType& type) const override
        {
            type = GetHDRLevel();
            return Core::ERROR_NONE;
        }

        Exchange::IHDRProperties::HDRType GetHDRLevel() const
        {
            Exchange::IHDRProperties::HDRType hdrType;
            std::string hdrStr = getLine(_hdrLevelFilepath);

            if (hdrStr == "SDR") {
                hdrType = Exchange::IHDRProperties::HDRType::HDR_OFF;
            } else if (hdrStr == "HDR10-GAMMA_ST2084") {
                hdrType = Exchange::IHDRProperties::HDRType::HDR_10;
            } else if (hdrStr == "HDR10-GAMMA_HLG") {
                hdrType = Exchange::IHDRProperties::HDRType::HDR_HLG;
            } else if (hdrStr == "HDR10Plus-VSIF") {
                hdrType = Exchange::IHDRProperties::HDRType::HDR_10PLUS;
            } else if (hdrStr == "DolbyVision-Std" || hdrStr == "DolbyVision-Lowlatency") {
                hdrType = Exchange::IHDRProperties::HDRType::HDR_DOLBYVISION;
            } else if (hdrStr == "HDR10-others") {
                TRACE(Trace::Warning, (_T("Received unknown HDR10 value. Falling back to HDR10.")));
                hdrType = Exchange::IHDRProperties::HDRType::HDR_10;
            } else {
                hdrType = Exchange::IHDRProperties::HDRType::HDR_OFF;
                TRACE(Trace::Error, (_T("Received unknown HDR value %s. Falling back to SDR."), hdrStr.c_str()));
            }

            return hdrType;
        }

        BEGIN_INTERFACE_MAP(HDRProperties)
        INTERFACE_ENTRY(Exchange::IHDRProperties)
        END_INTERFACE_MAP

    private:
        std::string _hdrLevelFilepath;
    };

    class DisplayProperties {
    public:
        DisplayProperties(const std::string& drmDeviceName,
            const std::string& edidFilepath,
            const std::string& hdcpLevelFilepath,
            bool usePreferredMode = false)
            : _usePreferredMode(usePreferredMode)
            , _drmDevice(drmDeviceName)
            , _edidNode(edidFilepath)
            , _hdcpLevelNode(hdcpLevelFilepath)
            , _propertiesLock()
            , _audioPassthrough(false)
            , _edid()
            , _hdcpLevel(Exchange::IConnectionProperties::HDCPProtectionType::HDCP_Unencrypted)
        {
            RequeryProperties();
        };

        ~DisplayProperties() = default;

        bool Connected() const
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);
            return _drmConnector ? _drmConnector->IsConnected() : 0;
        }

        uint32_t Width() const
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);
            return _drmConnector ? _drmConnector->Width() : 0;
        }

        uint32_t Height() const
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);
            return _drmConnector ? _drmConnector->Height() : 0;
        }

        uint32_t RefreshRate() const
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);
            return _drmConnector ? _drmConnector->RefreshRate() : 0;
        }

        Exchange::IConnectionProperties::HDCPProtectionType HDCP() const
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);
            return _hdcpLevel;
        }

        bool AudioPassthrough() const
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);
            return _audioPassthrough;
        }

        std::vector<uint8_t> EDID() const
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);
            return _edid;
        }

        void Reauthenticate()
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);

            std::string hdcpStr = getLine(_hdcpLevelNode);
            if (hdcpStr.find("succeed") != std::string::npos) {
                if (hdcpStr.find("22") != std::string::npos) {
                    _hdcpLevel = Exchange::IConnectionProperties::HDCPProtectionType::HDCP_2X;
                } else if (hdcpStr.find("14") != std::string::npos) {
                    _hdcpLevel = Exchange::IConnectionProperties::HDCPProtectionType::HDCP_1X;
                } else {
                    TRACE(Trace::Error, (_T("Received HDCP value: %s"), hdcpStr.c_str()));
                    _hdcpLevel = Exchange::IConnectionProperties::HDCPProtectionType::HDCP_Unencrypted;
                }
            } else {
                _hdcpLevel = Exchange::IConnectionProperties::HDCPProtectionType::HDCP_Unencrypted;
            }

            TRACE(Trace::Information, (_T("HDCP protection level: %d"), _hdcpLevel));
        }

        void RequeryProperties()
        {
            QueryDisplayProperties();
            Reauthenticate();
            QueryEDID();
        }

    private:
        void QueryDisplayProperties()
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);

            _drmConnector.reset(new Linux::DRMConnector(_drmDevice, DRM_MODE_CONNECTOR_HDMIA, _usePreferredMode));

            /* clang-format off */
            TRACE(Trace::Information, (_T("HDMI status: %d, Resolution: %dx%d @ %d Hz")
                , (_drmConnector ? _drmConnector->IsConnected() : 0)
                , _drmConnector ? _drmConnector->Width() : 0
                , _drmConnector ? _drmConnector->Height() : 0
                , _drmConnector ? _drmConnector->RefreshRate() : 0));
            /* clang-format on */
        }

        void QueryEDID()
        {
            std::lock_guard<std::mutex> lock(_propertiesLock);

            std::ifstream instream(_edidNode, std::ios::in);
            _edid.reserve(512);
            if (instream.is_open()) {
                char asciiHexByte[2];
                while(!instream.eof()) {
                    instream.read(asciiHexByte, 2);
                    _edid.push_back(std::strtoul(asciiHexByte, nullptr, 16));
                }
            } else {
                _edid.clear();
            }
        }

        bool _usePreferredMode;
        std::string _drmDevice;
        std::string _edidNode;
        std::string _hdcpLevelNode;

        mutable std::mutex _propertiesLock;
        std::unique_ptr<Linux::DRMConnector> _drmConnector;
        bool _audioPassthrough;
        std::vector<uint8_t> _edid;
        Exchange::IConnectionProperties::HDCPProtectionType _hdcpLevel;
    };

    class DisplayInfoImplementation : public Exchange::IConnectionProperties,
                                      public Exchange::IConfiguration,
                                      public Exchange::IDisplayProperties {

    public:
        DisplayInfoImplementation(const DisplayInfoImplementation&) = delete;
        DisplayInfoImplementation& operator=(const DisplayInfoImplementation&) = delete;
        ~DisplayInfoImplementation() override
        {
            _activity.Revoke();
            _eventQueue.Stop();
            _graphics->Release();
            _hdr->Release();
        }

        DisplayInfoImplementation()
            : _config()
            , _udevObserver()
            , _callback([&](const std::string& devtype){
                if (devtype == "hdcp") {
                    _eventQueue.Post(Exchange::IConnectionProperties::INotification::Source::HDCP_CHANGE);
                } else {
                    _eventQueue.Post(Exchange::IConnectionProperties::INotification::Source::HDMI_CHANGE);
                }
            })
            , _eventQueue(4, *this)
            , _display(nullptr)
            , _graphics(nullptr)
            , _hdr(nullptr)
            , _activity(*this)
            , _observers()
            , _observersLock()
        {
            _udevObserver.Register(_callback, "drm_minor");
            _udevObserver.Register(_callback, "hdcp");
        }

        uint32_t Configure(PluginHost::IShell* framework) override
        {
            _config.FromString(framework->ConfigLine());
            
            /* clang-format off */
            _display.reset(new DisplayProperties(Config::GetValue(_config.drmDeviceName)
            , Config::GetValue(_config.edidFilepath)
            , Config::GetValue(_config.hdcpLevelFilepath)
            , Config::GetValue(_config.usePreferredMode)));

            _graphics = Core::Service<GraphicsProperties>::Create<Exchange::IGraphicsProperties>(Config::GetValue(_config.gpuMemoryFile)
                , Config::GetValue(_config.gpuMemoryFreePattern)
                , Config::GetValue(_config.gpuMemoryTotalPattern)
                , Config::GetValue(_config.gpuMemoryUnitMultiplier));

            _hdr = Core::Service<HDRProperties>::Create<Exchange::IHDRProperties>(Config::GetValue(_config.hdrLevelFilepath));
            /* clang-format on */

            return Core::ERROR_NONE;
        }

        uint32_t Register(IConnectionProperties::INotification* notification) override
        {
            std::lock_guard<std::mutex> lock(_observersLock);

            auto index = std::find(_observers.begin(), _observers.end(), notification);
            ASSERT(index == _observers.end());

            if (index == _observers.end()) {
                _observers.push_back(notification);
                notification->AddRef();
            }

            return (Core::ERROR_NONE);
        }

        uint32_t Unregister(IConnectionProperties::INotification* notification) override
        {
            std::lock_guard<std::mutex> lock(_observersLock);

            std::list<IConnectionProperties::INotification*>::iterator index(std::find(_observers.begin(), _observers.end(), notification));

            ASSERT(index != _observers.end());

            if (index != _observers.end()) {
                (*index)->Release();
                _observers.erase(index);
            }

            return (Core::ERROR_NONE);
        }

        uint32_t IsAudioPassthrough(VARIABLE_IS_NOT_USED bool& passthru) const override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t Connected(bool& isconnected) const override
        {
            uint32_t result = Core::ERROR_NONE;

            if (_display) {
                isconnected = _display->Connected();
            } else {
                result = Core::ERROR_UNAVAILABLE;
            }
            return result;
        }

        uint32_t Width(uint32_t& width) const override
        {
            uint32_t result = Core::ERROR_NONE;

            if (_display) {
                width = _display->Width();
            } else {
                result = Core::ERROR_UNAVAILABLE;
            }
            return result;
        }

        uint32_t Height(uint32_t& height) const override
        {
            uint32_t result = Core::ERROR_NONE;

            if (_display) {
                height = _display->Height();
            } else {
                result = Core::ERROR_UNAVAILABLE;
            }
            return result;
        }

        uint32_t VerticalFreq(uint32_t& vf) const override
        {
            uint32_t result = Core::ERROR_NONE;

            if (_display) {
                vf = _display->RefreshRate();
            } else {
                result = Core::ERROR_UNAVAILABLE;
            }
            return result;
        }

        uint32_t EDID(uint16_t& length, uint8_t data[]) const override
        {
            uint32_t result = Core::ERROR_NONE;

            if (_display && length > 0) {
                auto edid = _display->EDID();
                memcpy(data, edid.data(), edid.size());
                length = edid.size();
            } else {
                result = Core::ERROR_UNAVAILABLE;
            }
            return result;
        }

        uint32_t WidthInCentimeters(VARIABLE_IS_NOT_USED uint8_t& width /* @out */) const override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t HeightInCentimeters(VARIABLE_IS_NOT_USED uint8_t& heigth /* @out */) const override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t HDCPProtection(HDCPProtectionType& value) const override
        {
            uint32_t result = Core::ERROR_NONE;

            if (_display) {
                value = _display->HDCP();
            } else {
                result = Core::ERROR_UNAVAILABLE;
            }
            return result;
        }

        uint32_t HDCPProtection(VARIABLE_IS_NOT_USED const HDCPProtectionType value) override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t PortName(VARIABLE_IS_NOT_USED string& name) const override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        void Dispatch() const
        {
            std::lock_guard<std::mutex> lock(_observersLock);

            std::list<IConnectionProperties::INotification*>::const_iterator index = _observers.begin();

            if (index != _observers.end()) {
                (*index)->Updated(IConnectionProperties::INotification::Source::HDMI_CHANGE);
            }
        }

        uint32_t ColorSpace(ColourSpaceType&) const override
        {
            return (Core::ERROR_UNAVAILABLE);
        }

        uint32_t FrameRate(FrameRateType&) const override
        {
            return (Core::ERROR_UNAVAILABLE);
        }

        uint32_t ColourDepth(ColourDepthType&) const override
        {
            return (Core::ERROR_UNAVAILABLE);
        }

        uint32_t Colorimetry(IColorimetryIterator*&) const override
        {
            return (Core::ERROR_UNAVAILABLE);
        }

        uint32_t QuantizationRange(QuantizationRangeType&) const override
        {
            return (Core::ERROR_UNAVAILABLE);
        }

        uint32_t EOTF(EotfType&) const override
        {
            return (Core::ERROR_UNAVAILABLE);
        }

        BEGIN_INTERFACE_MAP(DisplayInfoImplementation)
        INTERFACE_ENTRY(Exchange::IConnectionProperties)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        INTERFACE_ENTRY(Exchange::IDisplayProperties)
        INTERFACE_AGGREGATE(Exchange::IGraphicsProperties, _graphics)
        INTERFACE_AGGREGATE(Exchange::IHDRProperties, _hdr)
        END_INTERFACE_MAP

    private:
        class Config : public Core::JSON::Container {
        public:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

            Config()
                : Core::JSON::Container()
                , usePreferredMode()
                , drmDeviceName()
                , edidFilepath()
                , hdcpLevelFilepath()
                , hdrLevelFilepath()
                , gpuMemoryFile()
                , gpuMemoryFreePattern()
                , gpuMemoryTotalPattern()
            {
                Add(_T("usePreferredMode"), &usePreferredMode);
                Add(_T("drmDeviceName"), &drmDeviceName);
                Add(_T("edidFilepath"), &edidFilepath);
                Add(_T("hdcpLevelFilepath"), &hdcpLevelFilepath);
                Add(_T("hdrLevelFilepath"), &hdrLevelFilepath);
                Add(_T("gpuMemoryFile"), &gpuMemoryFile);
                Add(_T("gpuMemoryFreePattern"), &gpuMemoryFreePattern);
                Add(_T("gpuMemoryTotalPattern"), &gpuMemoryTotalPattern);
                Add(_T("gpuMemoryUnitMultiplier"), &gpuMemoryUnitMultiplier);
            }

            static std::string GetValue(const Core::JSON::String& jsonValue) { return jsonValue.IsSet() ? jsonValue.Value() : ""; }
            static bool GetValue(const Core::JSON::Boolean& jsonValue) { return jsonValue.IsSet() ? jsonValue.Value() : false; }
            static int32_t GetValue(const Core::JSON::DecUInt32& jsonValue) { return jsonValue.IsSet() ? jsonValue.Value() : 0; }

            Core::JSON::Boolean usePreferredMode;
            Core::JSON::String drmDeviceName;
            Core::JSON::String edidFilepath;
            Core::JSON::String hdcpLevelFilepath;
            Core::JSON::String hdrLevelFilepath;
            Core::JSON::String gpuMemoryFile;
            Core::JSON::String gpuMemoryFreePattern;
            Core::JSON::String gpuMemoryTotalPattern;
            Core::JSON::DecUInt32 gpuMemoryUnitMultiplier;
        };

        class EventQueue : public Core::Thread {
        public:
            EventQueue(const EventQueue&) = delete;
            const EventQueue& operator=(const EventQueue&) = delete;

            EventQueue(int elementLimit, DisplayInfoImplementation& parent)
                : _eventQueue(elementLimit)
                , _arrived(false, true)
                , _parent(parent)
            {
                Run();
            }

            ~EventQueue()
            {
                Stop();
                _arrived.SetEvent();
            }

            bool Post(IConnectionProperties::INotification::Source type)
            {
                if (_eventQueue.Post(type)) {
                    _arrived.SetEvent();
                    return true;
                } else {
                    return false;
                }
            }

            uint32_t Worker() override
            {
                _arrived.Lock();
                _arrived.ResetEvent();
                if (IsRunning()) {
                    IConnectionProperties::INotification::Source eventType;
                    bool isExtracted = _eventQueue.Extract(eventType, Core::infinite);
                    if (isExtracted && eventType == IConnectionProperties::INotification::Source::HDCP_CHANGE) {
                        _parent._display->Reauthenticate();
                    } else if (isExtracted && eventType == IConnectionProperties::INotification::Source::HDMI_CHANGE) {
                        _parent._display->RequeryProperties();
                    }
                }
                return Core::infinite;
            };

        private:
            Core::QueueType<IConnectionProperties::INotification::Source> _eventQueue;
            Core::Event _arrived;
            DisplayInfoImplementation& _parent;
        };

        Config _config;

        UdevObserverType _udevObserver;
        std::function<void(const std::string&)> _callback;
        EventQueue _eventQueue;

        std::unique_ptr<DisplayProperties> _display;
        Exchange::IGraphicsProperties* _graphics;
        Exchange::IHDRProperties* _hdr;

        Core::WorkerPool::JobType<DisplayInfoImplementation&> _activity;
        std::list<IConnectionProperties::INotification*> _observers;
        mutable std::mutex _observersLock;
    };

    SERVICE_REGISTRATION(DisplayInfoImplementation, 1, 0);
}
}
