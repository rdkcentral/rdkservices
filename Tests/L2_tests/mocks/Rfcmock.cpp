# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#include "Rfc.h"


WDMP_STATUS getRFCParameter(char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData)
{
     printf("SUMA: inside mock getRFCparameter");
     pstParamData->type = WDMP_BOOLEAN;
     sprintf(pstParamData->value,"true");
     return WDMP_SUCCESS;
}
WDMP_STATUS setRFCParameter(char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType)
{
     return WDMP_SUCCESS;
}

const char* getRFCErrorString(WDMP_STATUS code)
{
   return "error";
}
