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
#include <stdint.h>
#include "LinearConfig.h"
#include "IDemuxer.h"

namespace WPEFramework {
namespace Plugin {

#define FCC_PLUGIN_PATH "fcc"

// File system separator
static inline std::string separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

/**
 * Concrete implementation of a demuxer interface for Nokia FCC
 */
class DemuxerStreamFsFCC : public IDemuxer {
    public:
        explicit DemuxerStreamFsFCC(LinearConfig::Config *config, uint8_t demuxId)
        {
            _fileChannel   = config->MountPoint.Value() + separator() + FCC_PLUGIN_PATH + separator() + "chan_select" + std::to_string(demuxId);
            _fileSeek      = config->MountPoint.Value() + separator() + FCC_PLUGIN_PATH + separator() + "seek" + std::to_string(demuxId);
            _fileTrickPlay = config->MountPoint.Value() + separator() + FCC_PLUGIN_PATH + separator() + "trick_play" + std::to_string(demuxId);
            _fileStatus    = config->MountPoint.Value() + separator() + FCC_PLUGIN_PATH + separator() + "stream_status";
        };

        ~DemuxerStreamFsFCC()
        {}

    IDemuxer::IO_STATUS open() override;

    IDemuxer::IO_STATUS close() override;

    IDemuxer::IO_STATUS setChannel(const string &channel) override;

    IDemuxer::IO_STATUS getChannel(string &channel) override;

    IDemuxer::IO_STATUS setSeekPosInSeconds(uint64_t seekSeconds) override;

    IDemuxer::IO_STATUS getSeek(SeekType &result) override;

    IDemuxer::IO_STATUS setTrickPlaySpeed(int16_t speed) override;

    IDemuxer::IO_STATUS getTrickPlaySpeed(int16_t &result) override;

    IDemuxer::IO_STATUS getStreamStatus(StreamStatusType &result) override;

    std::string getTrickPlayFile() const;

private:
    std::string _fileChannel;
    std::string _fileSeek;
    std::string _fileTrickPlay;
    std::string _fileStatus;
};

} // namespace Plugin
} // namespace WPEFramework
