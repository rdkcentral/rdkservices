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
#include <SoC_abstraction.h>

using namespace std;

SoC_WfdHdcpEventCallback gWfdHdcpEventCB = nullptr;

static SoC_WfdHdcpResultType convert_WfdHdcpResultTypeToGeneric( amlWfdHdcpResultType resultType )
{
    return SOC_HDCP_RESULT_ERROR_NOT_IMPLEMENTED;
}

static SoC_WfdHdcpEventType convert_WfdHdcpEventTypeToGeneric( amlWfdHdcpEventType eventType )
{
    return SOC_HDCP_EVENT_NOT_IMPLEMENTED;
}

static SoC_WfdHdcpLevel convert_WfdHdcpLevelToGeneric( amlWfdHdcpEventType eventType )
{
    return HDCP_LEVEL_NONE;
}

static void SoC_WfdHdcpStateEventCallback(amlWfdHdcpHandle handle, amlWfdHdcpEventType event)
{
    if ( nullptr != gWfdHdcpEventCB )
    {
        SoC_WfdHdcpEventType genericEvent = convert_WfdHdcpEventTypeToGeneric(event);
        gWfdHdcpEventCB( static_cast<SoC_WfdHdcpHandle>handle , genericEvent );
    }
}

/**
    Define module init function, this functiom must be this first calling to get a handle
    Paras:
        host:     [IN]     source's address     can be null, then use 127.0.0.1 as default address
        port:     [IN]     hdcp connect's port  can't be 0, must be a valid value 0 ~ 65535
        callback: [IN]     event callback       can't be null, must a valid callback function pointer
        handle:   [IN/OUT] module handle        can't be null
    Retrun:
        Success return HDCP_RESULT_SUCCESS
        Failed  return other
*/
SoC_WfdHdcpResultType SoC_WfdHdcpInitAsync(const char * host, uint16_t port, SoC_WfdHdcpEventCallback callback, SoC_WfdHdcpHandle * handle)
{
    return SOC_HDCP_RESULT_ERROR_NOT_IMPLEMENTED;
}

/**
    Define deinit interface to release the alloc source
    Para:
        handle:   [IN] which return from SoC_WfdHdcpInitAsync
    Return:
        success return HDCP_RESULT_SUCCESS
        failed return other
*/
SoC_WfdHdcpResultType SoC_WfdHdcpDeinitAsync(SoC_WfdHdcpHandle handle)
{
    return SOC_HDCP_RESULT_ERROR_NOT_IMPLEMENTED;
}

/**
    Define deinit interface to release the handle
    Para:
        handle:   [IN] which return from SoC_WfdHdcpInitAsync
    Return:
        success return HDCP_RESULT_SUCCESS
        failed return other
*/
SoC_WfdHdcpResultType SoC_WfdDestroyHandle(SoC_WfdHdcpHandle handle)
{
    return SOC_HDCP_RESULT_ERROR_NOT_IMPLEMENTED;
}

/**
    Define module init function.
    Paras:
        host:   [IN]        source's address     can be null, then use 127.0.0.1 as default address
        port:   [IN]        hdcp connect's port  can't be 0, must be a valid value 0 ~ 65535
        handle: [IN/OUT]    SoC_WfdHdcpHandle * type handle, success is not null or failed will be null
    Retrun:
        Success return HDCP_RESULT_SUCCESS
        Failed  return other
*/
SoC_WfdHdcpResultType SoC_WfdHdcpInit(const char * host, uint16_t port, SoC_WfdHdcpHandle * handle)
{
    return SOC_HDCP_RESULT_ERROR_NOT_IMPLEMENTED;
}

/**
    Define deinit interface to release the alloc source
    Para:
        handle:     [IN] which return from SoC_WfdHdcpInitAsync
    Return:
        success return HDCP_RESULT_SUCCESS
        failed return  other
*/
SoC_WfdHdcpResultType SoC_WfdHdcpDeinit(SoC_WfdHdcpHandle handle)
{
    return SOC_HDCP_RESULT_ERROR_NOT_IMPLEMENTED;
}

/**
    Define interface to return support HDCP level
    Paras:
        handle:     [IN]    which return from SoC_WfdHdcpInitAsync
        levle :     [INOUT] used to received the support hdcp level
    Return:
    Return:
        Success HDCP_RESULT_SUCCESS
        Failed  return other
*/
SoC_WfdHdcpResultType SoC_WfdHdcpGetSupportLevel(SoC_WfdHdcpHandle handle, SoC_WfdHdcpLevel * level)
{
    return SOC_HDCP_RESULT_ERROR_NOT_IMPLEMENTED;
}