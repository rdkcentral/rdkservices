/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 Synamedia
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

#include "Rfc.h"
#include <gmock/gmock.h>

RfcApiImpl* RfcApi::impl = nullptr;

RfcApi::RfcApi() {}

void RfcApi::setImpl(RfcApiImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

WDMP_STATUS RfcApi::getRFCParameter(char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData)
{
   EXPECT_NE(impl, nullptr);
   return impl->getRFCParameter(pcCallerID, pcParameterName, pstParamData);
}

WDMP_STATUS RfcApi::setRFCParameter(char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType)
{
   EXPECT_NE(impl, nullptr);
   return impl->setRFCParameter(pcCallerID, pcParameterName, pcParameterValue, eDataType);
}

const char* RfcApi::getRFCErrorString(WDMP_STATUS code)
{
   EXPECT_NE(impl, nullptr);
   return impl->getRFCErrorString(code);
}

WDMP_STATUS (*getRFCParameter)(char*,const char*,RFC_ParamData_t*) = &RfcApi::getRFCParameter;
WDMP_STATUS (*setRFCParameter)(char*,const char*,const char*,DATA_TYPE) = &RfcApi::setRFCParameter;
const char* (*getRFCErrorString)(WDMP_STATUS) = &RfcApi::getRFCErrorString;
