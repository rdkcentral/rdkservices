/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
#include <functional>

#include "Module.h"

#include <interfaces/IDRM.h>

namespace WPEFramework {
    namespace Plugin {

        class CapsParser : public CDMi::ICapsParser {
        public:
            CapsParser(const CapsParser&) = delete;
            CapsParser& operator= (const CapsParser&) = delete;

            CapsParser();
            ~CapsParser() override;

        public:
            void Parse(const uint8_t* info, uint16_t infoLength) override; 

            const uint16_t GetHeight() const { return _height; } 
            const uint16_t GetWidth() const { return _width; } 
            
            virtual const CDMi::MediaType GetMediaType() const { return _mediaType; } 
        
        private:
            virtual ::string FindMarker(::string& data, ::string& tag) const;
            virtual void SetMediaType(::string& media);
            virtual void SetWidth(::string& width);
            virtual void SetHeight(::string& height);
        private:
            size_t _lastHash;
            ::string _mediaTag;
            ::string _widthTag;
            ::string _heightTag;
            CDMi::MediaType _mediaType;
            uint16_t _width;
            uint16_t _height;
        };
    }
}
