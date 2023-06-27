#pragma once

#include <exception>
#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <vector>

typedef enum _dsAudioPortType_t {
    dsAUDIOPORT_TYPE_ID_LR, /**< RCA audio output.                  */
    dsAUDIOPORT_TYPE_HDMI, /**< HDMI audio output.                 */
    dsAUDIOPORT_TYPE_SPDIF, /**< S/PDIF audio output.               */
    dsAUDIOPORT_TYPE_SPEAKER, /**< SPEAKER audio output.             */
    dsAUDIOPORT_TYPE_HDMI_ARC, /** < HDMI ARC/EARC audio output.     */
    dsAUDIOPORT_TYPE_HEADPHONE, /**< 3.5mm headphone jack.           */
    dsAUDIOPORT_TYPE_MAX /**< Maximum index for audio port type. */
} dsAudioPortType_t;

typedef enum _dsAudioFormat_t {
    dsAUDIO_FORMAT_NONE,
    dsAUDIO_FORMAT_PCM,
    dsAUDIO_FORMAT_DOLBY_AC3,
    dsAUDIO_FORMAT_DOLBY_EAC3,
    dsAUDIO_FORMAT_DOLBY_AC4,
    dsAUDIO_FORMAT_DOLBY_MAT,
    dsAUDIO_FORMAT_DOLBY_TRUEHD,
    dsAUDIO_FORMAT_DOLBY_EAC3_ATMOS,
    dsAUDIO_FORMAT_DOLBY_TRUEHD_ATMOS,
    dsAUDIO_FORMAT_DOLBY_MAT_ATMOS,
    dsAUDIO_FORMAT_DOLBY_AC4_ATMOS,
    dsAUDIO_FORMAT_AAC,
    dsAUDIO_FORMAT_VORBIS,
    dsAUDIO_FORMAT_WMA,
    dsAUDIO_FORMAT_UNKNOWN,
    dsAUDIO_FORMAT_MAX /**< Maximum . */
} dsAudioFormat_t;

typedef enum _dsAudioCapabilities_t {
    dsAUDIOSUPPORT_NONE = 0x0,
    dsAUDIOSUPPORT_ATMOS = 0x01,
    dsAUDIOSUPPORT_DD = 0x02,
    dsAUDIOSUPPORT_DDPLUS = 0x04,
    dsAUDIOSUPPORT_DAD = 0x08,
    dsAUDIOSUPPORT_DAPv2 = 0x10,
    dsAUDIOSUPPORT_MS12 = 0x20,
    dsAUDIOSUPPORT_MS12V2 = 0x40,
    dsAUDIOSUPPORT_Invalid = 0x80,
} dsAudioCapabilities_t;

typedef enum _dsAudioARCTypes_t {
    dsAUDIOARCSUPPORT_NONE = 0x0,
    dsAUDIOARCSUPPORT_ARC = 0x01,
    dsAUDIOARCSUPPORT_eARC = 0x02,
} dsAudioARCTypes_t;

typedef enum _dsMS12Capabilities_t {
    dsMS12SUPPORT_NONE = 0x0,
    dsMS12SUPPORT_DolbyVolume = 0x01,
    dsMS12SUPPORT_InteligentEqualizer = 0x02,
    dsMS12SUPPORT_DialogueEnhancer = 0x04,
    dsMS12SUPPORT_Invalid = 0x80,
} dsMS12Capabilities_t;

typedef struct _dsVolumeLeveller_t {
    int mode; // 0 = off, 1= on, 2= auto
    int level;
} dsVolumeLeveller_t;

typedef struct _dsSurroundVirtualizer_t {
    int mode; // 0 = off, 1= on, 2= auto
    int boost;
} dsSurroundVirtualizer_t;

typedef enum StereoMode {
    dsAUDIO_STEREO_UNKNOWN, /**< Unknown mode.                         */
    dsAUDIO_STEREO_MONO = 1, /**< Mono mode.                            */
    dsAUDIO_STEREO_STEREO, /**< Normal stereo mode (L+R).             */
    dsAUDIO_STEREO_SURROUND, /**< Surround mode.                        */
    dsAUDIO_STEREO_PASSTHRU, /**< Passthrough mode.                     */
    dsAUDIO_STEREO_DD, /**< Dolby Digital.                        */
    dsAUDIO_STEREO_DDPLUS, /**< Dolby Digital Plus.                   */
    dsAUDIO_STEREO_MAX /**< Maximum index for audio stereo types. */
} dsAudioStereoMode_t;

typedef enum _dsATMOSCapability_t {
    dsAUDIO_ATMOS_NOTSUPPORTED = 0, /**< ATMOS audio not supported */
    dsAUDIO_ATMOS_DDPLUSSTREAM, /**< can handle dd plus stream which is only way to pass ATMOS metadata */
    dsAUDIO_ATMOS_ATMOSMETADATA, /**< capable of parsing ATMOS metadata */
} dsATMOSCapability_t;

typedef enum _dsHdcpProtocolVersion_t {
    dsHDCP_VERSION_1X = 0, /**< HDCP Protocol version 1.x */
    dsHDCP_VERSION_2X, /**< HDCP Protocol version 2.x */
    dsHDCP_VERSION_MAX /**< Maximum index for HDCP protocol. */
} dsHdcpProtocolVersion_t;

typedef enum _dsHdcpStatus_t {
    dsHDCP_STATUS_UNPOWERED = 0, /**< Connected Sink Device does not support HDCP */
    dsHDCP_STATUS_UNAUTHENTICATED, /**< HDCP Authentication Process is not initiated */
    dsHDCP_STATUS_AUTHENTICATED, /**< HDCP Authentication Process is initiated and Passed */
    dsHDCP_STATUS_AUTHENTICATIONFAILURE, /**< HDCP Authentication Failure or Link Integroty Failure */
    dsHDCP_STATUS_INPROGRESS, /**< HDCP Authentication in Progress */
    dsHDCP_STATUS_PORTDISABLED, /**< HDMI output port disabled */
    dsHDCP_STATUS_MAX /**< Maximum index for HDCP status. */
} dsHdcpStatus_t;

typedef enum _dsVideoResolution_t {
    dsVIDEO_PIXELRES_720x480, /**< 720x480 Resolution.                         */
    dsVIDEO_PIXELRES_720x576, /**< 720x576 Resolution.                         */
    dsVIDEO_PIXELRES_1280x720, /**< 1280x720 Resolution.                        */
    dsVIDEO_PIXELRES_1920x1080, /**< 1920x1080 Resolution.                       */
    dsVIDEO_PIXELRES_3840x2160, /**< 3840x2160 Resolution.                       */
    dsVIDEO_PIXELRES_4096x2160, /**< 3840x2160 Resolution.                       */
    dsVIDEO_PIXELRES_MAX /**< Maximum index for Video ouptut resolutions. */
} dsVideoResolution_t;

typedef enum _dsTVResolution_t {
    dsTV_RESOLUTION_480i = 0x0001, /**< 480i Resolution.                         */
    dsTV_RESOLUTION_480p = 0x0002, /**< 480p Resolution.                         */
    dsTV_RESOLUTION_576i = 0x0004, /**< 576p Resolution.                         */
    dsTV_RESOLUTION_576p = 0x0008, /**< 576p Resolution.                         */
    dsTV_RESOLUTION_720p = 0x0010, /**< 720p Resolution.                         */
    dsTV_RESOLUTION_1080i = 0x0020, /**< 1080i Resolution.                         */
    dsTV_RESOLUTION_1080p = 0x0040, /**< 1080p Resolution.                         */
    dsTV_RESOLUTION_2160p30 = 0x0080, /**< 2160p30 Resolution.                         */
    dsTV_RESOLUTION_2160p60 = 0x0100, /**< 2160p60 Resolution.                         */
} dsTVResolution_t;

typedef enum _dsVideoFrameRate_t {
    dsVIDEO_FRAMERATE_UNKNOWN, /**< Unknown frame rate.                       */
    dsVIDEO_FRAMERATE_24, /**< Played at 24 frames per second.           */
    dsVIDEO_FRAMERATE_25, /**< Played at 25 frames per second.           */
    dsVIDEO_FRAMERATE_30, /**< Played at 30 frames per second.           */
    dsVIDEO_FRAMERATE_60, /**< Played at 60 frames per second.           */
    dsVIDEO_FRAMERATE_23dot98, /**< Played at 23.98 frames per second.        */
    dsVIDEO_FRAMERATE_29dot97, /**< Played at 29.97 frames per second.        */
    dsVIDEO_FRAMERATE_50, /**< Played at 50 frames per second.           */
    dsVIDEO_FRAMERATE_59dot94, /**< Played at 59.94 frames per second.        */
    dsVIDEO_FRAMERATE_MAX /**< Maximum index for video frame rates.      */
} dsVideoFrameRate_t;

typedef enum _dsVideoZoom_t {
    dsVIDEO_ZOOM_UNKNOWN = -1, /**< Unknown mode.                                                                                   */
    dsVIDEO_ZOOM_NONE = 0, /**< Decoder format conversion is inactive.                                                          */
    dsVIDEO_ZOOM_FULL, /**< Full screen (16:9 video is streched to fit 4:3 frame).                                          */
    dsVIDEO_ZOOM_LB_16_9, /**< 16:9 Letterbox (16:9 video is placed in a 4:3 frame with a full letterbox effect).              */
    dsVIDEO_ZOOM_LB_14_9, /**< 14:9 Letterbox (16:9 video is transferred into a 4:3 frame with 14:9 letterbox effect).         */
    dsVIDEO_ZOOM_CCO, /**< Center Cut-out (16:9 video is cut to fill 4:3 frame with its center part).                      */
    dsVIDEO_ZOOM_PAN_SCAN, /**< Pan & Scan (16:9 is cut and panned to fille 4:3 frame, following MPEG pan & scan vectors).      */
    dsVIDEO_ZOOM_LB_2_21_1_ON_4_3, /**< 2.21:1 Letterbox on 4:3 (video is transferred into 4:3 frame with a 2.21:1 letterbox effect).   */
    dsVIDEO_ZOOM_LB_2_21_1_ON_16_9, /**< 2.21:1 Letterbox on 16:9 (video is transferred into 16:9 frame with a 2.21:1 letterbox effect). */
    dsVIDEO_ZOOM_PLATFORM, /**< Control over the decoder format conversions is managed by the platform.                         */
    dsVIDEO_ZOOM_16_9_ZOOM, /**< 16:9 Zoom (4:3 video is zoomed to fill 16:9 frame).                                             */
    dsVIDEO_ZOOM_PILLARBOX_4_3, /**< Pillarbox 4:3 (4:3 video is placed in a 16:9 frame with a pillarbox effect)                     */
    dsVIDEO_ZOOM_WIDE_4_3, /**< Wide 4:3 (4:3 video is stretched to fill 16:9 frame).                                           */
    dsVIDEO_ZOOM_MAX /**< Maximum index for screen zoom modes.                                                            */
} dsVideoZoom_t;

typedef enum _dsVideoAspectRatio_t {
    dsVIDEO_ASPECT_RATIO_4x3, /**< 4:3 aspect ratio.                      */
    dsVIDEO_ASPECT_RATIO_16x9, /**< 16:9 aspect ratio.                     */
    dsVIDEO_ASPECT_RATIO_MAX /**< Maximum index for video aspect ratios. */
} dsVideoAspectRatio_t;

typedef enum _dsVideoStereoScopicMode_t {
    dsVIDEO_SSMODE_UNKNOWN = 0, /**< Unknown mode.                                */
    dsVIDEO_SSMODE_2D, /**< 2D mode.                                     */
    dsVIDEO_SSMODE_3D_SIDE_BY_SIDE, /**< 3D side by side (L/R) stereo mode.           */
    dsVIDEO_SSMODE_3D_TOP_AND_BOTTOM, /**< 3D top & bottom stereo mode.                 */
    dsVIDEO_SSMODE_MAX /**< Maximum index for video stereoscopic modes.  */
} dsVideoStereoScopicMode_t;

typedef struct _dsVideoPortResolution_t {
    char name[32]; /**< Name the resolution (e.g. 480i, 480p, 1080p24).   */
    dsVideoResolution_t pixelResolution; /**< The resolution associated with the name.                 */
    dsVideoAspectRatio_t aspectRatio; /**< The associated aspect ratio.                             */
    dsVideoStereoScopicMode_t stereoScopicMode; /**< The associated stereoscopic mode.                        */
    dsVideoFrameRate_t frameRate; /**< The associated frame rate.                               */
    bool interlaced; /**< The associated scan mode(@a true if interlaced, @a false if progressive). */
} dsVideoPortResolution_t;

typedef enum _dsHDRStandard_t {
    dsHDRSTANDARD_NONE = 0x0,
    dsHDRSTANDARD_HDR10 = 0x01,
    dsHDRSTANDARD_HLG = 0x02,
    dsHDRSTANDARD_DolbyVision = 0x04,
    dsHDRSTANDARD_TechnicolorPrime = 0x08,
    dsHDRSTANDARD_Invalid = 0x80,
} dsHDRStandard_t;

typedef enum _dsSURROUNDMode_t {
    dsSURROUNDMODE_NONE = 0x0,
    dsSURROUNDMODE_DD = 0x1,
    dsSURROUNDMODE_DDPLUS = 0x2,
} dsSURROUNDMode_t;

typedef enum _dsDisplayColorDepth_t {
    dsDISPLAY_COLORDEPTH_UNKNOWN = 0x0, /* Unknown color depth */
    dsDISPLAY_COLORDEPTH_8BIT = 0x01, /* 8 bit color depth */
    dsDISPLAY_COLORDEPTH_10BIT = 0x02, /* 10 bit color depth */
    dsDISPLAY_COLORDEPTH_12BIT = 0x04, /* 12 bit color depth */
    dsDISPLAY_COLORDEPTH_AUTO = 0x08 /* Automatic color depth */
} dsDisplayColorDepth_t;

typedef enum _dsHdmiInPort_t {
    dsHDMI_IN_PORT_NONE = -1,
    dsHDMI_IN_PORT_0,
    dsHDMI_IN_PORT_1,
    dsHDMI_IN_PORT_2,
    dsHDMI_IN_PORT_MAX
} dsHdmiInPort_t;

typedef enum _dsHdmiInSignalStatus_t {
    dsHDMI_IN_SIGNAL_STATUS_NONE = -1,
    dsHDMI_IN_SIGNAL_STATUS_NOSIGNAL,
    dsHDMI_IN_SIGNAL_STATUS_UNSTABLE,
    dsHDMI_IN_SIGNAL_STATUS_NOTSUPPORTED,
    dsHDMI_IN_SIGNAL_STATUS_STABLE,
    dsHDMI_IN_SIGNAL_STATUS_MAX
} dsHdmiInSignalStatus_t;

typedef enum dsAviContentType {
  dsAVICONTENT_TYPE_GRAPHICS,
  dsAVICONTENT_TYPE_PHOTO,
  dsAVICONTENT_TYPE_CINEMA,
  dsAVICONTENT_TYPE_GAME,
  dsAVICONTENT_TYPE_INVALID,
}dsAviContentType_t;

struct dsSpd_infoframe_st {
    uint8_t pkttype;
    uint8_t version;
    uint8_t length; /*length=25*/
    uint8_t rsd;
    uint8_t checksum;
    /*Vendor Name Character*/
    uint8_t vendor_name[8];
    /*Product Description Character*/
    uint8_t product_des[16];
    /*byte 25*/
    uint8_t source_info;
};

typedef enum tv_hdmi_edid_version_e {
    HDMI_EDID_VER_14 = 0,
    HDMI_EDID_VER_20,
    HDMI_EDID_VER_MAX,
} tv_hdmi_edid_version_t;

typedef enum _dsAudioPortState {
    dsAUDIOPORT_STATE_UNINITIALIZED,
    dsAUDIOPORT_STATE_INITIALIZED,
    dsAUDIOPORT_STATE_MAX
} dsAudioPortState_t;

typedef enum {
    dsERR_NONE = 0, /**< No error (success).               */
    dsERR_GENERAL = 0x1000, /**< General failure to take action.   */
    dsERR_INVALID_PARAM, /**< Invalid input parameter.          */
    dsERR_INVALID_STATE, /**< Failure due to invalid state.     */
    dsERR_OPERATION_NOT_SUPPORTED, /**< Operation not supported.          */
    dsERR_UNKNOWN /**< Unknown error.                    */
} dsError_t;

#define MAX_LANGUAGE_LEN 10

typedef enum _dsSleepMode_t {
    dsHOST_SLEEP_MODE_LIGHT, /**< Light sleep mode.                                */
    dsHOST_SLEEP_MODE_DEEP, /**< Deep sleep mode.                                 */
    dsHOST_SLEEP_MODE_MAX, /**< Maximum index for sleep modes                    */
} dsSleepMode_t;

typedef enum __dsFPDTimeFormat_t {
    dsFPD_TIME_12_HOUR, /**< 12 hour time format. */
    dsFPD_TIME_24_HOUR, /**< 24 hour time format. */
    dsFPD_TIME_STRING /**< Text string.         */
} dsFPDTimeFormat_t;

typedef enum _dsCompInSignalStatus_t {
    dsCOMP_IN_SIGNAL_STATUS_NONE = -1,
    dsCOMP_IN_SIGNAL_STATUS_NOSIGNAL,
    dsCOMP_IN_SIGNAL_STATUS_UNSTABLE,
    dsCOMP_IN_SIGNAL_STATUS_NOTSUPPORTED,
    dsCOMP_IN_SIGNAL_STATUS_STABLE,
    dsCOMP_IN_SIGNAL_STATUS_MAX
} dsCompInSignalStatus_t;

typedef enum _dsCompositeInPort_t {
    dsCOMPOSITE_IN_PORT_NONE = -1,
    dsCOMPOSITE_IN_PORT_0,
    dsCOMPOSITE_IN_PORT_1,
    dsCOMPOSITE_IN_PORT_MAX
} dsCompositeInPort_t;

/*! DS Manager  Event Data */
typedef struct _DSMgr_EventData_t {
    union {
        struct _RESOLUTION_DATA {
            /* Declare Event Data structure for Video resolution Event */
            int width; /*!< Resolution Width  */
            int height; /*!< Key code */
        } resn; /*Reolution data*/
        struct _DFC_DATA {
            /* Declare Event Data structure for Zoom settings Event */
            int zoomsettings;
        } dfc; /*zoom data*/

        struct _AUDIOMODE_DATA {
            /* Declare Event Data structure for Video resolution Event */
            int type; /*!< device type  */
            int mode; /*!< device mode */
        } Audioport; /*Audio mode  data*/

        struct _HDMI_HPD_DATA {
            /* Declare HDMI HPD Data */
            int event;
        } hdmi_hpd; /*HDMI Hot Plug detect*/

        struct _HDMI_HDCP_DATA {
            /* Declare HDMI DCP Data */
            int hdcpStatus;
        } hdmi_hdcp; /*HDMI HDCP Hot Plug detect*/

        struct _HDMI_RXSENSE_DATA {
            /* Declare HDMI Rx Sense status */
            int status;
        } hdmi_rxsense; /*HDMI Rx Sense Data*/

        struct _HDMI_IN_CONNECT_DATA {
            dsHdmiInPort_t port;
            bool isPortConnected;
        } hdmi_in_connect;

        struct _HDMI_IN_STATUS_DATA {
            /* Declare HDMI Input status*/
            dsHdmiInPort_t port;
            bool isPresented;
        } hdmi_in_status; /*HDMI in status change detect*/

        struct _HDMI_IN_SIG_STATUS_DATA {
            /* Declare HDMI In signal status*/
            dsHdmiInPort_t port;
            dsHdmiInSignalStatus_t status;
        } hdmi_in_sig_status; /*HDMI in signal change detect*/

        struct _HDMI_IN_VIDEO_MODE_DATA {
            /* Declare HDMI In signal status*/
            dsHdmiInPort_t port;
            dsVideoPortResolution_t resolution;
        } hdmi_in_video_mode; /*HDMI in video mode update*/

        struct _COMPOSITE_IN_CONNECT_DATA {
            dsCompositeInPort_t port;
            bool isPortConnected;
        } composite_in_connect;

        struct _COMPOSITE_IN_STATUS_DATA {
            /* Declare Composite Input status*/
            dsCompositeInPort_t port;
            bool isPresented;
        } composite_in_status; /*Composite in status change detect*/

        struct _COMPOSITE_IN_SIG_STATUS_DATA {
            /* Declare Composite In signal status*/
            dsCompositeInPort_t port;
            dsCompInSignalStatus_t status;
        } composite_in_sig_status; /*Composite in signal change detect*/

        struct _FPD_TIME_FORMAT {
            dsFPDTimeFormat_t eTimeFormat;
        } FPDTimeFormat;

        struct _HDCP_PROTOCOL_DATA {
            dsHdcpProtocolVersion_t protocolVersion;
        } HDCPProtocolVersion;
        struct _SLEEP_MODE_DATA {
            dsSleepMode_t sleepMode;
        } sleepModeInfo;

        struct _AUDIO_LEVEL_DATA {
            int level;
        } AudioLevelInfo;

        struct _AUDIO_OUT_CONNECT_DATA {
            dsAudioPortType_t portType;
            unsigned int uiPortNo;
            bool isPortConnected;
        } audio_out_connect;

        struct _AUDIO_FORMAT_DATA {
            dsAudioFormat_t audioFormat;
        } AudioFormatInfo;

        struct _LANGUAGE_DATA {
            char audioLanguage[MAX_LANGUAGE_LEN];
        } AudioLanguageInfo;

        struct _FADER_CONTROL_DATA {
            int mixerbalance;
        } FaderControlInfo;

        struct _ASSOCIATED_AUDIO_MIXING_DATA {
            bool mixing;
        } AssociatedAudioMixingInfo;

        struct _VIDEO_FORMAT_DATA {
            dsHDRStandard_t videoFormat;
        } VideoFormatInfo;

        struct _AUDIO_PORTSTATE_DATA {
            dsAudioPortState_t audioPortState;
        } AudioPortStateInfo;

        struct _HDMI_IN_ALLM_MODE_DATA {
            /* Declare HDMI In ALLM Mode*/
            dsHdmiInPort_t port;
            bool allm_mode;
        } hdmi_in_allm_mode; /*HDMI in ALLM Mode change*/
        struct _HDMI_IN_CONTENT_TYPE_DATA{
            dsHdmiInPort_t port;
            dsAviContentType_t aviContentType;
        }hdmi_in_content_type;
    } data;
} IARM_Bus_DSMgr_EventData_t;

typedef enum _dsDisplayEvent_t {
    dsDISPLAY_EVENT_CONNECTED = 0, //!< Display connected event.
    dsDISPLAY_EVENT_DISCONNECTED, //!< Display disconnected event.
    dsDISPLAY_RXSENSE_ON, //!< Rx Sense ON event
    dsDISPLAY_RXSENSE_OFF, //!< Rx Sense OFF event
    dsDISPLAY_HDCPPROTOCOL_CHANGE, //!< HDCP Protocol Version Change event
    dsDISPLAY_EVENT_MAX
} dsDisplayEvent_t;

namespace device {

template <class T>
using List = std::vector<T>;

}

namespace device {

class Exception : public std::exception {
    int _err;
    std::string _msg;

public:
    Exception(const char* msg = "No Message for this exception") throw()
        : _msg(msg)
    {
    }

    Exception(int err, const char* msg = "No Message for this Exception") throw()
        : _err(err)
        , _msg(msg){};

    virtual const std::string& getMessage() const
    {
        return _msg;
    }

    virtual int getCode() const
    {
        return _err;
    }

    virtual const char* what() const throw()
    {
        return _msg.c_str();
    }

    virtual ~Exception() throw(){};
};

}

namespace device {

class AudioOutputPortImpl {
public:
    virtual ~AudioOutputPortImpl() = default;

    virtual const std::string& getName() const = 0;
    virtual std::vector<std::string> getMS12AudioProfileList() const = 0;
    virtual void getAudioCapabilities(int* capabilities) = 0;
    virtual void getMS12Capabilities(int* capabilities) = 0;
    virtual bool isAudioMSDecode() const = 0;
};

class AudioOutputPort {
public:
    AudioOutputPortImpl* impl;

    const std::string& getName() const
    {
        return impl->getName();
    }

    std::vector<std::string> getMS12AudioProfileList() const
    {
        return impl->getMS12AudioProfileList();
    }

    void getAudioCapabilities(int* capabilities)
    {
        return impl->getAudioCapabilities(capabilities);
    }

    void getMS12Capabilities(int* capabilities)
    {
        return impl->getMS12Capabilities(capabilities);
    }

    bool isAudioMSDecode()
    {
        return impl->isAudioMSDecode();
    }
};

}

namespace device {
class CompositeInputImpl {
public:
    virtual ~CompositeInputImpl() = default;
    virtual uint8_t getNumberOfInputs() const = 0;
    virtual bool isPortConnected(int8_t Port) const = 0;
    virtual void selectPort(int8_t Port) const = 0;
    virtual void scaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) const = 0;
};

class CompositeInput {
public:
    static CompositeInput& getInstance()
    {
        static CompositeInput instance;
        return instance;
    }

    CompositeInputImpl* impl;

    uint8_t getNumberOfInputs() const
    {
        return impl->getNumberOfInputs();
    }
    bool isPortConnected(int8_t Port) const
    {
        return impl->isPortConnected(Port);
    }
    void selectPort(int8_t Port) const
    {
        return impl->selectPort(Port);
    }
    void scaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) const
    {
        return impl->scaleVideo(x, y, width, height);
    }
};

}

namespace device {

class HdmiInputImpl {
public:
    virtual ~HdmiInputImpl() = default;

    virtual uint8_t getNumberOfInputs() const = 0;
    virtual bool isPortConnected(int8_t Port) const = 0;
    virtual std::string getCurrentVideoMode() const = 0;
    virtual void selectPort(int8_t Port) const = 0;
    virtual void scaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) const = 0;

    virtual void getEDIDBytesInfo(int iHdmiPort, std::vector<uint8_t>& edid) const = 0;
    virtual void getHDMISPDInfo(int iHdmiPort, std::vector<uint8_t>& data) const = 0;
    virtual void setEdidVersion(int iHdmiPort, int iEdidVersion) const = 0;
    virtual void getEdidVersion(int iHdmiPort, int* iEdidVersion) const = 0;
    virtual void getHdmiALLMStatus(int iHdmiPort, bool* allmStatus) const = 0;
    virtual void getSupportedGameFeatures(std::vector<std::string>& featureList) const = 0;
};

class HdmiInput {
public:
    static HdmiInput& getInstance()
    {
        static HdmiInput instance;
        return instance;
    }

    HdmiInputImpl* impl;

    uint8_t getNumberOfInputs() const
    {
        return impl->getNumberOfInputs();
    }
    bool isPortConnected(int8_t Port) const
    {
        return impl->isPortConnected(Port);
    }
    std::string getCurrentVideoMode() const
    {
        return impl->getCurrentVideoMode();
    }
    void selectPort(int8_t Port) const
    {
        return impl->selectPort(Port);
    }
    void scaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) const
    {
        return impl->scaleVideo(x, y, width, height);
    }
    void getEDIDBytesInfo(int iHdmiPort, std::vector<uint8_t>& edid) const
    {
        return impl->getEDIDBytesInfo(iHdmiPort, edid);
    }
    void getHDMISPDInfo(int iHdmiPort, std::vector<uint8_t>& data) const
    {
        return impl->getHDMISPDInfo(iHdmiPort, data);
    }
    void setEdidVersion(int iHdmiPort, int iEdidVersion) const
    {
        return impl->setEdidVersion(iHdmiPort, iEdidVersion);
    }
    void getEdidVersion(int iHdmiPort, int* iEdidVersion) const
    {
        return impl->getEdidVersion(iHdmiPort, iEdidVersion);
    }

    void getHdmiALLMStatus(int iHdmiPort, bool* allmStatus) const
    {
        return impl->getHdmiALLMStatus(iHdmiPort, allmStatus);
    }
    void getSupportedGameFeatures(std::vector<std::string>& featureList) const
    {
        return impl->getSupportedGameFeatures(featureList);
    }
};

}

namespace device {
class SleepMode;
class SleepModeImpl {
public:
    virtual ~SleepModeImpl() = default;
    virtual SleepMode & getInstanceById(int id) = 0;
    virtual SleepMode & getInstanceByName(const std::string &name) = 0;
    virtual List<SleepMode> getSleepModes() = 0;
    virtual const std::string& toString() const = 0;
};

class SleepMode {
public:
    SleepModeImpl* impl;

    static SleepMode& getInstance()
    {
        static SleepMode instance;
        return instance;
    }

    static SleepMode& getInstance(int id)
    {
        return getInstance().impl->getInstanceById(id);
    }

    static SleepMode& getInstance(const std::string &name)
    {
        return getInstance().impl->getInstanceByName(name);
    }

    List<SleepMode> getSleepModes()
    {
        return impl->getSleepModes();
    }

    const std::string& toString() const
    {
        return impl->toString();
    }
};
}

namespace device {

class VideoDeviceImpl {
public:
    virtual ~VideoDeviceImpl() = default;

    virtual int getFRFMode(int* frfmode) const = 0;
    virtual int setFRFMode(int frfmode) const = 0;
    virtual int getCurrentDisframerate(char* framerate) const = 0;
    virtual int setDisplayframerate(const char* framerate) const = 0;
};

class VideoDevice {
public:
    VideoDeviceImpl* impl;

    int getFRFMode(int* frfmode) const
    {
        return impl->getFRFMode(frfmode);
    }

    int setFRFMode(int frfmode) const
    {
        return impl->setFRFMode(frfmode);
    }

    int getCurrentDisframerate(char* framerate) const
    {
        return impl->getCurrentDisframerate(framerate);
    }

    int setDisplayframerate(const char* framerate) const
    {
        return impl->setDisplayframerate(framerate);
    }
};

}

namespace device {

class VideoResolutionImpl {
public:
    virtual ~VideoResolutionImpl() = default;

    virtual const std::string& getName() const = 0;
};

class VideoResolution {
public:
    VideoResolutionImpl* impl;

    const std::string& getName() const
    {
        return impl->getName();
    }
};

}

namespace device {

class VideoOutputPortTypeImpl {
public:
    virtual ~VideoOutputPortTypeImpl() = default;

    virtual int getId() const = 0;
    virtual const List<VideoResolution> getSupportedResolutions() const = 0;
};

class VideoOutputPortType {
public:
    VideoOutputPortTypeImpl* impl;

    int getId() const
    {
        return impl->getId();
    }

    const List<VideoResolution> getSupportedResolutions() const
    {
        return impl->getSupportedResolutions();
    }
};

}

namespace device {

class VideoOutputPortImpl {
public:
    virtual ~VideoOutputPortImpl() = default;

    virtual const VideoOutputPortType& getType() const = 0;
    virtual const std::string& getName() const = 0;
    virtual const VideoResolution& getDefaultResolution() const = 0;
    virtual int getHDCPProtocol() = 0;
    virtual int getHDCPReceiverProtocol() = 0;
    virtual int getHDCPCurrentProtocol() = 0;
    virtual int getHDCPStatus() = 0;
    virtual AudioOutputPort& getAudioOutputPort() const = 0;
    virtual bool isDisplayConnected() = 0;
    virtual bool isContentProtected() = 0;
};

class VideoOutputPort {
public:
    VideoOutputPortImpl* impl;

    const VideoOutputPortType& getType() const
    {
        return impl->getType();
    }

    const std::string& getName() const
    {
        return impl->getName();
    }

    const VideoResolution& getDefaultResolution() const
    {
        return impl->getDefaultResolution();
    }

    int getHDCPProtocol()
    {
        return impl->getHDCPProtocol();
    }

    int getHDCPReceiverProtocol()
    {
        return impl->getHDCPReceiverProtocol();
    }

    int getHDCPCurrentProtocol()
    {
        return impl->getHDCPCurrentProtocol();
    }

    int getHDCPStatus()
    {
        return impl->getHDCPStatus();
    }

    AudioOutputPort& getAudioOutputPort()
    {
        return impl->getAudioOutputPort();
    }

    bool isDisplayConnected()
    {
        return impl->isDisplayConnected();
    }

    bool isContentProtected()
    {
        return impl->isContentProtected();
    }
};

}

namespace device {

class VideoOutputPortConfigImpl {
public:
    virtual ~VideoOutputPortConfigImpl() = default;

    virtual VideoOutputPortType& getPortType(int id) = 0;
    virtual VideoOutputPort& getPort(const std::string& name) = 0;
};

class VideoOutputPortConfig {
public:
    static VideoOutputPortConfig& getInstance()
    {
        static VideoOutputPortConfig instance;
        return instance;
    }

    VideoOutputPortConfigImpl* impl;

    VideoOutputPortType& getPortType(int id)
    {
        return impl->getPortType(id);
    }

    VideoOutputPort& getPort(const std::string& name)
    {
        return impl->getPort(name);
    }
};

}

class ManagerImpl {
public:
    virtual ~ManagerImpl() = default;

    virtual void Initialize() = 0;
    virtual void DeInitialize() = 0;
};

namespace device {
class Manager {
public:
    static Manager& getInstance()
    {
        static Manager instance;
        return instance;
    }

    ManagerImpl* impl;

    static void Initialize()
    {
        return getInstance().impl->Initialize();
    }

    static void DeInitialize()
    {
        return getInstance().impl->DeInitialize();
    }
};
}

namespace device {

class HostImpl {
public:
    virtual ~HostImpl() = default;
    virtual SleepMode getPreferredSleepMode() = 0;
    virtual int setPreferredSleepMode(const SleepMode mode) = 0;
    virtual List<SleepMode> getAvailableSleepModes() = 0;
    virtual List<VideoOutputPort> getVideoOutputPorts() = 0;
    virtual List<AudioOutputPort> getAudioOutputPorts() = 0;
    virtual List<VideoDevice> getVideoDevices() = 0;
    virtual VideoOutputPort& getVideoOutputPort(const std::string& name) = 0;
    virtual AudioOutputPort& getAudioOutputPort(const std::string& name) = 0;
    virtual void getHostEDID(std::vector<uint8_t>& edid) const = 0;
    virtual std::string getDefaultVideoPortName() = 0;
    virtual std::string getDefaultAudioPortName() = 0;
};

class Host {
public:
    static Host& getInstance()
    {
        static Host instance;
        return instance;
    }

    HostImpl* impl;

    SleepMode getPreferredSleepMode()
    {
        return impl->getPreferredSleepMode();
    }

    int setPreferredSleepMode(const SleepMode mode)
    {
        return impl->setPreferredSleepMode(mode);
    }

    List<SleepMode> getAvailableSleepModes()
    {
        return impl->getAvailableSleepModes();
    }

    List<VideoOutputPort> getVideoOutputPorts()
    {
        return impl->getVideoOutputPorts();
    }

    List<AudioOutputPort> getAudioOutputPorts()
    {
        return impl->getAudioOutputPorts();
    }

    List<VideoDevice> getVideoDevices()
    {
        return impl->getVideoDevices();
    }

    VideoOutputPort& getVideoOutputPort(const std::string& name)
    {
        return impl->getVideoOutputPort(name);
    }

    AudioOutputPort& getAudioOutputPort(const std::string& name)
    {
        return impl->getAudioOutputPort(name);
    }

    void getHostEDID(std::vector<uint8_t>& edid) const
    {
        return impl->getHostEDID(edid);
    }

    std::string getDefaultVideoPortName()
    {
        return impl->getDefaultVideoPortName();
    }

    std::string getDefaultAudioPortName()
    {
        return impl->getDefaultAudioPortName();
    }
};

}

namespace device {
class FrontPanelIndicator {
public:
    class Color;
    class ColorImpl {
    public:
        virtual ~ColorImpl() = default;
        virtual const Color& getInstanceById(int id) = 0;
        virtual const Color& getInstanceByName(const std::string& name) = 0;
    };

    class Color {
    public:
        static Color& getInstance()
        {
            static Color instance;
            return instance;
        }

        ColorImpl* impl;

        static const Color& getInstance(int id)
        {
            return getInstance().impl->getInstanceById(id);
        }
        static const Color& getInstance(const std::string& name)
        {
            return getInstance().impl->getInstanceByName(name);
        }
    };

    static FrontPanelIndicator& getInstance()
    {
        static FrontPanelIndicator instance;
        return instance;
    }

    class FrontPanelIndicatorImpl {
    public:
        virtual ~FrontPanelIndicatorImpl() = default;
        virtual FrontPanelIndicator& getInstance(const std::string& name) = 0;
        virtual void setState(const bool bState) const = 0;
        virtual const std::string& getName() const = 0;
        virtual void setBrightness(const int& brightness, const bool toPersist) = 0;
        virtual int getBrightness() = 0;
        virtual void setColor(const Color& newColor, bool toPersist) = 0;
        virtual void setColor(const uint32_t color, const bool toPersist) = 0;
    };

    FrontPanelIndicatorImpl* impl;

    static FrontPanelIndicator& getInstance(const std::string& name)
    {
        return getInstance().impl->getInstance(name);
    }

    void setState(const bool bState) const
    {
        return impl->setState(bState);
    }

    const std::string & getName() const
    {
        return impl->getName();
    }
    void setBrightness(const int &brightness, const bool toPersist = true)
    {
        return impl->setBrightness(brightness, toPersist);
    }
    int getBrightness()
    {
        return impl->getBrightness();
    }
    void setColor(const Color & newColor,bool toPersist = true)
    {
        return impl->setColor(newColor, toPersist);
    }
    void setColor(const uint32_t color,const bool toPersist = true)
    {
        return impl->setColor(color, toPersist);
    }
};

class FrontPanelTextDisplay;
class FrontPanelTextDisplayImpl {
public:
    virtual ~FrontPanelTextDisplayImpl() = default;
    virtual int getCurrentTimeFormat() = 0;
    virtual void setTimeFormat(const int iTimeFormat) = 0;
};

class FrontPanelTextDisplay {
public:
    static const int kModeClock12Hr = dsFPD_TIME_12_HOUR;
    static const int kModeClock24Hr = dsFPD_TIME_24_HOUR;

    FrontPanelTextDisplayImpl* impl;

    int getCurrentTimeFormat()
    {
        return impl->getCurrentTimeFormat();
    }
    void setTimeFormat(const int iTimeFormat)
    {
        impl->setTimeFormat(iTimeFormat);
    }
};

class FrontPanelConfig;
class FrontPanelConfigImpl {
public:
    virtual ~FrontPanelConfigImpl() = default;
    virtual List<FrontPanelIndicator> getIndicators() = 0;
    virtual FrontPanelTextDisplay& getTextDisplay(const std::string &name) = 0;
};

class FrontPanelConfig {
public:
    FrontPanelConfigImpl* impl;

    static FrontPanelConfig& getInstance()
    {
        static FrontPanelConfig instance;
        return instance;
    }
    List<FrontPanelIndicator> getIndicators()
    {
        return impl->getIndicators();
    }
    FrontPanelTextDisplay& getTextDisplay(const std::string &name)
    {
        return impl->getTextDisplay(name);
    }
};

}

