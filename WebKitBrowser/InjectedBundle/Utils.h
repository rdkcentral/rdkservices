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
 
#ifndef __INJECTEDBUNDLE_UTILS_H
#define __INJECTEDBUNDLE_UTILS_H

#include "Module.h"

#include <WPE/WebKit.h>

#include <memory>
#include <vector>

namespace WPEFramework {
namespace WebKit {
    namespace Utils {
        void AppendStringToWKArray(const string& item, WKMutableArrayRef array);
        string GetStringFromWKArray(WKArrayRef array, unsigned int index);
        std::string GetURL();
        WKBundleRef GetBundle();
        string WKStringToString(WKStringRef wkStringRef);
        std::vector<string> ConvertWKArrayToStringVector(WKArrayRef array);
    };
}
}

#endif // __INJECTEDBUNDLE_UTILS_H
