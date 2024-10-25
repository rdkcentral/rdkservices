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

        // These classes are needed because Watermark has no json interface

        class OnWatermarkRequestStatusParams
            : public Core::JSON::Container {
        private:
            OnWatermarkRequestStatusParams(
                const OnWatermarkRequestStatusParams&)
                = delete;
            OnWatermarkRequestStatusParams& operator=(
                const OnWatermarkRequestStatusParams&)
                = delete;

        public:
            OnWatermarkRequestStatusParams()
            {
                Add(_T("id"), &Id);
                Add(_T("type"), &Type);
                Add(_T("success"), &Success);
            }

        public:
            Core::JSON::DecUInt32 Id;
            Core::JSON::String Type;
            Core::JSON::Boolean Success;
        };

        class OnWatermarkRenderFailedParams
            : public Core::JSON::Container {
        private:
            OnWatermarkRenderFailedParams(
                const OnWatermarkRenderFailedParams&)
                = delete;
            OnWatermarkRenderFailedParams& operator=(
                const OnWatermarkRenderFailedParams&)
                = delete;

        public:
            OnWatermarkRenderFailedParams()
            {
                Add(_T("image"), &Image);
            }

        public:
            Core::JSON::DecUInt32 Image;
        };

    } // namespace JsonRpc
} // namespace Plugin
} // namespace WPEFramework
