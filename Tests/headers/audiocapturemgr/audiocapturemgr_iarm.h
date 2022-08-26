#pragma once

#include <string>

#define IARMBUS_AUDIOCAPTUREMGR_NAME "audiocapturemgr"
#define IARMBUS_AUDIOCAPTUREMGR_ENABLE "enableCapture"
#define IARMBUS_AUDIOCAPTUREMGR_REQUEST_SAMPLE "requestSample"

#define IARMBUS_AUDIOCAPTUREMGR_OPEN "open"
#define IARMBUS_AUDIOCAPTUREMGR_CLOSE "close"
#define IARMBUS_AUDIOCAPTUREMGR_START "start"
#define IARMBUS_AUDIOCAPTUREMGR_STOP "stop"
#define IARMBUS_AUDIOCAPTUREMGR_GET_DEFAULT_AUDIO_PROPS "getDefaultAudioProperties"
#define IARMBUS_AUDIOCAPTUREMGR_GET_AUDIO_PROPS "getAudioProperties"
#define IARMBUS_AUDIOCAPTUREMGR_GET_OUTPUT_PROPS "getOutputProperties"
#define IARMBUS_AUDIOCAPTUREMGR_SET_AUDIO_PROPERTIES "setAudioProperties"
#define IARMBUS_AUDIOCAPTUREMGR_SET_OUTPUT_PROPERTIES "setOutputProperties"

#define AUDIOCAPTUREMGR_FILENAME_PREFIX "audio_sample"
#define AUDIOCAPTUREMGR_FILE_PATH "/opt/"

#define DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY 100

#define ACM_RESULT_GENERAL_FAILURE 0
#define ACM_RESULT_PRECAPTURE_NOT_SUPPORTED 1
#define ACM_RESULT_STREAM_UNAVAILABLE 2
#define ACM_RESULT_DURATION_OUT_OF_BOUNDS 3
#define BUFFERED_FILE_OUTPUT ""

enum Format {
    acmFormate16BitStereo,
    acmFormate16BitMonoLeft,
    acmFormate16BitMonoRight,
    acmFormate16BitMono,
    acmFormate24BitStereo,
    acmFormate24Bit5_1
};

enum Sampling_frequency {
    acmFreqe48000,
    acmFreqe44100,
    acmFreqe32000,
    acmFreqe24000,
    acmFreqe16000
};

namespace audiocapturemgr
{
    using session_id_t = int;

    struct audio_properties_ifce_t {
        Format format;
        Sampling_frequency sampling_frequency;
    };
}

struct iarmbus_notification_payload_t {
    char dataLocator[64];
};

struct iarmbus_acm_arg_t {
    struct iarmbus_open_args {
        int	source;
        std::string output_type;
    };

    struct iarmbus_request_payload_t {
        float duration;
        bool is_precapture;
    };

    struct output_t {
        unsigned int buffer_duration;
        unsigned int max_buffer_duration;
    };

    struct arg_output_props_t {
        output_t output;
    };

    struct Details {
        iarmbus_open_args arg_open;
        audiocapturemgr::audio_properties_ifce_t arg_audio_properties;
        iarmbus_request_payload_t arg_sample_request;
        arg_output_props_t arg_output_props;
    };

    int session_id;
    int result;
    Details details;
};
