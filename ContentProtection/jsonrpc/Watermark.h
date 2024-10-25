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
#include <interfaces/IWatermark.h>

namespace WPEFramework {
namespace Plugin {
    namespace JsonRpc {

        // This class is needed because Watermark plugin has no c++ interface

        class Watermark : public Exchange::IWatermark {
        private:
            class Callback : public PluginHost::IDispatcher::ICallback {
            private:
                Callback(const Callback&) = delete;
                Callback& operator=(const Callback&) = delete;

            public:
                explicit Callback(Watermark& parent)
                    : _parent(parent)
                    , _events({ "onWatermarkRequestStatus",
                          "onWatermarkRenderFailed" })
                {
                    ASSERT(_parent._dispatcher != nullptr);
                    _parent._dispatcher->AddRef();
                    for (auto& event : _events) {
                        ASSERT(_parent._dispatcher->Subscribe(
                                   this, event, "")
                            == Core::ERROR_NONE);
                    }
                }
                ~Callback() override
                {
                    for (auto& event : _events) {
                        ASSERT(_parent._dispatcher->Unsubscribe(
                                   this, event, "")
                            == Core::ERROR_NONE);
                    }
                    _parent._dispatcher->Release();
                }

                BEGIN_INTERFACE_MAP(Callback)
                INTERFACE_ENTRY(PluginHost::IDispatcher::ICallback)
                END_INTERFACE_MAP

            private:
                Core::hresult Event(const string& event,
                    const string& /*designator*/,
                    const string& parameters) override
                {
                    ASSERT(std::find(_events.begin(), _events.end(), event)
                        != _events.end());

                    _parent.WatermarkEvent(event, parameters);

                    return Core::ERROR_NONE;
                }

            private:
                Watermark& _parent;
                const std::list<string> _events;
            };

        private:
            Watermark(const Watermark&) = delete;
            Watermark& operator=(const Watermark&) = delete;

        public:
            explicit Watermark(PluginHost::IDispatcher* dispatcher,
                const string& token)
                : _dispatcher(dispatcher)
                , _token(token)
                , _callback(*this)
            {
                ASSERT(_dispatcher != nullptr);
                _dispatcher->AddRef();
            }
            ~Watermark() override
            {
                _dispatcher->Release();
            }

            BEGIN_INTERFACE_MAP(Watermark)
            INTERFACE_ENTRY(Exchange::IWatermark)
            END_INTERFACE_MAP

        private:
            uint32_t Initialize(string /*waylandDisplay*/,
                bool /*synchronized*/) override
            {
                return Core::ERROR_NOT_SUPPORTED;
            }

            uint32_t Deinitialize() override
            {
                return Core::ERROR_NOT_SUPPORTED;
            }

            uint32_t Register(INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);
                ASSERT(std::find(_clients.begin(), _clients.end(),
                           notification)
                    == _clients.end());
                notification->AddRef();
                _clients.push_back(notification);

                return Core::ERROR_NONE;
            }

            uint32_t Unregister(INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);
                auto index = std::find(_clients.begin(), _clients.end(),
                    notification);
                ASSERT(index != _clients.end());
                if (index != _clients.end()) {
                    notification->Release();
                    _clients.erase(index);
                }

                return Core::ERROR_NONE;
            }

            bool ShowWatermark(const bool show) override
            {
                bool result;
                JsonObject params;
                params["show"] = show;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                auto ret = _dispatcher->Invoke(0, 0, _token, "showWatermark",
                    paramsStr, responseStr);
                if (ret == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    result = response["success"].Boolean();
                } else {
                    result = false;
                }
                return result;
            }

            bool CreateWatermark(uint32_t id, uint32_t zorder) override
            {
                bool result;
                JsonObject params;
                params["id"] = id;
                params["zorder"] = zorder;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                auto ret = _dispatcher->Invoke(0, 0, _token, "createWatermark",
                    paramsStr, responseStr);
                if (ret == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    result = response["success"].Boolean();
                } else {
                    result = false;
                }
                return result;
            }

            bool UpdateWatermark(uint32_t id,
                uint32_t key, uint32_t size) override
            {
                bool result;
                JsonObject params;
                params["id"] = id;
                params["key"] = key;
                params["size"] = size;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                auto ret = _dispatcher->Invoke(0, 0, _token, "updateWatermark",
                    paramsStr, responseStr);
                if (ret == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    result = response["success"].Boolean();
                } else {
                    result = false;
                }
                return result;
            }

            bool AdjustWatermark(uint32_t /*id*/, uint32_t /*zorder*/) override
            {
                return false;
            }

            bool DeleteWatermark(uint32_t id) override
            {
                bool result;
                JsonObject params;
                params["id"] = id;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                auto ret = _dispatcher->Invoke(0, 0, _token, "deleteWatermark",
                    paramsStr, responseStr);
                if (ret == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    result = response["success"].Boolean();
                } else {
                    result = false;
                }
                return result;
            }

            Exchange::PalettedImageData GetPalettedWatermark(
                uint32_t id) override
            {
                Exchange::PalettedImageData result{ 0, 0, 0 };
                JsonObject params;
                params["id"] = id;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                auto ret = _dispatcher->Invoke(
                    0, 0, _token, "getPalettedWatermark",
                    paramsStr, responseStr);
                if (ret == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    if (response["success"].Boolean()) {
                        result.imageKey = response["imageKey"].Number();
                        result.imageWidth = response["imageWidth"].Number();
                        result.imageHeight = response["imageHeight"].Number();
                        result.clutKey = response["clutKey"].Number();
                        result.clutSize = response["clutSize"].Number();
                        result.clutType = response["clutType"].String();
                    }
                }
                return result;
            }

            bool ModifyPalettedWatermark(uint32_t id,
                Exchange::PalettedImageData data) override
            {
                bool result;
                JsonObject params;
                params["id"] = id;
                params["imageKey"] = data.imageKey;
                params["imageWidth"] = data.imageWidth;
                params["imageHeight"] = data.imageHeight;
                params["clutKey"] = data.clutKey;
                params["clutSize"] = data.clutSize;
                string paramsStr, responseStr;
                params.ToString(paramsStr);
                auto ret = _dispatcher->Invoke(
                    0, 0, _token, "modifyPalettedWatermark",
                    paramsStr, responseStr);
                if (ret == Core::ERROR_NONE) {
                    JsonObject response;
                    response.FromString(responseStr);
                    result = response["success"].Boolean();
                } else {
                    result = false;
                }
                return result;
            }

        private:
            void WatermarkEvent(const string& eventName,
                const string& parametersJson)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);
                for (auto& i : _clients) {
                    i->WatermarkEvent(eventName, parametersJson);
                }
            }

        private:
            PluginHost::IDispatcher* _dispatcher;
            const string _token;
            Core::Sink<Callback> _callback;
            std::list<INotification*> _clients;
            Core::CriticalSection _clientLock;
        };

    } // namespace JsonRpc
} // namespace Plugin
} // namespace WPEFramework
