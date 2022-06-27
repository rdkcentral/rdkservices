/**
 * @file
 *
 * @brief Device Settings HAL error codes.
 *
 * This API defines common error codes used by the Device Settings HAL.
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


#ifndef _DS_ERROR_H_ 
#define _DS_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup DSHAL_ERROR Device Settings HAL Error Codes
*  @ingroup DSHAL_API
*
*  Defined hereunder are the error codes that are used in the Device Settings library.
*  @{
*/

/** 
 * @brief Device Settings API Error return codes.
 * 
 * A non-zero value indicates an error.
 * 
 */
typedef enum {
    dsERR_NONE = 0,                   /**< No error (success).               */
    dsERR_GENERAL = 0x1000,           /**< General failure to take action.   */
    dsERR_INVALID_PARAM,              /**< Invalid input parameter.          */
    dsERR_INVALID_STATE,              /**< Failure due to invalid state.     */
    dsERR_OPERATION_NOT_SUPPORTED,    /**< Operation not supported.          */
    dsERR_UNKNOWN                    /**< Unknown error.                    */
} dsError_t;

/* End of DSHAL_ERROR doxygen group */
/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* _DS_ERROR_H_ */


/** @} */
/** @} */
