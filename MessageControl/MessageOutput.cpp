/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#include "MessageOutput.h"

namespace WPEFramework {

namespace Publishers {

    string Text::Convert(const Core::Messaging::MessageInfo& metadata, const string& text) /* override */
    {
        ASSERT(metadata.Type() != Core::Messaging::Metadata::type::INVALID);

        string output = metadata.ToString(_abbreviated).c_str() +
                        Core::Format("%s\n", text.c_str());

        return (output);
    }

    void ConsoleOutput::Message(const Core::Messaging::MessageInfo& metadata, const string& text) /* override */
    {
        std::cout << _convertor.Convert(metadata, text);
    }

    void SyslogOutput::Message(const Core::Messaging::MessageInfo& metadata, const string& text) /* override */
    {
#ifndef __WINDOWS__
        syslog(LOG_NOTICE, _T("%s"), _convertor.Convert(metadata, text).c_str());
#else
        printf(_T("%s"), _convertor.Convert(metadata, text).c_str());
#endif
    }

    void FileOutput::Message(const Core::Messaging::MessageInfo& metadata, const string& text) /* override */
    {
        if (_file.IsOpen()) {
            const string line = _convertor.Convert(metadata, text);
            _file.Write(reinterpret_cast<const uint8_t*>(line.c_str()), static_cast<uint32_t>(line.length()));
        }
    }

    void JSON::Convert(const Core::Messaging::MessageInfo& metadata, const string& text, Data& data)
    {
        ExtraOutputOptions options = _outputOptions;

        if ((AsNumber(options) & AsNumber(ExtraOutputOptions::PAUSED)) == 0) {

            if ((AsNumber(options) & AsNumber(ExtraOutputOptions::CATEGORY)) != 0) {
                data.Category = metadata.Category();
            }

            if ((AsNumber(options) & AsNumber(ExtraOutputOptions::MODULE)) != 0) {
                data.Module = metadata.Module();
            }

            if (metadata.Type() == Core::Messaging::Metadata::type::TRACING) {
                ASSERT(dynamic_cast<const Core::Messaging::IStore::Tracing*>(&metadata) != nullptr);
                const Core::Messaging::IStore::Tracing& trace = static_cast<const Core::Messaging::IStore::Tracing&>(metadata);
                const Core::Time now(trace.TimeStamp());

                if ((AsNumber(options) & AsNumber(ExtraOutputOptions::INCLUDINGDATE)) != 0) {
                    data.Time = now.ToRFC1123(true);
                }
                else {
                    data.Time = now.ToTimeOnly(true);
                }

                if ((AsNumber(options) & AsNumber(ExtraOutputOptions::FILENAME)) != 0) {
                    data.FileName = trace.FileName();
                }

                if ((AsNumber(options) & AsNumber(ExtraOutputOptions::LINENUMBER)) != 0) {
                    data.LineNumber = trace.LineNumber();
                }

                if ((AsNumber(options) & AsNumber(ExtraOutputOptions::CLASSNAME)) != 0) {
                    data.ClassName = trace.ClassName();
                }
            }
            else if (metadata.Type() == Core::Messaging::Metadata::type::LOGGING) {
                ASSERT(dynamic_cast<const Core::Messaging::IStore::Logging*>(&metadata) != nullptr);
                const Core::Messaging::IStore::Logging& log = static_cast<const Core::Messaging::IStore::Logging&>(metadata);
                const Core::Time now(log.TimeStamp());

                if ((AsNumber(options) & AsNumber(ExtraOutputOptions::INCLUDINGDATE)) != 0) {
                    data.Time = now.ToRFC1123(true);
                }
                else {
                    data.Time = now.ToTimeOnly(true);
                }
            }
            else if (metadata.Type() == Core::Messaging::Metadata::type::REPORTING) {
                ASSERT(dynamic_cast<const Core::Messaging::IStore::WarningReporting*>(&metadata) != nullptr);
                const Core::Messaging::IStore::WarningReporting& report = static_cast<const Core::Messaging::IStore::WarningReporting&>(metadata);
                const Core::Time now(report.TimeStamp());

                if ((AsNumber(options) & AsNumber(ExtraOutputOptions::INCLUDINGDATE)) != 0) {
                    data.Time = now.ToRFC1123(true);
                }
                else {
                    data.Time = now.ToTimeOnly(true);
                }

                if ((AsNumber(options) & AsNumber(ExtraOutputOptions::CALLSIGN)) != 0) {
                    data.Callsign = report.Callsign();
                }
            }
            else {
                ASSERT(metadata.Type() != Core::Messaging::Metadata::type::INVALID);
            }

            data.Message = text;
        }
    }

    //UDPOutput
    UDPOutput::Channel::Channel(const Core::NodeId& nodeId)
        : Core::SocketDatagram(false, nodeId.Origin(), nodeId, Messaging::MessageUnit::DataSize, 0)
        , _loaded(0)
    {
        ::memset(_sendBuffer, 0, sizeof(_sendBuffer));
    }
    UDPOutput::Channel::~Channel() {
        Close(Core::infinite);
    }

    uint16_t UDPOutput::Channel::SendData(uint8_t* dataFrame, const uint16_t maxSendSize)
    {
        _adminLock.Lock();

        uint16_t actualByteCount = (_loaded > maxSendSize ? maxSendSize : _loaded);
        memcpy(dataFrame, _sendBuffer, actualByteCount);
        _loaded = 0;

        _adminLock.Unlock();

        return (actualByteCount);
    }

    //unused
    uint16_t UDPOutput::Channel::ReceiveData(uint8_t*, const uint16_t) {
        return 0;
    }

    void UDPOutput::Channel::StateChange()
    {
    }

    void UDPOutput::Channel::Output(const Core::Messaging::Metadata& metadata, const Core::Messaging::IEvent* message)
    {
        _adminLock.Lock();

        uint16_t length = 0;
        ASSERT(metadata.Type() != Core::Messaging::Metadata::INVALID);

        length += metadata.Serialize(_sendBuffer + length, sizeof(_sendBuffer) - length);
        length += message->Serialize(_sendBuffer + length, sizeof(_sendBuffer) - length);
        _loaded = length;

        _adminLock.Unlock();

        Trigger();
    }

    UDPOutput::UDPOutput(const Core::NodeId& nodeId)
        : _output(nodeId) {
        _output.Open(0);
    }

    void UDPOutput::Message(const Core::Messaging::MessageInfo& metadata, const string& text) /* override */
    {
        //yikes, recreating stuff from received pieces
        Messaging::TextMessage textMessage(text);
        _output.Output(metadata, &textMessage);
    }

} // namespace Publishers
}
