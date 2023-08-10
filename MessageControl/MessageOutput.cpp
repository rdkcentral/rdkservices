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

    string Text::Convert(const Core::Messaging::Metadata::type,
        const string& module, const string& category, const string& fileName,
        const uint16_t lineNumber, const string& className,
        const uint64_t timeStamp, const string& text) /* override */
    {
        string output;

        const Core::Time now(timeStamp);

        if (_abbreviated == true) {
            const string time(now.ToTimeOnly(true));
            output = Core::Format("[%s]:[%s]:[%s]: %s\n",
                    time.c_str(),
                    module.c_str(),
                    category.c_str(),
                    text.c_str());
        } else {
            const string time(now.ToRFC1123(true));
            output = Core::Format("[%s]:[%s:%u]:[%s]:[%s]: %s\n",
                    time.c_str(),
                    Core::FileNameOnly(fileName.c_str()),
                    lineNumber,
                    className.c_str(),
                    category.c_str(),
                    text.c_str());
        }

        return (output);
    }

    void ConsoleOutput::Message(const Core::Messaging::Metadata::type type,
        const string& module, const string& category, const string& fileName,
        const uint16_t lineNumber, const string& className,
        const uint64_t timeStamp, const string& text) /* override */
    {
        std::cout << _convertor.Convert(type, category,module,fileName, lineNumber, className, timeStamp, text);
    }

    void SyslogOutput::Message(const Core::Messaging::Metadata::type type,
            const string& module, const string& category, const string& fileName,
            const uint16_t lineNumber, const string& className,
            const uint64_t timeStamp, const string& text) /* override */
    {
#ifndef __WINDOWS__
        syslog(LOG_NOTICE, _T("%s"), _convertor.Convert(type, module, category, fileName, lineNumber, className, timeStamp, text).c_str());
#else
        printf(_T("%s"), _convertor.Convert(type, module, category, fileName, lineNumber, className, timeStamp, text).c_str());
#endif
    }

    void FileOutput::Message(const Core::Messaging::Metadata::type type,
        const string& module, const string& category, const string& fileName,
        const uint16_t lineNumber, const string& className,
        const uint64_t timeStamp, const string& text) /* override */
    {
        if (_file.IsOpen()) {
            string line = _convertor.Convert(type, module, category, fileName, lineNumber, className, timeStamp, text);
            _file.Write(reinterpret_cast<const uint8_t*>(line.c_str()), static_cast<uint32_t>(line.length()));
        }
    }

    void JSON::Convert(const Core::Messaging::Metadata::type,
        const string& module, const string& category, const string& fileName,
        const uint16_t lineNumber, const string& className,
        const uint64_t, const string& text, Data& data)
    {
        ExtraOutputOptions options = _outputOptions;

        if ((AsNumber(options) & AsNumber(ExtraOutputOptions::PAUSED)) == 0) {

            if ((AsNumber(options) & AsNumber(ExtraOutputOptions::INCLUDINGDATE)) != 0) {
                data.Time = Core::Time::Now().ToRFC1123(true);
            } else {
                data.Time = Core::Time::Now().ToTimeOnly(true);
            }

            if ((AsNumber(options) & AsNumber(ExtraOutputOptions::FILENAME)) != 0) {
                data.FileName = fileName;
            }

            if ((AsNumber(options) & AsNumber(ExtraOutputOptions::LINENUMBER)) != 0) {
                data.LineNumber = lineNumber;
            }

            if( (AsNumber(options) & AsNumber(ExtraOutputOptions::CLASSNAME) ) != 0 ) {
                data.ClassName = className;
            }

            if ((AsNumber(options) & AsNumber(ExtraOutputOptions::MODULE)) != 0) {
                data.Module = module;
            }

            if ((AsNumber(options) & AsNumber(ExtraOutputOptions::CATEGORY)) != 0) {
                data.Category = category;
            }

            data.Message = text;
        }
    }

    //UDPOutput
    UDPOutput::Channel::Channel(const Core::NodeId& nodeId)
        : Core::SocketDatagram(false, nodeId.Origin(), nodeId, Messaging::MessageUnit::DataSize, 0)
        , _loaded(0)
    {
    }
    UDPOutput::Channel::~Channel()
    {
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
    uint16_t UDPOutput::Channel::ReceiveData(uint8_t*, const uint16_t)
    {
        return 0;
    }
    void UDPOutput::Channel::StateChange()
    {
    }

    void UDPOutput::Channel::Output(const Core::Messaging::IStore::Information& info, const Core::Messaging::IEvent* message)
    {
        _adminLock.Lock();

        uint16_t length = 0;
        length += info.Serialize(_sendBuffer + length, sizeof(_sendBuffer) - length);
        length += message->Serialize(_sendBuffer + length, sizeof(_sendBuffer) - length);
        _loaded = length;

        _adminLock.Unlock();

        Trigger();
    }

    UDPOutput::UDPOutput(const Core::NodeId& nodeId)
        : _output(nodeId)
    {
        _output.Open(0);
    }

    void UDPOutput::Message(const Core::Messaging::Metadata::type type,
            const string& module, const string& category, const string& fileName,
            const uint16_t lineNumber, const string& className,
            const uint64_t timeStamp, const string& text) /* override */
    {
        //yikes, recreating stuff from received pieces
        Messaging::TextMessage textMessage(text);
        Core::Messaging::IStore::Information info(Core::Messaging::Metadata(type, category, module), fileName, lineNumber, className, timeStamp);

        _output.Output(info, &textMessage);
    }
}
}
