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
 
#include "Utils.h"

using std::unique_ptr;
using std::vector;

namespace WPEFramework {
namespace WebKit {
    namespace Utils {

        // Adds string to WKMutableArray.
        void AppendStringToWKArray(const string& item, WKMutableArrayRef array)
        {
            WKStringRef itemString = WKStringCreateWithUTF8CString(item.c_str());
            WKArrayAppendItem(array, itemString);
            WKRelease(itemString);
        }

        // Reads string from WKArray.
        string GetStringFromWKArray(WKArrayRef array, unsigned int index)
        {
            WKStringRef itemString = static_cast<WKStringRef>(WKArrayGetItemAtIndex(array, index));
            return WKStringToString(itemString);
        }

        // Converts WKString to string.
        string WKStringToString(WKStringRef wkStringRef)
        {
            size_t bufferSize = WKStringGetMaximumUTF8CStringSize(wkStringRef);
            std::unique_ptr<char[]> buffer(new char[bufferSize]);
            size_t stringLength = WKStringGetUTF8CString(wkStringRef, buffer.get(), bufferSize);
            return Core::ToString(buffer.get(), stringLength - 1);
        }

        // Converts WKArray to string vector.
        vector<string> ConvertWKArrayToStringVector(WKArrayRef array)
        {
            size_t arraySize = WKArrayGetSize(array);

            vector<string> stringVector;

            for (unsigned int index = 0; index < arraySize; ++index) {
                stringVector.push_back(GetStringFromWKArray(array, index));
            }

            return stringVector;
        }
    }
}
}
