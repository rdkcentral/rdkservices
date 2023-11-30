/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 RDK Management
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

#include "../helpers/UtilsInputValidator.h"

namespace WPEFramework {
namespace Plugin {

using Utils::ExpectedValues;
using Utils::Validator;
using Utils::ValidationManager;

// Country and language codes from ISO-639/ISO-3166.
static const std::set<std::string> expectedLanguageSetCollection = {
    "af", "af-ZA", "ar", "ar-AE", "ar-BH", "ar-DZ", "ar-EG", "ar-IQ", "ar-JO", "ar-KW", "ar-LB", "ar-LY",
    "ar-MA", "ar-OM", "ar-QA", "ar-SA", "ar-SY", "ar-TN", "ar-YE", "az", "az-AZ", "az-AZ",
    "be", "be-BY", "bg", "bg-BG", "bs-BA",
    "ca", "ca-ES", "cs", "cs-CZ", "cy", "cy-GB",
    "da", "da-DK", "de", "de-AT", "de-CH", "de-DE", "de-LI", "de-LU", "dv", "dv-MV",
    "el", "el-GR", "en", "en-AU", "en-BZ", "en-CA", "en-CB", "en-GB", "en-IE", "en-JM", "en-NZ", "en-PH",
    "en-TT", "en-US", "en-ZA", "en-ZW", "eo", "es", "es-AR", "es-BO", "es-CL", "es-CO", "es-CR", "es-DO",
    "es-EC", "es-ES", "es-ES", "es-GT", "es-HN", "es-MX", "es-NI", "es-PA", "es-PE", "es-PR", "es-PY", "es-SV",
    "es-UY", "es-VE", "et", "et-EE", "eu", "eu-ES",
    "fa", "fa-IR", "fi", "fi-FI", "fo", "fo-FO", "fr", "fr-BE", "fr-CA", "fr-CH", "fr-FR", "fr-LU", "fr-MC",
    "gl", "gl-ES", "gu", "gu-IN",
    "he", "he-IL", "hi", "hi-IN", "hr", "hr-BA", "hr-HR", "hu", "hu-HU", "hy", "hy-AM", "id", "id-ID",
    "is", "is-IS", "it", "it-CH", "it-IT",
    "ja", "ja-JP",
    "ka", "ka-GE", "kk", "kk-KZ", "kn", "kn-IN", "ko", "ko-KR", "kok", "kok-IN", "ky", "ky-KG",
    "lt", "lt-LT", "lv", "lv-LV",
    "mi", "mi-NZ", "mk", "mk-MK", "mn", "mn-MN", "mr", "mr-IN", "ms", "ms-BN", "ms-MY", "mt", "mt-MT",
    "nb", "nb-NO", "nl", "nl-BE", "nl-NL", "nn-NO", "ns", "ns-ZA",
    "pa", "pa-IN", "pl", "pl-PL", "ps", "ps-AR", "pt", "pt-BR", "pt-PT",
    "qu", "qu-BO", "qu-EC", "qu-PE",
    "ro", "ro-RO", "ru", "ru-RU",
    "sa", "sa-IN", "se", "se-FI", "se-FI", "se-FI", "se-NO", "se-NO", "se-NO", "se-SE", "se-SE",
    "se-SE", "sk", "sk-SK", "sl", "sl-SI", "sq", "sq-AL", "sr-BA", "sr-BA", "sr-SP", "sr-SP",
    "sv", "sv-FI", "sv-SE", "sw", "sw-KE", "syr", "syr-SY",
    "ta", "ta-IN", "te", "te-IN", "th", "th-TH", "tl", "tl-PH", "tn", "tn-ZA", "tr", "tr-TR", "tt", "tt-RU", "ts",
    "uk", "uk-UA", "ur", "ur-PK", "uz", "uz-UZ", "uz-UZ",
    "vi", "vi-VN",
    "xh", "xh-ZA",
    "zh", "zh-CN", "zh-HK", "zh-MO", "zh-SG", "zh-TW", "zu", "zu-ZA",""};

static const std::set<std::string> expectedVoicesSetCollection = {"carol","amelie","Angelica","ava",""};

struct InputValidation
{
    NON_COPYABLE(InputValidation);
    NON_MOVABLE(InputValidation);
    ~InputValidation() = default;

    static ValidationManager &Instance()
    {
        static InputValidation s_instance;
        return s_instance.m_validationManager;
    }

    private:
        InputValidation() = default;

    private:
        ValidationManager m_validationManager;
};

} // namespace Plugin
} // namespace WPEFramework
