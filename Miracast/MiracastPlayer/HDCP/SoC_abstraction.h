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

#ifndef __SOC_WFD_HDCP_MODULE_H__
#define __SOC_WFD_HDCP_MODULE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdint.h>

/** Define Function's return value */
typedef enum {
    SOC_HDCP_RESULT_SUCCESS = 0,
    SOC_HDCP_RESULT_ERROR_INVALID_PARAMETER,
    SOC_HDCP_RESULT_ERROR_INVALID_ADDRESS,
    SOC_HDCP_RESULT_ERROR_OUT_OF_MEM,
    SOC_HDCP_RESULT_ERROR_SPEC_VERSION,
    SOC_HDCP_RESULT_ERROR_KEY_ERR,
    SOC_HDCP_RESULT_ERROR_AUTH_ERR,
    SOC_HDCP_RESULT_ERROR_DISCONNECTED,
    SOC_HDCP_RESULT_ERROR_TEE_ERR,
    SOC_HDCP_RESULT_ERROR_DECRYPT_ERR,
    SOC_HDCP_RESULT_ERROR_INTERNAL,
    SOC_HDCP_RESULT_ERROR_PREV_INSTANCE_NOT_FREED,  // Previous session not closed properly
    SOC_HDCP_RESULT_ERROR_NOT_IMPLEMENTED,  // Yet to do SoC implementation
    SOC_HDCP_RESULT_MAX_VALUE,
}
SoC_WfdHdcpResultType;

/** Define support HDCP level */
typedef enum {
    SOC_HDCP_LEVEL_NONE = 0,        //No hdcp
    SOC_HDCP_LEVEL_14,              //For hdcp 1.4
    SOC_HDCP_LEVEL_20,              //For hdcp 2.0
    SOC_HDCP_LEVEL_21,              //For hdcp 2.1
    SOC_HDCP_LEVEL_22,              //For hdcp 2.2
    SOC_HDCP_LEVEL_23,              //For hdcp 2.3
} SoC_WfdHdcpLevel;

/** Define the callback event for async notify the init deinit and connect event */
typedef enum
{
    SOC_HDCP_EVENT_INITIALIZATION_COMPLETE = 0,       //Async notify init success
    SOC_HDCP_EVENT_INITIALIZATION_FAILED,             //Async notify init failed
    SOC_HDCP_EVENT_SHUTDOWN_COMPLETE,                 //Async notify shutdown success
    SOC_HDCP_EVENT_SPEC_VERSION_NOT_SUPPORT,          //Async notify not support' tx version
    SOC_HDCP_EVENT_AUTH_DISCONNECTED,                 //Async notify rx tx auth process disconnected
    SOC_HDCP_EVENT_AUTH_ERROR,                        //Async notify auth error
    SOC_HDCP_EVENT_INTERNAL_ERROR,                    //Async notify nternal error
    SOC_HDCP_EVENT_NOT_IMPLEMENTED,                   // Yet to do SoC implementation
    SOC_HDCP_EVENT_MAX_VALUE
}
SoC_WfdHdcpEventType;

typedef void* SoC_WfdHdcpHandle;

/** Define event receiver callbacktype */
typedef void (*SoC_WfdHdcpEventCallback)(SoC_WfdHdcpHandle handle, SoC_WfdHdcpEventType event);

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
SoC_WfdHdcpResultType SoC_WfdHdcpInitAsync(const char * host, uint16_t port, SoC_WfdHdcpEventCallback callback, SoC_WfdHdcpHandle * handle);

/**
    Define deinit interface to release the alloc source
    Para:
        handle:   [IN] which return from SoC_WfdHdcpInitAsync
    Return:
        success return HDCP_RESULT_SUCCESS
        failed return other
*/
SoC_WfdHdcpResultType SoC_WfdHdcpDeinitAsync(SoC_WfdHdcpHandle handle);

/**
    Define deinit interface to release the handle
    Para:
        handle:   [IN] which return from SoC_WfdHdcpInitAsync
    Return:
        success return HDCP_RESULT_SUCCESS
        failed return other
*/
SoC_WfdHdcpResultType SoC_WfdDestroyHandle(SoC_WfdHdcpHandle handle);

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
SoC_WfdHdcpResultType SoC_WfdHdcpInit(const char * host, uint16_t port, SoC_WfdHdcpHandle * handle);

/**
    Define deinit interface to release the alloc source
    Para:
        handle:     [IN] which return from SoC_WfdHdcpInitAsync
    Return:
        success return HDCP_RESULT_SUCCESS
        failed return  other
*/
SoC_WfdHdcpResultType SoC_WfdHdcpDeinit(SoC_WfdHdcpHandle handle);

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
SoC_WfdHdcpResultType SoC_WfdHdcpGetSupportLevel(SoC_WfdHdcpHandle handle, SoC_WfdHdcpLevel * level);

#ifdef __cplusplus
}
#endif
#endif