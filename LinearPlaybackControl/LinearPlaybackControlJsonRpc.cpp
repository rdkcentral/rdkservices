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
 
#include "Module.h"
#include "LinearPlaybackControl.h"
#include <interfaces/json/JsonData_LinearPlaybackControl.h>
#include <inttypes.h>
#include <fstream>

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::LinearPlaybackControl;

    namespace {
        // Helper function for converting a IDemuxer:IO_STATUS
        // status to an associated WPEFramework:Core status
        inline uint32_t DmxStatusToCoreStatus(IDemuxer::IO_STATUS status)
        {
            switch (status)
            {
                case DemuxerStreamFsFCC::OK:
                    return Core::ERROR_NONE;
                case DemuxerStreamFsFCC::WRITE_ERROR:
                    syslog(LOG_ERR, "Demuxer failed with error: %d", status);
                    return Core::ERROR_WRITE_ERROR;
                default:
                    syslog(LOG_ERR, "Demuxer failed with error: %d", status);
                    return Core::ERROR_READ_ERROR;
            }
        }
    }

    // Registration
    //

    void LinearPlaybackControl::RegisterAll()
    {
        Property<ChannelData>(_T("channel"), &LinearPlaybackControl::endpoint_get_channel, &LinearPlaybackControl::endpoint_set_channel, this);
        Property<SeekData>(_T("seek"), &LinearPlaybackControl::endpoint_get_seek, &LinearPlaybackControl::endpoint_set_seek, this);
        Property<TrickplayData>(_T("trickplay"), &LinearPlaybackControl::endpoint_get_trickplay, &LinearPlaybackControl::endpoint_set_trickplay, this);
        Property<StatusData>(_T("status"), &LinearPlaybackControl::endpoint_get_status, nullptr, this);
        Property<TracingData>(_T("tracing"), &LinearPlaybackControl::endpoint_get_tracing, &LinearPlaybackControl::endpoint_set_tracing, this);
    }

    void LinearPlaybackControl::UnregisterAll()
    {
        PluginHost::JSONRPC::Unregister(_T("channel"));
        PluginHost::JSONRPC::Unregister(_T("seek"));
        PluginHost::JSONRPC::Unregister(_T("trickplay"));
        PluginHost::JSONRPC::Unregister(_T("status"));
        PluginHost::JSONRPC::Unregister(_T("tracing"));
    }

    // API implementation
    //

    // Property: channel - Set channel URI for a certain demuxer ID (optional - default is 0)
    // Valid input formats:
    //      1.) ${IP}:${PORT} - valid IP and port number
    //      2.) ${IP}         - default port 8433 will be used.
    //
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_BAD_REQUEST: Bad JSON param data format
    //  - ERROR_WRITE_ERROR: Error writing to file
    uint32_t LinearPlaybackControl::endpoint_set_channel(const string& demuxerId,const ChannelData& params)
    {
        // Make sure that channel URI is defined (empty string is ok), otherwise return ERROR_BAD_REQUEST
        if (params.Channel.IsSet() == false) {
            return Core::ERROR_BAD_REQUEST;
        }

        syslog(LOG_ERR, "Invoked LinearPlaybackControl::endpoint_set_channel");
        return callDemuxer(demuxerId,
                           [&](IDemuxer* dmx)->uint32_t {
                               return DmxStatusToCoreStatus(dmx->setChannel(params.Channel.Value()));
                           });
    }

    // Property: channel - Get channel URI for a certain demuxer ID (optional - default is 0)
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_READ_ERROR: Error reading file or parsing one or more values
    uint32_t LinearPlaybackControl::endpoint_get_channel(const string& demuxerId, ChannelData& params) const
    {
        syslog(LOG_ERR, "Invoked LinearPlaybackControl::endpoint_get_channel");
        return callDemuxer(demuxerId,
                           [&](IDemuxer* dmx)->uint32_t {
                               std::string chan;
                               auto result = DmxStatusToCoreStatus(dmx->getChannel(chan));
                               params.Channel = chan;
                               return result;
                           });
    }

    // Property: seek - Set seek position for current stream for a certain demuxer ID (optional - default is 0)
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_BAD_REQUEST: Bad JSON param data format
    //  - ERROR_WRITE_ERROR: Error writing to file
    uint32_t LinearPlaybackControl::endpoint_set_seek(const string& demuxerId, const SeekData& params)
    {
        syslog(LOG_ERR, "Invoked LinearPlaybackControl::endpoint_set_seek");

        // Make sure that the seek is defined, otherwise return ERROR_BAD_REQUEST
        if (params.SeekPosInSeconds.IsSet() == false) {
            return Core::ERROR_BAD_REQUEST;
        }

        return callDemuxer(demuxerId,
                           [&](IDemuxer* dmx)->uint32_t {
                               return DmxStatusToCoreStatus(dmx->setSeekPosInSeconds(params.SeekPosInSeconds.Value()));
                           });
    }

    // Property: seek - Get seek position for current stream for a certain demuxer ID (optional - default is 0)
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_BAD_REQUEST: Bad JSON param data format
    //  - ERROR_READ_ERROR: Error reading file or parsing one or more values
    uint32_t LinearPlaybackControl::endpoint_get_seek(const string& demuxerId, SeekData& params) const
    {
        syslog(LOG_DEBUG, "Invoked LinearPlaybackControl::endpoint_get_seek");
        return callDemuxer(demuxerId,
                           [&](IDemuxer* dmx)->uint32_t {
                               IDemuxer::SeekType seek;
                               auto status = DmxStatusToCoreStatus(dmx->getSeek(seek));
                               if (status == Core::ERROR_NONE) {
                                   params.SeekPosInSeconds = seek.seekPosInSeconds;
                               }
                               return status;
                           });
    }

    // Property: trickplay - Set trick play speed and direction for current stream for a certain demuxer ID (optional - default is 0)
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_BAD_REQUEST: Bad JSON param data format
    //  - ERROR_WRITE_ERROR: Error writing to file
    uint32_t LinearPlaybackControl::endpoint_set_trickplay(const string& demuxerId, const TrickplayData& params)
    {
        syslog(LOG_ERR, "Invoked LinearPlaybackControl::endpoint_set_trickplay");

        // Make sure that speed is defined, otherwise return ERROR_BAD_REQUEST
        if (params.Speed.IsSet() == false) {
            return Core::ERROR_BAD_REQUEST;
        }

        return callDemuxer(demuxerId,
                           [&](IDemuxer* dmx)->uint32_t {
                               return DmxStatusToCoreStatus(dmx->setTrickPlaySpeed(params.Speed.Value()));
                           });
    }

    // Property: trickplay - Get trick play speed and direction for current stream for a certain demuxer ID (optional - default is 0)
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_BAD_REQUEST: Bad JSON param data format
    //  - ERROR_READ_ERROR: Error reading file or parsing one or more values
    uint32_t LinearPlaybackControl::endpoint_get_trickplay(const string& demuxerId, TrickplayData& params) const
    {
        syslog(LOG_DEBUG, "Invoked LinearPlaybackControl::endpoint_get_seek");
        return callDemuxer(demuxerId,
                           [&](IDemuxer* dmx)->uint32_t {
                               int16_t speed;
                               auto status = DmxStatusToCoreStatus(dmx->getTrickPlaySpeed(speed));
                               if (status == Core::ERROR_NONE) {
                                   params.Speed = speed;
                               }
                               return status;
                           });
    }

    // Property: status - Get TSB status (seek position, TSB size and health info) for a certain demuxer ID (optional - default is 0)
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_READ_ERROR: Error reading file or parsing one or more values
    uint32_t LinearPlaybackControl::endpoint_get_status(const string& demuxerId, StatusData& params) const
    {
        syslog(LOG_DEBUG, "Invoked LinearPlaybackControl::endpoint_get_status");
        return callDemuxer(demuxerId,
                           [&](IDemuxer* dmx)->uint32_t {
                               // Parameter declaration
                               int16_t speed;
                               IDemuxer::SeekType seek;
                               IDemuxer::StreamStatusType streamStatus;
                               // Get parameters from selected demuxer.
                               // Note: OR operation is used for concatenating the status since possible
                               // set of status is ERROR_NONE (0) or ERROR_READ_ERROR (39)
                               uint32_t status = DmxStatusToCoreStatus(dmx->getSeek(seek));
                               status |= DmxStatusToCoreStatus(dmx->getTrickPlaySpeed(speed));
                               status |= DmxStatusToCoreStatus(dmx->getStreamStatus(streamStatus));
                               // Set parameters in JSON response
                               if (status == Core::ERROR_NONE) {
                                   params.SeekPosInSeconds      = seek.seekPosInSeconds;
                                   params.SeekPosInBytes        = seek.seekPosInBytes;
                                   params.CurrentSizeInSeconds  = seek.currentSizeInSeconds;
                                   params.CurrentSizeInBytes    = seek.currentSizeInBytes;
                                   params.MaxSizeInBytes        = seek.maxSizeInBytes;
                                   params.TrickPlaySpeed        = speed;
                                   params.StreamSourceLost      = streamStatus.streamSourceLost;
                                   params.StreamSourceLossCount = streamStatus.streamSourceLossCount;
                               }
                               return status;
                           });
    }

    // Property: tracing - Set tracing enabled/disabled
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_BAD_REQUEST: Bad JSON param data format
    uint32_t LinearPlaybackControl::endpoint_set_tracing(const TracingData& params)
    {
        // The following stub implementation is added for testing only.
        uint32_t result = Core::ERROR_NONE;
        const bool enabled = params.Tracing.Value();
        syslog(LOG_ERR, "Invoked LinearPlaybackControl::endpoint_set_tracing with tracing = %s", enabled ? "enabled" : "disabled");
        return result;
    }

    // Property: tracing - Get tracing enabled/disabled
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_BAD_REQUEST: Bad JSON param data format
    uint32_t LinearPlaybackControl::endpoint_get_tracing(TracingData &params) const
    {
        // The following stub implementation is added for testing only.
        uint32_t result = Core::ERROR_NONE;
        params.Tracing = true;
        syslog(LOG_ERR, "Invoked LinearPlaybackControl::endpoint_get_tracing");
        return result;
    }

    uint32_t LinearPlaybackControl::callDemuxer(const string& demuxerId, const std::function<endpoint_func>& func) const
    {
        if(!_isStreamFSEnabled) {
            syslog(LOG_ERR, "No demuxer set");
            return Core::ERROR_BAD_REQUEST;
        }
        // In a future implementation, the concrete demuxer instance shall be obtained based
        // on the demuxerId. For now we just use the _demuxer instance, associated with
        // demuxId = 0, when invoking the lambda function for interacting with the instance.
        return func(_demuxer.get());
    }

    void LinearPlaybackControl::speedchangedNotify(const std::string &data) {
        uint16_t trickPlaySpeed;
        std::istringstream is(data);
        is >> trickPlaySpeed;

        SpeedchangedParamsData params;
        params.Speed = trickPlaySpeed;
        params.MuxId = 0;

        Notify(_T("speedchanged"), params);
    }
} // namespace Plugin
} // namespace WPEFramework
