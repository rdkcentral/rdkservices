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

#include "../Module.h"

namespace WPEFramework {
namespace Plugin {
    namespace JsonRpc {

        // This class is needed because SecManager plugin has no c++ interface

        class SecManager : public Core::IReferenceCounted {
        private:
            SecManager(const SecManager&) = delete;
            SecManager& operator=(const SecManager&) = delete;

        public:
            explicit SecManager(PluginHost::IDispatcher* dispatcher,
                const string& token)
                : _dispatcher(dispatcher)
                , _token(token)
                , _events({ "onAddWatermark",
                      "onRemoveWatermark",
                      "onDisplayWatermark",
                      "onWatermarkSession",
                      "onUpdateWatermark" })
            {
                ASSERT(_dispatcher != nullptr);
                _dispatcher->AddRef();
            }
            ~SecManager()
            {
                _dispatcher->Release();
            }

        public:
            uint32_t Register(PluginHost::IDispatcher::ICallback* callback)
            {
                uint32_t result;
                for (auto& event : _events) {
                    result = _dispatcher->Subscribe(callback, event, "");
                    if (result != Core::ERROR_NONE) {
                        break;
                    }
                }
                return result;
            }

            uint32_t Unregister(PluginHost::IDispatcher::ICallback* callback)
            {
                uint32_t result;
                for (auto& event : _events) {
                    result = _dispatcher->Unsubscribe(callback, event, "");
                    if (result != Core::ERROR_NONE) {
                        break;
                    }
                }
                return result;
            }

            uint32_t OpenPlaybackSession(const string& clientId,
                const string& keySystem, const string& licenseRequest,
                const string& initData, uint32_t& sessionId, string& response)
            {
                uint32_t result;
                JsonObject params;
                params.FromString(initData);
                params["clientId"] = clientId;
                params["keySystem"] = keySystem;
                params["licenseRequest"] = licenseRequest;
                string paramsStr;
                params.ToString(paramsStr);
                result = _dispatcher->Invoke(
                    0, 0, _token, "openPlaybackSession", paramsStr, response);
                if (result == Core::ERROR_NONE) {
                    JsonObject json;
                    json.FromString(response);
                    if (!json["success"].Boolean()) {
                        result = Core::ERROR_GENERAL;
                    } else {
                        sessionId = json["sessionId"].Number();
                    }
                }
                return result;
            }

            uint32_t SetPlaybackSessionState(const string& clientId,
                uint32_t sessionId, const string& sessionState)
            {
                uint32_t result;
                JsonObject params;
                params["clientId"] = clientId;
                params["sessionId"] = sessionId;
                params["sessionState"] = sessionState;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                result = _dispatcher->Invoke(
                    0, 0, _token, "setPlaybackSessionState",
                    paramsStr, responseStr);
                if (result == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    if (!response["success"].Boolean()) {
                        result = Core::ERROR_GENERAL;
                    }
                }
                return result;
            }

            uint32_t UpdatePlaybackSession(const string& clientId,
                const string& keySystem, const string& licenseRequest,
                const string& initData, uint32_t sessionId, string& response)
            {
                uint32_t result;
                JsonObject params;
                params.FromString(initData);
                params["clientId"] = clientId;
                params["sessionId"] = sessionId;
                params["keySystem"] = keySystem;
                params["licenseRequest"] = licenseRequest;
                string paramsStr;
                params.ToString(paramsStr);
                result = _dispatcher->Invoke(
                    0, 0, _token, "updatePlaybackSession",
                    paramsStr, response);
                if (result == Core::ERROR_NONE) {
                    JsonObject json;
                    json.FromString(response);
                    if (!json["success"].Boolean()) {
                        result = Core::ERROR_GENERAL;
                    }
                }
                return result;
            }

            uint32_t ClosePlaybackSession(const string& clientId,
                uint32_t sessionId)
            {
                uint32_t result;
                JsonObject params;
                params["clientId"] = clientId;
                params["sessionId"] = sessionId;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                result = _dispatcher->Invoke(
                    0, 0, _token, "closePlaybackSession",
                    paramsStr, responseStr);
                if (result == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    if (!response["success"].Boolean()) {
                        result = Core::ERROR_GENERAL;
                    }
                }
                return result;
            }

            uint32_t SetPlaybackSpeedState(uint32_t sessionId,
                int32_t playbackSpeed, long playbackPosition)
            {
                uint32_t result;
                JsonObject params;
                params["sessionId"] = sessionId;
                params["playbackSpeed"] = playbackSpeed;
                params["playbackPosition"] = playbackPosition;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                result = _dispatcher->Invoke(
                    0, 0, _token, "setPlaybackSpeedState",
                    paramsStr, responseStr);
                if (result == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    if (!response["success"].Boolean()) {
                        result = Core::ERROR_GENERAL;
                    }
                }
                return result;
            }

            uint32_t LoadClutWatermark(
                uint32_t sessionId, uint32_t graphicId,
                uint32_t watermarkClutBufferKey,
                uint32_t watermarkImageBufferKey,
                uint32_t clutPaletteSize, const string& clutPaletteFormat,
                uint32_t watermarkWidth, uint32_t watermarkHeight,
                float aspectRatio)
            {
                uint32_t result;
                JsonObject params;
                params["sessionId"] = sessionId;
                params["graphicId"] = graphicId;
                params["watermarkClutBufferKey"] = watermarkClutBufferKey;
                params["watermarkImageBufferKey"] = watermarkImageBufferKey;
                params["clutPaletteSize"] = clutPaletteSize;
                params["clutPaletteFormat"] = clutPaletteFormat;
                params["watermarkWidth"] = watermarkWidth;
                params["watermarkHeight"] = watermarkHeight;
                params["aspectRatio"] = aspectRatio;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                result = _dispatcher->Invoke(
                    0, 0, _token, "setPlaybackSpeedState",
                    paramsStr, responseStr);
                if (result == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    if (!response["success"].Boolean()) {
                        result = Core::ERROR_GENERAL;
                    }
                }
                return result;
            }

        private:
            PluginHost::IDispatcher* _dispatcher;
            const string _token;
            const std::list<string> _events;
        };

    } // namespace JsonRpc
} // namespace Plugin
} // namespace WPEFramework
