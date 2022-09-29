/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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
 
/**
 * @file dsDisplay.h
 */



/**
* @defgroup devicesettings
* @{
* @defgroup hal
* @{
**/


#ifndef _DS_VIDEODISPLAY_H_
#define _DS_VIDEODISPLAY_H_

#include "dsError.h"
#include "dsTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @enum typedef enum _dsDisplayEvent_t
 * @brief This enumeration defines standard display event types. 
 * @note Vendors, please add appropriate events needed for your implementation.
 */
typedef enum _dsDisplayEvent_t {
    dsDISPLAY_EVENT_CONNECTED = 0,         //!< Display connected event.
    dsDISPLAY_EVENT_DISCONNECTED,          //!< Display disconnected event.
    dsDISPLAY_RXSENSE_ON,			       //!< Rx Sense ON event
    dsDISPLAY_RXSENSE_OFF,			       //!< Rx Sense OFF event
    dsDISPLAY_HDCPPROTOCOL_CHANGE,		   //!< HDCP Protocol Version Change event

    dsDISPLAY_EVENT_MAX
} dsDisplayEvent_t;


/** @addtogroup DSHAL_DISPLAY_API Device Settings HAL Display Public API
 *  @ingroup devicesettingshalapi
 *  A set of events sourced from display device (e.g. TV)
 *  Described herein are DeviceSettings HAL types and functions that are part of the
 *  Display subsystem. The Display subsystem manages operations relating to connected
 *  Vendors please add appropriate events needed for your implementation.
 *  display devices, e.g. TVs.
 *  @{
 */


/**
 * @brief Callback function used to notify applications of display events.
 * 
 * HAL Implementation should call this method to delivery display events
 * to the application (e.g. connected, disconnected).
 *
 * @param handle    Handle of the display device.
 * @param event     Display event.
 * @param eventData Data associated with the event. 
 *                  This is currently a place holder.
 *
 * @note HAL implementation is responsible for allocation of memory needed for event data. 
 * The memory can be free'd after callback returns.
 *
 * @return None.
 */
typedef void (*dsDisplayEventCallback_t)(int handle, dsDisplayEvent_t event, void *eventData/*Optional*/);

/**
 * @brief Initialize the Display sub-system.
 * 
 * This function must initialize all underlying Display sub-system modules and 
 * associated data structures.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE If sucessfully dsDisplayInit api has been called using IARM support.
 * @retval dsERR_GENERAL General failure.
 */
dsError_t dsDisplayInit();

/**
 * @brief Get the handle of a display device.
 * 
 * This function will get the handle for the display device connected to the 
 * specified video port.
 *
 * @param [in]  vType    Type of video port (e.g. HDMI, COMPONENT, etc.).
 * @param [in]  index    The index of the video port (0, 1, ...).
 * @param [out] *handle  The address of a location to hold the handle for the
 *                       specified display device on return. 
 * 
 * @return Device Settings error code
 * @retval dsERR_NONE If sucessfully dsGetDisplay api has been called using IARM support.
 * @retval dsERR_GENERAL General failure.
 */
 dsError_t dsGetDisplay(dsVideoPortType_t vType, int index, int *handle);
  
/** 
 * @brief Get the EDID information from the specified display device.
 * 
 * This function gets the EDID information from the HDMI/DVI display corresponding to
 * the specified display device handle.
 * 
 * @param [in]  handle  Handle of the display device.
 * @param [out] *edid   The address of a location to hold the EDID for the specified
 *                      display device on return.
 * @return Device Settings error code
 * @retval dsERR_NONE If sucessfully dsGetEDID api has been called using IARM support.
 * @retval dsERR_GENERAL General failure.
 */
dsError_t dsGetEDID(int handle, dsDisplayEDID_t *edid);

/**
 * @brief This function is used to get the EDID buffer and length of the connected display.
 *
 * @param[in] handle   Handle for the video display. This must be HDMI output
 *                      handle.
 * @param[out] edid  The EDID raw buffer of the display. The HAL implementation should
 *                      malloc() the buffer and return it to the application. The
 *                      application is required to free() the buffer after using it;
 *                      If HDMI is not connected  no data should be returned,
 *                      and the API returns dsERR_INVALID_STATE.
 * @param[out] length The length of EDID buffer data
 * @return Device Settings error code
 * @retval dsERR_NONE If sucessfully dsGetEDIDBytes api has been called using IARM support.
 * @retval dsERR_GENERAL General failure.
 */
dsError_t dsGetEDIDBytes(int handle, unsigned char **edid, int *length);

/**
 * @brief This function gets the aspect ratio for the dsiaply device.
 * This function returns the aspect ratio of the display corresponding to the specified display device handle.
 *
 * @param[in]  handle         Handle of the display device.
 * @param[out] aspectRatio    The address of a location to hold the current aspect ratio
 *                            for the specified display device on return.
 * @return Device Settings error code
 * @retval dsERR_NONE If sucessfully dsGetDisplayAspectRatio api has been called using IARM support.
 * @retval dsERR_GENERAL General failure.
 */
dsError_t dsGetDisplayAspectRatio(int handle, dsVideoAspectRatio_t *aspectRatio);

/**
 * @brief This function deinitialize the display sub-system.
 * This function deallocates the data structures used within the display module and releases any display specific handles.
 *
 * @return Device Settings error code
 * @retval dsERR_NONE If sucessfully dsDisplayTerm api has been called using IARM support.
 * @retval dsERR_GENERAL General failure.
 */
dsError_t dsDisplayTerm();

/**
 * @brief Register a callback function to listen for display events.
 *
 * This function registers a callback function for display events corresponding to
 * the specified display device handle.
 * 
 * @note Application should install at most one callback function per handle.
 * Multiple listeners are supported at application layer and thus not
 * required in HAL implementation.
 *
 * @param handle Handle of the display device.
 * @param cb     The callback function.
 * @return       Error code.
 * @retval       ::dsError_t 
 */
dsError_t dsRegisterDisplayEventCallback(int handle, dsDisplayEventCallback_t cb);

/**
 * @}
 */
#ifdef __cplusplus
}
#endif
#endif


/** @} */
/** @} */
