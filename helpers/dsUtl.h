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
 * @file
 *
 * @brief Device Settings HAL utilities.
 *
 * This API defines some common utilities for the Device Settings HAL.
 *
 * @par Document
 * Document reference.
 *
 * @par Open Issues (in no particular order)
 * -# None
 *
 * @par Assumptions
 * -# None
 *
 * @par Abbreviations
 * - BE:      Big-Endian.
 * - cb:      Callback function (suffix).
 * - DS:      Device Settings.
 * - FPD:     Front-Panel Display.
 * - HAL:     Hardware Abstraction Layer.
 * - LE:      Little-Endian.
 * - LS:      Least Significant.
 * - MBZ:     Must be zero.
 * - MS:      Most Significant.
 * - RDK:     Reference Design Kit.
 * - _t:      Type (suffix).
 *
 * @par Implementation Notes
 * -# None
 *
 */



/**
* @defgroup devicesettings
* @{
* @defgroup hal
* @{
**/


#ifndef _DS_UTILS_H_
#define _DS_UTILS_H_

#include <stdio.h>

/** @addtogroup DSHAL_UTILS Device Settings HAL Utilities
 *  @ingroup DSHAL_API
 *  @{
 */

/**
 * @brief Device Settings general Assert Failure inline definition.
 */
#define dsUTL_ASSERT(x) do {\
	if (!(x)) printf("ASSERTFAILURE at %s-%d\r\n", __FUNCTION__, __LINE__);\
}\
while(0)

/**
 * @brief Device Settings general Array dimension calculation inline definition.
 */
#define dsUTL_DIM(arr) (sizeof((arr))/sizeof((arr)[0]))

/* End of DSHAL_UTILS doxygen group */
/**
 * @}
 */

#endif /* _DS_UTILS_H_ */


/** @} */
/** @} */
