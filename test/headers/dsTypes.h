/**
 * This enumeration defines all of the standard video port resolutions that can be used.
 */
typedef enum _dsVideoResolution_t{
    dsVIDEO_PIXELRES_720x480,     /**< 720x480 Resolution.                         */
    dsVIDEO_PIXELRES_720x576,     /**< 720x576 Resolution.                         */
    dsVIDEO_PIXELRES_1280x720,    /**< 1280x720 Resolution.                        */
    dsVIDEO_PIXELRES_1920x1080,   /**< 1920x1080 Resolution.                       */

    dsVIDEO_PIXELRES_3840x2160,   /**< 3840x2160 Resolution.                       */
    dsVIDEO_PIXELRES_4096x2160,   /**< 3840x2160 Resolution.                       */

    dsVIDEO_PIXELRES_MAX         /**< Maximum index for Video ouptut resolutions. */
}dsVideoResolution_t;

/**
 * This enumeration defines all of the standard frame rates at which video may be played out of the video port.
 */
typedef enum _dsVideoFrameRate_t{
    dsVIDEO_FRAMERATE_UNKNOWN,    /**< Unknown frame rate.                       */
    dsVIDEO_FRAMERATE_24,         /**< Played at 24 frames per second.           */
    dsVIDEO_FRAMERATE_25,         /**< Played at 25 frames per second.           */
    dsVIDEO_FRAMERATE_30,         /**< Played at 30 frames per second.           */
    dsVIDEO_FRAMERATE_60,         /**< Played at 60 frames per second.           */
    dsVIDEO_FRAMERATE_23dot98,    /**< Played at 23.98 frames per second.        */
    dsVIDEO_FRAMERATE_29dot97,    /**< Played at 29.97 frames per second.        */
    dsVIDEO_FRAMERATE_50,         /**< Played at 50 frames per second.           */
    dsVIDEO_FRAMERATE_59dot94,    /**< Played at 59.94 frames per second.        */
    dsVIDEO_FRAMERATE_MAX         /**< Maximum index for video frame rates.      */
}dsVideoFrameRate_t;

/**
 * This enumeration defines all of the standard video aspect ratios.
 */
typedef enum _dsVideoAspectRatio_t{
    dsVIDEO_ASPECT_RATIO_4x3,     /**< 4:3 aspect ratio.                      */
    dsVIDEO_ASPECT_RATIO_16x9,    /**< 16:9 aspect ratio.                     */
    dsVIDEO_ASPECT_RATIO_MAX     /**< Maximum index for video aspect ratios. */
}dsVideoAspectRatio_t;

/**
 * This enumeration defines all of the standard video Stereo Scopic modes.
 */
typedef enum _dsVideoStereoScopicMode_t {
    dsVIDEO_SSMODE_UNKNOWN = 0,          /**< Unknown mode.                                */
    dsVIDEO_SSMODE_2D,                   /**< 2D mode.                                     */
    dsVIDEO_SSMODE_3D_SIDE_BY_SIDE,      /**< 3D side by side (L/R) stereo mode.           */
    dsVIDEO_SSMODE_3D_TOP_AND_BOTTOM,    /**< 3D top & bottom stereo mode.                 */
    dsVIDEO_SSMODE_MAX                   /**< Maximum index for video stereoscopic modes.  */
}dsVideoStereoScopicMode_t;

/**
 * @ingroup DSHAL_VIDEOPORT_TYPES
 * @brief Structure that defines video port resolution settings of output video device.
 */
typedef struct _dsVideoPortResolution_t {
    char name[32];                                    /**< Name the resolution (e.g. 480i, 480p, 1080p24).   */
    dsVideoResolution_t  pixelResolution;           /**< The resolution associated with the name.                 */
    dsVideoAspectRatio_t  aspectRatio;              /**< The associated aspect ratio.                             */
    dsVideoStereoScopicMode_t  stereoScopicMode;    /**< The associated stereoscopic mode.                        */
    dsVideoFrameRate_t  frameRate;                  /**< The associated frame rate.                               */
    bool interlaced;                                /**< The associated scan mode(@a true if interlaced, @a false if progressive). */
}dsVideoPortResolution_t;
 
 /** @addtogroup DSHAL_HDMI_IN_TYPES Device Settings HAL HDMI IN Type Definitions
 *  @ingroup DSHAL_HDMI_IN
 *  @{
 */
typedef enum _dsHdmiInPort_t
{
    dsHDMI_IN_PORT_NONE = -1,
    dsHDMI_IN_PORT_0,
    dsHDMI_IN_PORT_1,
    dsHDMI_IN_PORT_2,
    dsHDMI_IN_PORT_MAX
} dsHdmiInPort_t;

/** @addtogroup DSHAL_HDMI_IN_TYPES Device Settings HAL HDMI IN Signal Status Definitions
 *  @ingroup DSHAL_HDMI_IN
 *  @{
 */
typedef enum _dsHdmiInSignalStatus_t
{
    dsHDMI_IN_SIGNAL_STATUS_NONE = -1,
    dsHDMI_IN_SIGNAL_STATUS_NOSIGNAL,
    dsHDMI_IN_SIGNAL_STATUS_UNSTABLE,
    dsHDMI_IN_SIGNAL_STATUS_NOTSUPPORTED,
    dsHDMI_IN_SIGNAL_STATUS_STABLE,
    dsHDMI_IN_SIGNAL_STATUS_MAX
} dsHdmiInSignalStatus_t;

typedef enum tv_hdmi_edid_version_e {
    HDMI_EDID_VER_14 = 0,
    HDMI_EDID_VER_20,
    HDMI_EDID_VER_MAX,
} tv_hdmi_edid_version_t;

 /** @addtogroup DSHAL_HDMI_IN_TYPES Device Settings HAL COMPOSITE IN Type Definitions
 *  @ingroup DSHAL_COMPOSITE_IN
 *  @{
 */
typedef enum _dsCompositeInPort_t
{
    dsCOMPOSITE_IN_PORT_NONE = -1,
    dsCOMPOSITE_IN_PORT_0,
    dsCOMPOSITE_IN_PORT_1,
    dsCOMPOSITE_IN_PORT_MAX
} dsCompositeInPort_t;

