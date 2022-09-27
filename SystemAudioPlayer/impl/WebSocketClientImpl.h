/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include <atomic>
#include <functional>

#include "AudioPlayer.h"
#include "logger.h"
#include "WebSockets/WSEndpoint.h"
#include "WebSockets/PingPong/PingPongEnabled.h"
#include "WebSockets/Roles/Client.h"
#include "WebSockets/CommunicationInterface/BinaryInterface.h"
#include "WebSockets/ConnectionInitializationResult.h"


namespace impl {

template <template <typename, typename> typename Encryption>
class WebSocketClientImpl
{
public:
    explicit WebSocketClientImpl(AudioPlayer* player);
    void connect(const std::string& uri);
    void disconnect();

    WebSockets::WSEndpoint<
        WebSockets::Client,
        WebSockets::BinaryInterface,
        WebSockets::PingPongEnabled,
        Encryption
    > wsClient_;
private:
    WebSocketClientImpl() = delete;
    WebSocketClientImpl(const WebSocketClientImpl&) = delete;
    WebSocketClientImpl& operator=(const WebSocketClientImpl&) = delete;

    void onServiceConnection(WebSockets::ConnectionInitializationResult result);
    void onServiceDisconnected();
    void onMessage(const std::string& msg);
    std::string removeProtocol(const std::string& uri) const;
    std::string ensureAddressHasPortNumber(std::string address) const;

    std::atomic_bool connected_{false};
    AudioPlayer* player_;
    const std::string defaultAudioPort_{"40001"};
};

template <template <typename, typename> typename Encryption>
WebSocketClientImpl<Encryption>::WebSocketClientImpl(AudioPlayer* player)
    : player_{player}
{
    wsClient_.setOnMessageHandler(std::bind(&WebSocketClientImpl<Encryption>::onMessage, this, std::placeholders::_1));
}

template <template <typename, typename> typename Encryption>
void WebSocketClientImpl<Encryption>::connect(const std::string& uri)
{
    if(connected_)
    {
        SAPLOG_INFO("Websocket already connected.disconnect first.");
        return;
    }

    const auto& address = ensureAddressHasPortNumber(removeProtocol(uri));

    SAPLOG_INFO("Trying to connect to: %s", address.c_str());
    wsClient_.connect(address, std::bind(&WebSocketClientImpl<Encryption>::onServiceConnection, this, std::placeholders::_1),
        std::bind(&WebSocketClientImpl<Encryption>::onServiceDisconnected, this));
}

template <template <typename, typename> typename Encryption>
void WebSocketClientImpl<Encryption>::disconnect()
{
    SAPLOG_INFO("Disconnecting.");
    wsClient_.disconnect();
}

template <template <typename, typename> typename Encryption>
void WebSocketClientImpl<Encryption>::onMessage(const std::string& msg)
{
    const void* in = static_cast<const void*>(msg.c_str());
    size_t len = msg.size();
    player_->push_data(in, len);
}

template <template <typename, typename> typename Encryption>
void WebSocketClientImpl<Encryption>::onServiceConnection(WebSockets::ConnectionInitializationResult result)
{
    if (!result)
    {
        SAPLOG_INFO("Websocket Connection Error.");
        connected_ = false;
        player_->wsConnectionStatus(WSStatus::NETWORKERROR);
    }
    else
    {
        SAPLOG_INFO("Websocket Connection Established.");
        connected_ = true;
        player_->wsConnectionStatus(WSStatus::CONNECTED);
    }
}

template <template <typename, typename> typename Encryption>
void WebSocketClientImpl<Encryption>::onServiceDisconnected()
{
    SAPLOG_INFO("Websocket Connection Closed.");
    connected_ = false;
    player_->wsConnectionStatus(WSStatus::DISCONNECTED);
}

template <template <typename, typename> typename Encryption>
std::string WebSocketClientImpl<Encryption>::removeProtocol(const std::string& uri) const
{
    const std::string protocolEndSign{"://"};
    const auto& protocolEnd = uri.find(protocolEndSign);
    if(protocolEnd != std::string::npos)
        return uri.substr(protocolEnd + protocolEndSign.size());
    else
        return uri;
}

template <template <typename, typename> typename Encryption>
std::string WebSocketClientImpl<Encryption>::ensureAddressHasPortNumber(std::string address) const
{
    const std::string portNumberBeginSign{":"};
    const auto& portNumberBegin = address.find_first_of(portNumberBeginSign);

    if(std::string::npos == portNumberBegin)
    {
        const std::string portNumberEndSign{"/"};
        const auto& portNumberEnd = address.find_first_of(portNumberEndSign);
        if (std::string::npos != portNumberEnd)
            address.insert(portNumberEnd, ":" + defaultAudioPort_);
        else
            address += ":" + defaultAudioPort_;
    }
    return address;
}

} // namespace impl
