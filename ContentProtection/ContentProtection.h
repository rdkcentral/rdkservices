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
#include "jsonrpc/SecManager.h"
#include "jsonrpc/SecManagerEventParams.h"
#include "jsonrpc/WatermarkEventParams.h"
#include <interfaces/IWatermark.h>
#include <interfaces/json/JContentProtection.h>

namespace WPEFramework {
namespace Plugin {

    class ContentProtection
        : public PluginHost::IPlugin,
          public PluginHost::JSONRPC {
    private:
        class Implementation : public Exchange::IContentProtection {
        private:
            struct Session {
                uint32_t SessionId;
                string ClientId;
                IContentProtection::KeySystem KeySystem;
            };

        private:
            struct Watermark {
                uint32_t GraphicId;
                uint32_t SessionId;
                bool AdjustVisibilityRequired;
                uint32_t ZIndex;
                string ImageType;
                uint32_t GraphicImageBufferKey;
                uint32_t GraphicImageSize;
            };

        private:
            class Callback : public PluginHost::IDispatcher::ICallback {
            private:
                enum ConditionContext : uint8_t {
                    NO_ISSUES = 0,
                    SESSION_GRANTED = 1,
                    SESSION_NOT_REQUIRED = 2,
                    SESSION_DENIED = 3
                };

            private:
                Callback(const Callback&) = delete;
                Callback& operator=(const Callback&) = delete;

            public:
                explicit Callback(Implementation& parent)
                    : _parent(parent)
                {
                }
                ~Callback() override = default;

                BEGIN_INTERFACE_MAP(Callback)
                INTERFACE_ENTRY(PluginHost::IDispatcher::ICallback)
                END_INTERFACE_MAP

            private:
                Core::hresult Event(const string& event,
                    const string& /*designator*/,
                    const string& parameters) override
                {
                    if (event == "onAddWatermark") {
                        JsonRpc::OnAddWatermarkParams params;
                        params.FromString(parameters);
                        OnAddWatermark(params);
                    } else if (event == "onRemoveWatermark") {
                        JsonRpc::OnRemoveWatermarkParams params;
                        params.FromString(parameters);
                        OnRemoveWatermark(params);
                    } else if (event == "onDisplayWatermark") {
                        JsonRpc::OnDisplayWatermarkParams params;
                        params.FromString(parameters);
                        OnDisplayWatermark(params);
                    } else if (event == "onWatermarkSession") {
                        JsonRpc::OnWatermarkSessionParams params;
                        params.FromString(parameters);
                        OnWatermarkSession(params);
                    } else if (event == "onUpdateWatermark") {
                        JsonRpc::OnUpdateWatermarkParams params;
                        params.FromString(parameters);
                        OnUpdateWatermark(params);
                    }

                    return Core::ERROR_NONE;
                }

            private:
                void OnAddWatermark(
                    const JsonRpc::OnAddWatermarkParams& params)
                {
                    _parent.CreateWatermark({ params.GraphicId,
                        params.SessionId,
                        params.AdjustVisibilityRequired,
                        params.ZIndex,
                        params.ImageType,
                        params.GraphicImageBufferKey,
                        params.GraphicImageSize });
                    OnAddWatermark(params.GraphicId, params.ZIndex);
                }

                void OnAddWatermark(uint32_t graphicId, uint32_t zIndex)
                {
                    auto result = _parent.CreateWatermarkInternal(
                        graphicId, zIndex);
                    if (result == false) {
                        TRACE(Trace::Error,
                            (_T("create watermark %d failed"), graphicId));
                    }
                }

                void OnRemoveWatermark(
                    const JsonRpc::OnRemoveWatermarkParams& params)
                {
                    auto result = _parent.DeleteWatermarkInternal(
                        params.GraphicId);
                    if (result == false) {
                        TRACE(Trace::Error,
                            (_T("delete watermark %d failed"),
                                params.GraphicId));
                    }
                }

                void OnDisplayWatermark(
                    const JsonRpc::OnDisplayWatermarkParams& params)
                {
                    auto result = _parent.ShowWatermarkInternal(
                        !params.HideWatermark);
                    if (result == false) {
                        TRACE(Trace::Error, (_T("show watermark failed")));
                    }
                }

                void OnWatermarkSession(
                    const JsonRpc::OnWatermarkSessionParams& params)
                {
                    INotification::Status status;
                    switch (params.ConditionContext) {
                    case ConditionContext::SESSION_GRANTED:
                        status.state = INotification::Status::State::GRANTED;
                        break;
                    case ConditionContext::SESSION_NOT_REQUIRED:
                        status.state = INotification::Status::State::NOT_REQUIRED;
                        break;
                    case ConditionContext::SESSION_DENIED:
                        status.state = INotification::Status::State::DENIED;
                        break;
                    default:
                        TRACE(Trace::Error,
                            (_T("condition context %d"),
                                params.ConditionContext));
                        status.state = INotification::Status::State::FAILED;
                        status.failureReason = params.ConditionContext;
                    }
                    _parent.WatermarkStatusChanged(params.SessionId, status);
                }

                void OnUpdateWatermark(
                    const JsonRpc::OnUpdateWatermarkParams& params)
                {
                    auto copy = _parent.FindPalette(params.GraphicId);
                    if (copy.IsSet()) {
                        copy.Value().clutKey = params.WatermarkClutBufferKey;
                        copy.Value().imageKey = params.WatermarkImageBufferKey;
                        OnUpdateWatermark(params.GraphicId, copy);
                    }
                }

                void OnUpdateWatermark(uint32_t graphicId,
                    const Exchange::PalettedImageData& data)
                {
                    auto result = _parent.ModifyPalettedWatermarkInternal(
                        graphicId, data);
                    if (result == false) {
                        TRACE(Trace::Error,
                            (_T("modify palette %d failed"), graphicId));
                    }
                }

            private:
                Implementation& _parent;
            };

        private:
            class Notification : public Exchange::IWatermark::INotification {
            private:
                Notification(const Notification&) = delete;
                Notification& operator=(const Notification&) = delete;

            public:
                explicit Notification(Implementation& parent)
                    : _parent(parent)
                {
                }
                ~Notification() override = default;

                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::IWatermark::INotification)
                END_INTERFACE_MAP

            private:
                void WatermarkEvent(const string& eventName,
                    const string& parametersJson) override
                {
                    if (eventName == "onWatermarkRequestStatus") {
                        JsonRpc::OnWatermarkRequestStatusParams params;
                        params.FromString(parametersJson);
                        OnWatermarkRequestStatus(params);
                    } else if (eventName == "onWatermarkRenderFailed") {
                        JsonRpc::OnWatermarkRenderFailedParams params;
                        params.FromString(parametersJson);
                        OnWatermarkRenderFailed(params);
                    }
                }

            private:
                void OnWatermarkRequestStatus(
                    const JsonRpc::OnWatermarkRequestStatusParams& params)
                {
                    if (params.Type == "show") {
                        if (!params.Success) {
                            TRACE(Trace::Error, (_T("show watermark failed")));
                        }
                        OnShowRequestStatus(params.Success);
                    } else if (!params.Success) {
                        TRACE(Trace::Error,
                            (_T("%s watermark %d failed"),
                                params.Type.Value().c_str(), params.Id));
                    } else if (params.Type == "create") {
                        OnCreateRequestStatus(params.Id);
                    } else if (params.Type == "delete") {
                        OnDeleteRequestStatus(params.Id);
                    } else if (params.Type == "update") {
                        OnUpdateRequestStatus(params.Id);
                    }
                }

                void OnWatermarkRenderFailed(
                    const JsonRpc::OnWatermarkRenderFailedParams& params)
                {
                    auto copy = _parent.FindWatermark(params.Image);
                    if (copy.IsSet()) {
                        IContentProtection::INotification::Status status{
                            IContentProtection::INotification::Status::State::FAILED,
                            20001
                        };
                        _parent.WatermarkStatusChanged(copy.Value().SessionId,
                            status);
                    } else {
                        TRACE(Trace::Error,
                            (_T("render watermark %d failed"), params.Image));
                    }
                }

            private:
                void OnShowRequestStatus(bool success)
                {
                    _parent._showSuccess = success;
                    _parent._showEvent.SetEvent();
                }

                void OnCreateRequestStatus(uint32_t id)
                {
                    auto copy = _parent.FindWatermark(id);
                    if (copy.IsSet()) {
                        OnCreateRequestStatus(id,
                            copy.Value().GraphicImageBufferKey,
                            copy.Value().GraphicImageSize);
                    }
                }

                void OnCreateRequestStatus(uint32_t id,
                    uint32_t key, uint32_t size)
                {
                    auto result = _parent.UpdateWatermarkInternal(
                        id, key, size);
                    if (result == false) {
                        TRACE(Trace::Error,
                            (_T("update watermark %d failed"), id));
                    }
                }

                void OnDeleteRequestStatus(uint32_t id)
                {
                    _parent.DeleteWatermark(id);
                    _parent.DeletePalette(id);
                }

                void OnUpdateRequestStatus(uint32_t id)
                {
                    auto copy = _parent.FindWatermark(id);
                    if (copy.IsSet()
                        && copy.Value().AdjustVisibilityRequired) {
                        auto palette = _parent.GetPalettedWatermarkInternal(
                            id);
                        if ((palette.imageWidth == 0)
                            || (palette.imageHeight == 0)) {
                            TRACE(Trace::Error,
                                (_T("get palette %d failed"), id));
                        } else {
                            _parent.CreatePalette(id, palette);
                            OnUpdateRequestStatus(copy.Value().SessionId, id,
                                palette);
                        }
                    }
                }

                void OnUpdateRequestStatus(uint32_t sessionId, uint32_t id,
                    const Exchange::PalettedImageData& data)
                {
                    auto result = _parent.LoadClutWatermarkInternal(
                        sessionId, id, data);
                    if (result != Core::ERROR_NONE) {
                        TRACE(Trace::Error,
                            (_T("load clut %d failed: %d"), id, result));
                    }
                }

            private:
                Implementation& _parent;
            };

        private:
            Implementation(const Implementation&) = delete;
            Implementation& operator=(const Implementation&) = delete;

        public:
            explicit Implementation(
                JsonRpc::SecManager* secManager,
                Exchange::IWatermark* watermark)
                : _secManager(secManager)
                , _watermark(watermark)
                , _callback(*this)
                , _notification(*this)
                , _showEvent(false, true)
            {
                ASSERT(_secManager != nullptr);
                ASSERT(_watermark != nullptr);
                _secManager->AddRef();
                _watermark->AddRef();

                ASSERT(_secManager->Register(&_callback)
                    == Core::ERROR_NONE);
                ASSERT(_watermark->Register(&_notification)
                    == Core::ERROR_NONE);
            }
            ~Implementation() override
            {
                ASSERT(_secManager->Unregister(&_callback)
                    == Core::ERROR_NONE);
                ASSERT(_watermark->Unregister(&_notification)
                    == Core::ERROR_NONE);

                _secManager->Release();
                _watermark->Release();
            }

            BEGIN_INTERFACE_MAP(Implementation)
            INTERFACE_ENTRY(Exchange::IContentProtection)
            END_INTERFACE_MAP

        private:
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

            uint32_t OpenDrmSession(const string& clientId,
                KeySystem keySystem, const string& licenseRequest,
                const string& initData, uint32_t& sessionId, string& response)
                override
            {
                auto result = OpenDrmSessionInternal(clientId, keySystem,
                    licenseRequest, initData, sessionId, response);
                if (result == Core::ERROR_NONE) {
                    CreateSession({ sessionId, clientId, keySystem });
                }
                return result;
            }

            uint32_t SetDrmSessionState(uint32_t sessionId,
                State sessionState) override
            {
                uint32_t result;
                auto copy = FindSession(sessionId);
                if (copy.IsSet()) {
                    result = SetDrmSessionStateInternal(copy.Value().ClientId,
                        sessionId, sessionState);
                } else {
                    result = Core::ERROR_GENERAL;
                }
                return result;
            }

            uint32_t UpdateDrmSession(uint32_t sessionId,
                const string& licenseRequest,
                const string& initData, string& response) override
            {
                uint32_t result;
                auto copy = FindSession(sessionId);
                if (copy.IsSet()) {
                    result = UpdateDrmSessionInternal(
                        copy.Value().ClientId, copy.Value().KeySystem,
                        sessionId,
                        licenseRequest,
                        initData, response);
                } else {
                    result = Core::ERROR_GENERAL;
                }
                return result;
            }

            uint32_t CloseDrmSession(uint32_t sessionId) override
            {
                uint32_t result;
                auto copy = FindSession(sessionId);
                if (copy.IsSet()) {
                    result = CloseDrmSessionInternal(copy.Value().ClientId,
                        sessionId);
                    if (result == Core::ERROR_NONE) {
                        DeleteSession(sessionId);
                    }
                } else {
                    result = Core::ERROR_GENERAL;
                }
                return result;
            }

            uint32_t ShowWatermark(uint32_t /*sessionId*/,
                bool show, bool /*localOverlay*/) override
            {
                uint32_t result;
                if (ShowWatermarkInternal(show) == false) {
                    result = Core::ERROR_GENERAL;
                } else {
                    result = Core::ERROR_NONE;
                }
                return result;
            }

            uint32_t SetPlaybackPosition(uint32_t sessionId,
                int32_t speed, long position) override
            {
                return SetPlaybackPositionInternal(sessionId, speed, position);
            }

        private:
            uint32_t OpenDrmSessionInternal(const string& clientId,
                KeySystem keySystem, const string& licenseRequest,
                const string& initData, uint32_t& sessionId, string& response)
            {
                return _secManager->OpenPlaybackSession(clientId,
                    Core::JSON::EnumType<KeySystem>(keySystem).Data(),
                    licenseRequest, initData, sessionId, response);
            }

            uint32_t SetDrmSessionStateInternal(const string& clientId,
                uint32_t sessionId, State sessionState)
            {
                return _secManager->SetPlaybackSessionState(
                    clientId, sessionId,
                    Core::JSON::EnumType<State>(sessionState).Data());
            }

            uint32_t UpdateDrmSessionInternal(const string& clientId,
                KeySystem keySystem, uint32_t sessionId,
                const string& licenseRequest,
                const string& initData, string& response)
            {
                return _secManager->UpdatePlaybackSession(
                    clientId,
                    Core::JSON::EnumType<KeySystem>(keySystem).Data(),
                    licenseRequest,
                    initData, sessionId, response);
            }

            uint32_t CloseDrmSessionInternal(const string& clientId,
                uint32_t sessionId)
            {
                return _secManager->ClosePlaybackSession(clientId, sessionId);
            }

            bool ShowWatermarkInternal(bool show)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_showLock);
                _showEvent.ResetEvent();
                return (_watermark->ShowWatermark(show) == true)
                    && (_showEvent.Lock(RPC::CommunicationTimeOut)
                        == Core::ERROR_NONE)
                    && _showSuccess;
            }

            uint32_t SetPlaybackPositionInternal(uint32_t sessionId,
                int32_t playbackSpeed, long playbackPosition)
            {
                return _secManager->SetPlaybackSpeedState(sessionId,
                    playbackSpeed, playbackPosition);
            }

            Exchange::PalettedImageData GetPalettedWatermarkInternal(
                uint32_t id)
            {
                return _watermark->GetPalettedWatermark(id);
            }

            uint32_t LoadClutWatermarkInternal(uint32_t sessionId, uint32_t id,
                const Exchange::PalettedImageData& data)
            {
                return _secManager->LoadClutWatermark(
                    sessionId, id,
                    data.clutKey, data.imageKey,
                    data.clutSize, data.clutType,
                    data.imageWidth, data.imageHeight,
                    ((float)data.imageWidth / (float)data.imageHeight));
            }

            uint32_t CreateWatermarkInternal(uint32_t graphicId,
                uint32_t zIndex)
            {
                return _watermark->CreateWatermark(graphicId, zIndex);
            }

            uint32_t DeleteWatermarkInternal(uint32_t graphicId)
            {
                return _watermark->DeleteWatermark(graphicId);
            }

            uint32_t ModifyPalettedWatermarkInternal(uint32_t graphicId,
                const Exchange::PalettedImageData& data)
            {
                return _watermark->ModifyPalettedWatermark(graphicId, data);
            }

            uint32_t UpdateWatermarkInternal(uint32_t id,
                uint32_t key, uint32_t size)
            {
                return _watermark->UpdateWatermark(id, key, size);
            }

        private:
            void WatermarkStatusChanged(uint32_t sessionId,
                const INotification::Status& status)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);
                for (auto& i : _clients) {
                    i->WatermarkStatusChanged(sessionId, status);
                }
            }

        private:
            void CreateSession(const Session& session)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_sessionLock);
                _sessions.push_back(session);
            }

            Core::OptionalType<Session> FindSession(uint32_t sessionId)
            {
                Core::OptionalType<Session> result;
                Core::SafeSyncType<Core::CriticalSection> lock(_sessionLock);
                for (auto it = _sessions.begin(); it != _sessions.end();
                    ++it) {
                    if (it->SessionId == sessionId) {
                        result = *it;
                        break;
                    }
                }
                return result;
            }

            void DeleteSession(uint32_t sessionId)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_sessionLock);
                for (auto it = _sessions.begin(); it != _sessions.end();) {
                    if (it->SessionId == sessionId) {
                        it = _sessions.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

        private:
            void CreateWatermark(const Watermark& watermark)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_watermarkLock);
                _watermarks.push_back(watermark);
            }

            Core::OptionalType<Watermark> FindWatermark(uint32_t graphicId)
            {
                Core::OptionalType<Watermark> result;
                Core::SafeSyncType<Core::CriticalSection> lock(_watermarkLock);
                for (auto it = _watermarks.begin(); it != _watermarks.end();
                    ++it) {
                    if (it->GraphicId == graphicId) {
                        result = *it;
                        break;
                    }
                }
                return result;
            }

            void DeleteWatermark(uint32_t graphicId)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_watermarkLock);
                for (auto it = _watermarks.begin(); it != _watermarks.end();) {
                    if (it->GraphicId == graphicId) {
                        it = _watermarks.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

        private:
            void CreatePalette(uint32_t id,
                const Exchange::PalettedImageData& palette)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_paletteLock);
                _palettes.emplace(id, palette);
            }

            Core::OptionalType<Exchange::PalettedImageData> FindPalette(
                uint32_t id)
            {
                Core::OptionalType<Exchange::PalettedImageData> result;
                Core::SafeSyncType<Core::CriticalSection> lock(_paletteLock);
                auto it = _palettes.find(id);
                if (it != _palettes.end()) {
                    result = it->second;
                }
                return result;
            }

            void DeletePalette(uint32_t id)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_paletteLock);
                _palettes.erase(id);
            }

        private:
            JsonRpc::SecManager* _secManager;
            Exchange::IWatermark* _watermark;
            Core::Sink<Callback> _callback;
            Core::Sink<Notification> _notification;
            std::list<INotification*> _clients;
            Core::CriticalSection _clientLock;
            std::list<Session> _sessions;
            Core::CriticalSection _sessionLock;
            std::list<Watermark> _watermarks;
            Core::CriticalSection _watermarkLock;
            std::map<uint32_t, Exchange::PalettedImageData> _palettes;
            Core::CriticalSection _paletteLock;
            Core::CriticalSection _showLock;
            Core::Event _showEvent;
            bool _showSuccess;
        };

    private:
        ContentProtection(const ContentProtection&) = delete;
        ContentProtection& operator=(const ContentProtection&) = delete;

    public:
        ContentProtection()
            : _contentProtection(nullptr)
        {
        }
        ~ContentProtection() override = default;

        BEGIN_INTERFACE_MAP(ContentProtection)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IContentProtection, _contentProtection)
        END_INTERFACE_MAP

    private:
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

    private:
        Exchange::IContentProtection* _contentProtection;
    };

} // namespace Plugin
} // namespace WPEFramework
