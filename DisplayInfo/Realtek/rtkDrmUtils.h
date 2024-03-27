#include <stdio.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

int rtksysfs_get_sysfs_str(const char *path, char *valstr, int size);
int openDefaultDRMDevice(void);

typedef enum _drmConnectorModes {
    drmMode_Unknown = 0,
    drmMode_480i,
    drmMode_480p,
    drmMode_720p,
    drmMode_720p24,
    drmMode_720p25,
    drmMode_720p30,
    drmMode_720p50,
    drmMode_1080i,
    drmMode_1080p,
    drmMode_1080p24,
    drmMode_1080p25,
    drmMode_1080p30,
    drmMode_1080p50,
    drmMode_3840x2160p24,
    drmMode_3840x2160p25,
    drmMode_3840x2160p30,
    drmMode_3840x2160p50,
    drmMode_3840x2160p60,
    drmMode_4096x2160p24,
    drmMode_4096x2160p25,
    drmMode_4096x2160p30,
    drmMode_4096x2160p50,
    drmMode_4096x2160p60,
    drmMode_Max

} drmConnectorModes;

typedef enum {
	rtkERR_NONE = 0,                   /**< No error (success).               */
	rtkERR_GENERAL = 0x1000,           /**< General failure to take action.   */
	rtkERR_INVALID_PARAM,              /**< Invalid input parameter.          */
	rtkERR_INVALID_STATE,              /**< Failure due to invalid state.     */
	rtkERR_OPERATION_NOT_SUPPORTED,    /**< Operation not supported.          */
	rtkERR_UNKNOWN                    /**< Unknown error.                    */
} rtkError_t;
#define DEFUALT_DRM_DEVICE "/dev/dri/card0"
int openDefaultDRMDevice(void);
void closeDefaultDRMDevice(int drmFD);

int getDefaultDRMResolution (drmModeConnector *conn, drmConnectorModes *drmResolution) ;
int getSupportedDRMResolutions (drmModeConnector *conn, drmConnectorModes *drmResolution);
int getCurrentDRMResolution (int drmFD, drmModeRes *res, drmModeConnector *conn, drmConnectorModes *drmResolution);
