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

#include "Module.h"
#include <interfaces/IWatermark.h>
#include <interfaces/json/JContentProtection.h>

namespace WPEFramework {
namespace Plugin {

    class ContentProtection
        : public PluginHost::IPlugin,
          public PluginHost::JSONRPC {
    private:
        struct Session {
            string ClientId;
            Exchange::IContentProtection::KeySystem KeySystem;
        };

    private:
        struct Watermark {
            uint32_t SessionId;
            bool AdjustVisibilityRequired;
            uint32_t GraphicImageBufferKey;
            uint32_t GraphicImageSize;
        };

    private:
        template <typename OBJECT>
        class Storage {
        public:
            void Set(uint32_t id, const OBJECT& item)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_lock);
                auto it = _items.find(id);
                if (it != _items.end()) {
                    it->second = item;
                } else {
                    _items.emplace(id, item);
                }
            }
            Core::OptionalType<OBJECT> Get(uint32_t id)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_lock);
                Core::OptionalType<OBJECT> result;
                auto it = _items.find(id);
                if (it != _items.end()) {
                    result = it->second;
                }
                return result;
            }
            void Delete(uint32_t id)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_lock);
                _items.erase(id);
            }

        private:
            Core::CriticalSection _lock;
            std::map<uint32_t, OBJECT> _items;
        };

    private:
        struct OnAddWatermarkParams : public Core::JSON::Container {
            OnAddWatermarkParams()
            {
                Add(_T("sessionId"), &SessionId);
                Add(_T("graphicId"), &GraphicId);
                Add(_T("adjustVisibilityRequired"),
                    &AdjustVisibilityRequired);
                Add(_T("zIndex"), &ZIndex);
                Add(_T("imageType"), &ImageType);
                Add(_T("graphicImageBufferKey"), &GraphicImageBufferKey);
                Add(_T("graphicImageSize"), &GraphicImageSize);
            }
            Core::JSON::DecUInt32 SessionId;
            Core::JSON::DecUInt32 GraphicId;
            Core::JSON::Boolean AdjustVisibilityRequired;
            Core::JSON::DecUInt32 ZIndex;
            Core::JSON::String ImageType;
            Core::JSON::DecUInt32 GraphicImageBufferKey;
            Core::JSON::DecUInt32 GraphicImageSize;
        };

        struct OnRemoveWatermarkParams : public Core::JSON::Container {
            OnRemoveWatermarkParams()
            {
                Add(_T("graphicId"), &GraphicId);
            }
            Core::JSON::DecUInt32 GraphicId;
        };

        struct OnDisplayWatermarkParams : public Core::JSON::Container {
            OnDisplayWatermarkParams()
            {
                Add(_T("hideWatermark"), &HideWatermark);
            }
            Core::JSON::Boolean HideWatermark;
        };

        struct OnWatermarkSessionParams : public Core::JSON::Container {
            OnWatermarkSessionParams()
            {
                Add(_T("sessionId"), &SessionId);
                Add(_T("conditionContext"), &ConditionContext);
            }
            Core::JSON::DecUInt32 SessionId;
            Core::JSON::DecSInt32 ConditionContext;
        };

        struct OnUpdateWatermarkParams : public Core::JSON::Container {
            OnUpdateWatermarkParams()
            {
                Add(_T("graphicId"), &GraphicId);
                Add(_T("watermarkClutBufferKey"), &WatermarkClutBufferKey);
                Add(_T("watermarkImageBufferKey"),
                    &WatermarkImageBufferKey);
            }
            Core::JSON::DecUInt32 GraphicId;
            Core::JSON::DecUInt32 WatermarkClutBufferKey;
            Core::JSON::DecUInt32 WatermarkImageBufferKey;
        };

    private:
        struct OnWatermarkRequestStatusParams : public Core::JSON::Container {
            OnWatermarkRequestStatusParams()
            {
                Add(_T("id"), &Id);
                Add(_T("type"), &Type);
                Add(_T("success"), &Success);
            }
            Core::JSON::DecUInt32 Id;
            Core::JSON::String Type;
            Core::JSON::Boolean Success;
        };

        struct OnWatermarkRenderFailedParams : public Core::JSON::Container {
            OnWatermarkRenderFailedParams()
            {
                Add(_T("image"), &Image);
            }
            Core::JSON::DecUInt32 Image;
        };

    private:
        struct SetPlaybackSessionStateParams : public Core::JSON::Container {
            SetPlaybackSessionStateParams()
            {
                Add(_T("clientId"), &ClientId);
                Add(_T("sessionid"), &SessionId);
                Add(_T("sessionstate"), &SessionState);
            }
            Core::JSON::String ClientId;
            Core::JSON::DecUInt32 SessionId;
            Core::JSON::EnumType<
                Exchange::IContentProtection::State>
                SessionState;
        };

        struct ClosePlaybackSessionParams : public Core::JSON::Container {
            ClosePlaybackSessionParams()
            {
                Add(_T("clientId"), &ClientId);
                Add(_T("sessionid"), &SessionId);
            }
            Core::JSON::String ClientId;
            Core::JSON::DecUInt32 SessionId;
        };

        struct SetPlaybackSpeedStateParams : public Core::JSON::Container {
            SetPlaybackSpeedStateParams()
            {
                Add(_T("sessionid"), &SessionId);
                Add(_T("playbackSpeed"), &PlaybackSpeed);
                Add(_T("playbackPosition"), &PlaybackPosition);
            }
            Core::JSON::DecUInt32 SessionId;
            Core::JSON::DecSInt32 PlaybackSpeed;
            Core::JSON::DecSInt32 PlaybackPosition;
        };

        struct LoadClutWatermarkParams : public Core::JSON::Container {
            LoadClutWatermarkParams()
            {
                Add(_T("sessionId"), &SessionId);
                Add(_T("graphicId"), &GraphicId);
                Add(_T("watermarkClutBufferKey"), &WatermarkClutBufferKey);
                Add(_T("watermarkImageBufferKey"), &WatermarkImageBufferKey);
                Add(_T("clutPaletteSize"), &ClutPaletteSize);
                Add(_T("clutPaletteFormat"), &ClutPaletteFormat);
                Add(_T("watermarkWidth"), &WatermarkWidth);
                Add(_T("watermarkHeight"), &WatermarkHeight);
                Add(_T("aspectRatio"), &AspectRatio);
            }
            Core::JSON::DecUInt32 SessionId;
            Core::JSON::DecUInt32 GraphicId;
            Core::JSON::DecUInt32 WatermarkClutBufferKey;
            Core::JSON::DecUInt32 WatermarkImageBufferKey;
            Core::JSON::DecUInt32 ClutPaletteSize;
            Core::JSON::String ClutPaletteFormat;
            Core::JSON::DecUInt32 WatermarkWidth;
            Core::JSON::DecUInt32 WatermarkHeight;
            Core::JSON::Float AspectRatio;
        };

    private:
        struct ShowWatermarkParams : public Core::JSON::Container {
            ShowWatermarkParams()
            {
                Add(_T("show"), &Show);
            }
            Core::JSON::Boolean Show;
        };

        struct CreateWatermarkParams : public Core::JSON::Container {
            CreateWatermarkParams()
            {
                Add(_T("id"), &Id);
                Add(_T("zorder"), &Zorder);
            }
            Core::JSON::DecUInt32 Id;
            Core::JSON::DecUInt32 Zorder;
        };

        struct DeleteWatermarkParams : public Core::JSON::Container {
            DeleteWatermarkParams()
            {
                Add(_T("id"), &Id);
            }
            Core::JSON::DecUInt32 Id;
        };

        struct PaletteWatermarkParams : public Core::JSON::Container {
            PaletteWatermarkParams()
            {
                Add(_T("id"), &Id);
                Add(_T("imageKey"), &ImageKey);
                Add(_T("imageWidth"), &ImageWidth);
                Add(_T("imageHeight"), &ImageHeight);
                Add(_T("clutKey"), &ClutKey);
                Add(_T("clutSize"), &ClutSize);
                Add(_T("clutType"), &ClutType);
            }
            Core::JSON::DecUInt32 Id;
            Core::JSON::DecUInt32 ImageKey;
            Core::JSON::DecUInt32 ImageWidth;
            Core::JSON::DecUInt32 ImageHeight;
            Core::JSON::DecUInt32 ClutKey;
            Core::JSON::DecUInt32 ClutSize;
            Core::JSON::String ClutType;
        };

        struct UpdateWatermarkParams : public Core::JSON::Container {
            UpdateWatermarkParams()
            {
                Add(_T("id"), &Id);
                Add(_T("key"), &Key);
                Add(_T("size"), &Size);
            }
            Core::JSON::DecUInt32 Id;
            Core::JSON::DecUInt32 Key;
            Core::JSON::DecUInt32 Size;
        };

    private:
        enum { Timeout = 1000 };

    private:
        using JSONRPCLink = WPEFramework::JSONRPC::SmartLinkType<
            Core::JSON::IElement>;
        using State
            = Exchange::IContentProtection::INotification::Status::State;

    private:
        class Implementation : public Exchange::IContentProtection {
        public:
            Implementation(const Implementation&) = delete;
            Implementation(Implementation&&) = delete;
            Implementation& operator=(const Implementation&) = delete;
            Implementation& operator=(Implementation&&) = delete;

            explicit Implementation(ContentProtection& parent)
                : _parent(parent)
            {
            }

            BEGIN_INTERFACE_MAP(Implementation)
            INTERFACE_ENTRY(IContentProtection)
            END_INTERFACE_MAP

        private:
            uint32_t Register(
                IContentProtection::INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(
                    _parent._clientLock);
                ASSERT(std::find(
                           _parent._clients.begin(), _parent._clients.end(),
                           notification)
                    == _parent._clients.end());
                notification->AddRef();
                _parent._clients.push_back(notification);
                return Core::ERROR_NONE;
            }

            uint32_t Unregister(
                IContentProtection::INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(
                    _parent._clientLock);
                auto index = std::find(
                    _parent._clients.begin(), _parent._clients.end(),
                    notification);
                ASSERT(index != _parent._clients.end());
                if (index != _parent._clients.end()) {
                    notification->Release();
                    _parent._clients.erase(index);
                }
                return Core::ERROR_NONE;
            }

            uint32_t OpenDrmSession(const string& clientId,
                KeySystem keySystem, const string& licenseRequest,
                const string& initData, uint32_t& sessionId, string& response)
                override
            {
                uint32_t result;
                JsonObject out;
                out.FromString(initData);
                out["clientId"] = clientId;
                out["keySystem"] = Core::JSON::EnumType<KeySystem>(keySystem)
                                       .Data();
                out["licenseRequest"] = licenseRequest;
                JsonObject in;
                result = _parent._secManager->Invoke<JsonObject, JsonObject>(
                    Timeout, _T("openPlaybackSession"), out, in);
                if (result == Core::ERROR_NONE) {
                    if (!in["success"].Boolean()) {
                        result = Core::ERROR_GENERAL;
                    } else {
                        sessionId = in["sessionId"].Number();
                        in.ToString(response);

                        _parent._sessionStorage.Set(sessionId,
                            { clientId, keySystem });
                    }
                }
                return result;
            }

            uint32_t SetDrmSessionState(uint32_t sessionId,
                State sessionState) override
            {
                auto session = _parent._sessionStorage.Get(sessionId);
                if (!session.IsSet()) {
                    return Core::ERROR_ILLEGAL_STATE; // No such session
                }

                uint32_t result;
                SetPlaybackSessionStateParams out;
                out.ClientId = session.Value().ClientId;
                out.SessionId = sessionId;
                out.SessionState = sessionState;
                JsonObject in;
                result = _parent._secManager->Invoke<
                    SetPlaybackSessionStateParams, JsonObject>(
                    Timeout, _T("setPlaybackSessionState"), out, in);
                if (result == Core::ERROR_NONE) {
                    if (!in["success"].Boolean()) {
                        result = Core::ERROR_GENERAL;
                    }
                }
                return result;
            }

            uint32_t UpdateDrmSession(uint32_t sessionId,
                const string& licenseRequest,
                const string& initData, string& response) override
            {
                auto session = _parent._sessionStorage.Get(sessionId);
                if (!session.IsSet()) {
                    return Core::ERROR_ILLEGAL_STATE; // No such session
                }

                uint32_t result;
                JsonObject out;
                out.FromString(initData);
                out["clientId"] = session.Value().ClientId;
                out["sessionId"] = sessionId;
                out["keySystem"] = Core::JSON::EnumType<KeySystem>(
                    session.Value().KeySystem)
                                       .Data();
                out["licenseRequest"] = licenseRequest;
                JsonObject in;
                result = _parent._secManager->Invoke<
                    JsonObject, JsonObject>(
                    Timeout, _T("updatePlaybackSession"), out, in);
                if (result == Core::ERROR_NONE) {
                    if (!in["success"].Boolean()) {
                        result = Core::ERROR_GENERAL;
                    } else {
                        in.ToString(response);
                    }
                }
                return result;
            }

            uint32_t CloseDrmSession(uint32_t sessionId) override
            {
                auto session = _parent._sessionStorage.Get(sessionId);
                if (!session.IsSet()) {
                    return Core::ERROR_ILLEGAL_STATE; // No such session
                }

                uint32_t result;
                ClosePlaybackSessionParams out;
                out.ClientId = session.Value().ClientId;
                out.SessionId = sessionId;
                JsonObject in;
                result = _parent._secManager->Invoke<
                    ClosePlaybackSessionParams, JsonObject>(
                    Timeout, _T("closePlaybackSession"), out, in);
                if ((result == Core::ERROR_NONE) && !in["success"].Boolean()) {
                    result = Core::ERROR_GENERAL;
                }
                return result;
            }

            uint32_t ShowWatermark(uint32_t /*sessionId*/,
                bool show, bool /*localOverlay*/) override
            {
                uint32_t result;
                ShowWatermarkParams out;
                out.Show = show;
                JsonObject in;
                result = _parent._watermark->Invoke<
                    ShowWatermarkParams, JsonObject>(
                    Timeout, _T("showWatermark"), out, in);
                if ((result == Core::ERROR_NONE) && !in["success"].Boolean()) {
                    result = Core::ERROR_GENERAL;
                }
                return result;
            }

            uint32_t SetPlaybackPosition(uint32_t sessionId,
                int32_t speed, long position) override
            {
                uint32_t result;
                SetPlaybackSpeedStateParams out;
                out.SessionId = sessionId;
                out.PlaybackSpeed = speed;
                out.PlaybackPosition = position;
                JsonObject in;
                result = _parent._secManager->Invoke<
                    SetPlaybackSpeedStateParams, JsonObject>(
                    Timeout, _T("setPlaybackSpeedState"), out, in);
                if ((result == Core::ERROR_NONE) && !in["success"].Boolean()) {
                    result = Core::ERROR_GENERAL;
                }
                return result;
            }

        private:
            ContentProtection& _parent;
        };

    public:
        ContentProtection(const ContentProtection&) = delete;
        ContentProtection(ContentProtection&&) = delete;
        ContentProtection& operator=(const ContentProtection&) = delete;
        ContentProtection& operator=(ContentProtection&&) = delete;
        ContentProtection() = default;

        BEGIN_INTERFACE_MAP(ContentProtection)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IContentProtection, _implementation)
        END_INTERFACE_MAP

    private:
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

    private:
        void Subscribe()
        {
            ASSERT(_secManager->Subscribe<OnAddWatermarkParams>(
                       Timeout, _T("onAddWatermark"),
                       [&](const OnAddWatermarkParams& params) {
                           _watermarkStorage.Set(
                               params.GraphicId,
                               { params.SessionId,
                                   params.AdjustVisibilityRequired,
                                   params.GraphicImageBufferKey,
                                   params.GraphicImageSize });

                           CreateWatermarkParams out;
                           out.Id = params.GraphicId;
                           out.Zorder = params.ZIndex;
                           JsonObject in;
                           if ((_watermark->Invoke<
                                    CreateWatermarkParams, JsonObject>(
                                    Timeout, _T("createWatermark"), out, in)
                                   != Core::ERROR_NONE)
                               || !in["success"].Boolean()) {
                               TRACE(Trace::Error,
                                   (_T("create %d failed"), params.GraphicId));
                           }
                       })
                == Core::ERROR_NONE);
            ASSERT(_secManager->Subscribe<OnRemoveWatermarkParams>(
                       Timeout, _T("onRemoveWatermark"),
                       [&](const OnRemoveWatermarkParams& params) {
                           DeleteWatermarkParams out;
                           out.Id = params.GraphicId;
                           JsonObject in;
                           if ((_watermark->Invoke<
                                    DeleteWatermarkParams, JsonObject>(
                                    Timeout, _T("deleteWatermark"), out, in)
                                   != Core::ERROR_NONE)
                               || !in["success"].Boolean()) {
                               TRACE(Trace::Error,
                                   (_T("delete %d failed"), params.GraphicId));
                           }
                       })
                == Core::ERROR_NONE);
            ASSERT(_secManager->Subscribe<OnDisplayWatermarkParams>(
                       Timeout, _T("onDisplayWatermark"),
                       [&](const OnDisplayWatermarkParams& params) {
                           uint32_t result;
                           ShowWatermarkParams out;
                           out.Show = !params.HideWatermark;
                           JsonObject in;
                           result = _watermark->Invoke<
                               ShowWatermarkParams, JsonObject>(
                               Timeout, _T("showWatermark"), out, in);
                           if ((result != Core::ERROR_NONE)
                               || !in["success"].Boolean()) {
                               TRACE(Trace::Error, (_T("show failed")));
                           }
                       })
                == Core::ERROR_NONE);
            ASSERT(_secManager->Subscribe<OnWatermarkSessionParams>(
                       Timeout, _T("onWatermarkSession"),
                       [&](const OnWatermarkSessionParams& params) {
                           WatermarkStatusChanged(
                               params.SessionId,
                               { ((params.ConditionContext == 1)
                                         ? State::GRANTED
                                         : ((params.ConditionContext == 2)
                                                   ? State::NOT_REQUIRED
                                                   : ((params.ConditionContext == 3)
                                                             ? State::DENIED
                                                             : State::FAILED))),
                                   params.ConditionContext });
                       })
                == Core::ERROR_NONE);
            ASSERT(_secManager->Subscribe<OnUpdateWatermarkParams>(
                       Timeout, _T("onUpdateWatermark"),
                       [&](const OnUpdateWatermarkParams& params) {
                           auto palette = _palettedImageDataStorage.Get(
                               params.GraphicId);
                           if (!palette.IsSet()) {
                               TRACE(Trace::Error,
                                   (_T("no palette %d"), params.GraphicId));
                           } else {
                               PaletteWatermarkParams out;
                               out.Id = params.GraphicId;
                               out.ImageKey = params.WatermarkImageBufferKey;
                               out.ImageWidth = palette.Value().imageWidth;
                               out.ImageHeight = palette.Value().imageHeight;
                               out.ClutKey = params.WatermarkClutBufferKey;
                               out.ClutSize = palette.Value().clutSize;
                               out.ClutType = palette.Value().clutType;
                               JsonObject in;
                               if ((_watermark->Invoke<
                                        PaletteWatermarkParams, JsonObject>(
                                        Timeout, _T("modifyPalettedWatermark"),
                                        out, in)
                                       != Core::ERROR_NONE)
                                   || !in["success"].Boolean()) {
                                   TRACE(Trace::Error,
                                       (_T("modify %d failed"),
                                           params.GraphicId));
                               }
                           }
                       })
                == Core::ERROR_NONE);
            ASSERT(_watermark->Subscribe<OnWatermarkRequestStatusParams>(
                       Timeout, _T("onWatermarkRequestStatus"),
                       [&](const OnWatermarkRequestStatusParams& params) {
                           if (params.Type == "show") {
                               if (!params.Success) {
                                   TRACE(Trace::Error, (_T("show failed")));
                               }
                               // Watermark plugin does not tell
                               // which call ended, can be any. Can't take this
                               // information as a response
                           } else if (!params.Success) {
                               auto watermark = _watermarkStorage
                                                    .Get(params.Id);
                               if (watermark.IsSet()) {
                                   TRACE(Trace::Error,
                                       (_T("%s %d failed"),
                                           params.Type.Value().c_str(),
                                           params.Id));
                               }
                           } else if (params.Type == "create") {
                               auto watermark = _watermarkStorage
                                                    .Get(params.Id);
                               if (watermark.IsSet()) {
                                   UpdateWatermarkParams out;
                                   out.Id = params.Id;
                                   out.Key = watermark.Value()
                                                 .GraphicImageBufferKey;
                                   out.Size = watermark.Value()
                                                  .GraphicImageSize;
                                   JsonObject in;
                                   if ((_watermark->Invoke<
                                            UpdateWatermarkParams, JsonObject>(
                                            Timeout, _T("updateWatermark"),
                                            out, in)
                                           != Core::ERROR_NONE)
                                       || !in["success"].Boolean()) {
                                       TRACE(Trace::Error,
                                           (_T("update %d failed"),
                                               params.Id));
                                   }
                               }
                           } else if (params.Type == "update") {
                               auto watermark = _watermarkStorage
                                                    .Get(params.Id);
                               if (watermark.IsSet()
                                   && watermark.Value()
                                       .AdjustVisibilityRequired) {
                                   DeleteWatermarkParams out;
                                   out.Id = params.Id;
                                   PaletteWatermarkParams in;
                                   if ((_watermark->Invoke<
                                            DeleteWatermarkParams,
                                            PaletteWatermarkParams>(
                                            Timeout,
                                            _T("getPalettedWatermark"),
                                            out, in)
                                           != Core::ERROR_NONE)
                                       || !in.ImageWidth || !in.ImageHeight) {
                                       TRACE(Trace::Error,
                                           (_T("get %d failed"), params.Id));
                                   } else {
                                       _palettedImageDataStorage.Set(
                                           params.Id,
                                           { in.ImageKey,
                                               in.ImageWidth,
                                               in.ImageHeight,
                                               in.ClutKey,
                                               in.ClutSize,
                                               in.ClutType });

                                       LoadClutWatermarkParams out;
                                       out.SessionId = watermark.Value()
                                                           .SessionId;
                                       out.GraphicId = params.Id;
                                       out.WatermarkClutBufferKey = in.ClutKey;
                                       out.WatermarkImageBufferKey
                                           = in.ImageKey;
                                       out.ClutPaletteSize = in.ClutSize;
                                       out.ClutPaletteFormat = in.ClutType;
                                       out.WatermarkWidth = in.ImageWidth;
                                       out.WatermarkHeight = in.ImageHeight;
                                       out.AspectRatio
                                           = ((float)in.ImageWidth
                                               / (float)in.ImageHeight);
                                       JsonObject in2;
                                       if ((_secManager->Invoke<
                                                LoadClutWatermarkParams,
                                                JsonObject>(
                                                Timeout,
                                                _T("loadClutWatermark"),
                                                out, in2)
                                               != Core::ERROR_NONE)
                                           || !in2["success"].Boolean()) {
                                           TRACE(Trace::Error,
                                               (_T("load %d failed"),
                                                   params.Id));
                                       }
                                   }
                               }
                           }
                       })
                == Core::ERROR_NONE);
            ASSERT(_watermark->Subscribe<OnWatermarkRenderFailedParams>(
                       Timeout, _T("onWatermarkRenderFailed"),
                       [&](const OnWatermarkRenderFailedParams& params) {
                           auto watermark = _watermarkStorage
                                                .Get(params.Image);
                           if (watermark.IsSet()) {
                               WatermarkStatusChanged(
                                   watermark.Value().SessionId,
                                   { State::FAILED, 20001 });
                           }
                       })
                == Core::ERROR_NONE);
        }
        void Unsubscribe()
        {
            _secManager->Unsubscribe(Timeout, _T("onAddWatermark"));
            _secManager->Unsubscribe(Timeout, _T("onRemoveWatermark"));
            _secManager->Unsubscribe(Timeout, _T("onDisplayWatermark"));
            _secManager->Unsubscribe(Timeout, _T("onWatermarkSession"));
            _secManager->Unsubscribe(Timeout, _T("onUpdateWatermark"));
            _watermark->Unsubscribe(Timeout, _T("onWatermarkRequestStatus"));
            _watermark->Unsubscribe(Timeout, _T("onWatermarkRenderFailed"));
        }
        void WatermarkStatusChanged(uint32_t sessionId,
            const Exchange::IContentProtection::INotification::Status& status)
        {
            Exchange::JContentProtection::Event::WatermarkStatusChanged(
                *this, sessionId, status);

            Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);
            for (auto& i : _clients) {
                i->WatermarkStatusChanged(sessionId, status);
            }
        }

    private:
        Exchange::IContentProtection* _implementation;
        Core::ProxyType<JSONRPCLink> _secManager;
        Core::ProxyType<JSONRPCLink> _watermark;
        Storage<Session> _sessionStorage;
        Storage<Watermark> _watermarkStorage;
        Storage<Exchange::PalettedImageData> _palettedImageDataStorage;
        std::list<Exchange::IContentProtection::INotification*> _clients;
        Core::CriticalSection _clientLock;
    };

} // namespace Plugin
} // namespace WPEFramework
