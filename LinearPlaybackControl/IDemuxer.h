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

#pragma once

#include <string>

namespace WPEFramework {
namespace Plugin {

/**
 * Generic demuxer interface shared between Nokia FCC and QAM implementations
 */
class IDemuxer {
public:
    enum IO_STATUS {
        OK = 0x0,
        READ_ERROR = 0x1,
        WRITE_ERROR = 0x2,
        PARSE_ERROR = 0x3
    };

    struct SeekType {
        uint64_t seekPosInSeconds;
        uint64_t seekPosInBytes;
        uint64_t currentSizeInSeconds;
        uint64_t currentSizeInBytes;
        uint64_t maxSizeInBytes;
    };

    struct StreamStatusType {
        bool streamSourceLost;
        uint64_t streamSourceLossCount;
    };

public:
    virtual ~IDemuxer() = default;

    virtual IO_STATUS open() = 0;

    virtual IO_STATUS close() = 0;

    virtual IO_STATUS setChannel(const std::string& channel) = 0;

    virtual IO_STATUS getChannel(std::string& channel) = 0;

    virtual IO_STATUS setSeekPosInSeconds(uint64_t seekSeconds)= 0;

    virtual IO_STATUS getSeek(SeekType &result) = 0;

    virtual IO_STATUS setTrickPlaySpeed(int16_t speed)= 0;

    virtual IO_STATUS getTrickPlaySpeed(int16_t &result) = 0;

    virtual IO_STATUS getStreamStatus(StreamStatusType &result) = 0;

};

} // namespace Plugin
} // namespace WPEFramework