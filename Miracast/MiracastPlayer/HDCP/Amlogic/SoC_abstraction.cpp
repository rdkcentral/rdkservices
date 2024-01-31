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
#include <hdcp_module.h>

using namespace std;

SoC_WfdHdcpEventCallback gWfdHdcpEventCB = nullptr;

static SoC_WfdHdcpResultType convert_WfdHdcpResultTypeToGeneric( amlWfdHdcpResultType resultType )
{
    SoC_WfdHdcpResultType genericResultType = SOC_HDCP_RESULT_ERROR_INVALID_PARAMETER;
    switch (resultType)
    {
        case HDCP_RESULT_SUCCESS:
        {
            genericResultType = SOC_HDCP_RESULT_SUCCESS;
        }
        break;
        case HDCP_RESULT_ERROR_INVALID_PARAMETER:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_INVALID_PARAMETER;
        }
        break;
        case HDCP_RESULT_ERROR_INVALID_ADDRESS:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_INVALID_ADDRESS;
        }
        break;
        case HDCP_RESULT_ERROR_OUT_OF_MEM:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_OUT_OF_MEM;
        }
        break;
        case HDCP_RESULT_ERROR_SPEC_VERSION:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_SPEC_VERSION;
        }
        break;
        case HDCP_RESULT_ERROR_KEY_ERR:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_KEY_ERR;
        }
        break;
        case HDCP_RESULT_ERROR_AUTH_ERR:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_AUTH_ERR;
        }
        break;
        case HDCP_RESULT_ERROR_DISCONNECTED:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_DISCONNECTED;
        }
        break;
        case HDCP_RESULT_ERROR_TEE_ERR:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_TEE_ERR;
        }
        break;
        case HDCP_RESULT_ERROR_DECRYPT_ERR:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_DECRYPT_ERR;
        }
        break;
        case HDCP_RESULT_ERROR_INTERNAL:
        {
            genericResultType = SOC_HDCP_RESULT_ERROR_INTERNAL;
        }
        break;
        case HDCP_RESULT_MAX_VALUE:
        {
            genericResultType = SOC_HDCP_RESULT_MAX_VALUE;
        }
        break;
        default:
            break;
    }
    return genericResultType;
}

static SoC_WfdHdcpEventType convert_WfdHdcpEventTypeToGeneric( amlWfdHdcpEventType eventType )
{
    SoC_WfdHdcpEventType genericEventType = SOC_HDCP_EVENT_NOT_IMPLEMENTED;
    switch (eventType)
    {
        case HDCP_EVENT_INITIALIZATION_COMPLETE:
        {
            genericEventType = SOC_HDCP_EVENT_INITIALIZATION_COMPLETE;
        }
        break;
        case HDCP_EVENT_INITIALIZATION_FAILED:
        {
            genericEventType = SOC_HDCP_EVENT_INITIALIZATION_FAILED;
        }
        break;
        case HDCP_EVENT_SHUTDOWN_COMPLETE:
        {
            genericEventType = SOC_HDCP_EVENT_SHUTDOWN_COMPLETE;
        }
        break;
        case HDCP_EVENT_SPEC_VERSION_NOT_SUPPORT:
        {
            genericEventType = SOC_HDCP_EVENT_SPEC_VERSION_NOT_SUPPORT;
        }
        break;
        case HDCP_EVENT_AUTH_DISCONNECTED:
        {
            genericEventType = SOC_HDCP_EVENT_AUTH_DISCONNECTED;
        }
        break;
        case HDCP_EVENT_AUTH_ERROR:
        {
            genericEventType = SOC_HDCP_EVENT_AUTH_ERROR;
        }
        break;
        case HDCP_EVENT_INTERNAL_ERROR:
        {
            genericEventType = SOC_HDCP_EVENT_INTERNAL_ERROR;
        }
        break;
        case HDCP_EVENT_MAX_VALUE:
        {
            genericEventType = SOC_HDCP_EVENT_MAX_VALUE;
        }
        break;
        default:
            break;
    }
    return genericEventType;
}

static SoC_WfdHdcpLevel convert_WfdHdcpLevelToGeneric( amlWfdHdcpLevel level )
{
    SoC_WfdHdcpLevel genericHdcpLevel = SOC_HDCP_LEVEL_NONE;
    switch (level)
    {
        case HDCP_LEVEL_NONE:
        {
            genericHdcpLevel = SOC_HDCP_LEVEL_NONE;
        }
        break;
        case HDCP_LEVEL_14:
        {
            genericHdcpLevel = SOC_HDCP_LEVEL_14;
        }
        break;
        case HDCP_LEVEL_20:
        {
            genericHdcpLevel = SOC_HDCP_LEVEL_20;
        }
        break;
        case HDCP_LEVEL_21:
        {
            genericHdcpLevel = SOC_HDCP_LEVEL_21;
        }
        break;
        case HDCP_LEVEL_22:
        {
            genericHdcpLevel = SOC_HDCP_LEVEL_22;
        }
        break;
        case HDCP_LEVEL_23:
        {
            genericHdcpLevel = SOC_HDCP_LEVEL_23;
        }
        break;
        default:
            break;
    }
    return genericHdcpLevel;
}

static void SoC_WfdHdcpStateEventCallback(amlWfdHdcpHandle handle, amlWfdHdcpEventType event)
{
    if ( nullptr != gWfdHdcpEventCB )
    {
        SoC_WfdHdcpEventType genericEvent = convert_WfdHdcpEventTypeToGeneric(event);
        gWfdHdcpEventCB( static_cast<SoC_WfdHdcpHandle>(handle) , genericEvent );
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
    SoC_WfdHdcpResultType genericReturnType = SOC_HDCP_RESULT_ERROR_PREV_INSTANCE_NOT_FREED;
    amlWfdHdcpResultType returnType = HDCP_RESULT_ERROR_INVALID_PARAMETER;
    if ( nullptr == gWfdHdcpEventCB )
    {
        gWfdHdcpEventCB = callback;
        returnType = amlWfdHdcpInitAsync( host , port , SoC_WfdHdcpStateEventCallback , static_cast<amlWfdHdcpHandle*>(handle));
        if ( HDCP_RESULT_SUCCESS != returnType )
        {
            gWfdHdcpEventCB = nullptr;
        }
        genericReturnType = convert_WfdHdcpResultTypeToGeneric(returnType);
    }
    return genericReturnType;
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
    amlWfdHdcpResultType returntype = amlWfdHdcpDeinitAsync( static_cast<amlWfdHdcpHandle>(handle) );
    return convert_WfdHdcpResultTypeToGeneric(returntype);
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
    amlWfdHdcpResultType returntype = amlWfdDestroyHandle( static_cast<amlWfdHdcpHandle>(handle) );
    return convert_WfdHdcpResultTypeToGeneric(returntype);
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
    amlWfdHdcpResultType returntype = amlWfdHdcpInit(host, port, static_cast<amlWfdHdcpHandle*>(handle));
    return convert_WfdHdcpResultTypeToGeneric(returntype);
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
    amlWfdHdcpResultType returntype = amlWfdHdcpDeinit( static_cast<amlWfdHdcpHandle>(handle));
    return convert_WfdHdcpResultTypeToGeneric(returntype);
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
    amlWfdHdcpResultType returntype = HDCP_RESULT_ERROR_INVALID_PARAMETER;
    amlWfdHdcpLevel HdcpLevel = HDCP_LEVEL_NONE;

    if ( nullptr != level )
    {
        returntype = amlWfdHdcpGetSupportLevel( static_cast<amlWfdHdcpHandle>(handle) , &HdcpLevel );
        if ( HDCP_RESULT_SUCCESS == returntype )
        {
            *level = convert_WfdHdcpLevelToGeneric(HdcpLevel);
        }
    }
    return convert_WfdHdcpResultTypeToGeneric(returntype);
}