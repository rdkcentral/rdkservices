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

        // These classes are needed because SecManager has no json interface

        class OnAddWatermarkParams : public Core::JSON::Container {
        private:
            OnAddWatermarkParams(const OnAddWatermarkParams&) = delete;
            OnAddWatermarkParams& operator=(
                const OnAddWatermarkParams&)
                = delete;

        public:
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

        public:
            Core::JSON::DecUInt32 SessionId;
            Core::JSON::DecUInt32 GraphicId;
            Core::JSON::Boolean AdjustVisibilityRequired;
            Core::JSON::DecUInt32 ZIndex;
            Core::JSON::String ImageType;
            Core::JSON::DecUInt32 GraphicImageBufferKey;
            Core::JSON::DecUInt32 GraphicImageSize;
        };

        class OnRemoveWatermarkParams : public Core::JSON::Container {
        private:
            OnRemoveWatermarkParams(
                const OnRemoveWatermarkParams&)
                = delete;
            OnRemoveWatermarkParams& operator=(
                const OnRemoveWatermarkParams&)
                = delete;

        public:
            OnRemoveWatermarkParams()
            {
                Add(_T("graphicId"), &GraphicId);
            }

        public:
            Core::JSON::DecUInt32 GraphicId;
        };

        class OnDisplayWatermarkParams : public Core::JSON::Container {
        private:
            OnDisplayWatermarkParams(
                const OnDisplayWatermarkParams&)
                = delete;
            OnDisplayWatermarkParams& operator=(
                const OnDisplayWatermarkParams&)
                = delete;

        public:
            OnDisplayWatermarkParams()
            {
                Add(_T("hideWatermark"), &HideWatermark);
            }

        public:
            Core::JSON::Boolean HideWatermark;
        };

        class OnWatermarkSessionParams : public Core::JSON::Container {
        private:
            OnWatermarkSessionParams(
                const OnWatermarkSessionParams&)
                = delete;
            OnWatermarkSessionParams& operator=(
                const OnWatermarkSessionParams&)
                = delete;

        public:
            OnWatermarkSessionParams()
            {
                Add(_T("sessionId"), &SessionId);
                Add(_T("conditionContext"), &ConditionContext);
            }

        public:
            Core::JSON::DecUInt32 SessionId;
            Core::JSON::DecUInt32 ConditionContext;
        };

        class OnUpdateWatermarkParams : public Core::JSON::Container {
        private:
            OnUpdateWatermarkParams(
                const OnUpdateWatermarkParams&)
                = delete;
            OnUpdateWatermarkParams& operator=(
                const OnUpdateWatermarkParams&)
                = delete;

        public:
            OnUpdateWatermarkParams()
            {
                Add(_T("graphicId"), &GraphicId);
                Add(_T("watermarkClutBufferKey"), &WatermarkClutBufferKey);
                Add(_T("watermarkImageBufferKey"),
                    &WatermarkImageBufferKey);
            }

        public:
            Core::JSON::DecUInt32 GraphicId;
            Core::JSON::DecUInt32 WatermarkClutBufferKey;
            Core::JSON::DecUInt32 WatermarkImageBufferKey;
        };

    } // namespace JsonRpc
} // namespace Plugin
} // namespace WPEFramework
