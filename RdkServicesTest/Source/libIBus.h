#pragma once

#include "libIARM.h"

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
    std::string dataLocator;
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
        int max_buffer_duration;
        int buffer_duration;
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

typedef IARM_Result_t (*IARM_BusCall_t) (void *arg);

typedef void (*IARM_EventHandler_t)(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

namespace {
    IARM_EventHandler_t handler_ = nullptr;
    IARM_EventId_t eventId_ = -1;
    std::string ownerName_;
}

// Special method to trigger event
bool triggerEvent(const char *owner, IARM_EventId_t eventId, void *eventData, size_t len) {
    if (handler_) {
        handler_(owner, eventId, eventData, len);
        return true;
    }

    return false;
}

// Methods
IARM_Result_t IARM_Bus_Init(const char *name) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Term(void) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Connect(void) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Disconnect(void) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_GetContext(void **context) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_BroadcastEvent(const char *ownerName, IARM_EventId_t eventId, void *data, size_t len) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_IsConnected(const char *memberName, int *isRegistered) { 
    *isRegistered = 1; 
    return IARM_RESULT_SUCCESS;
}

IARM_Result_t IARM_Bus_RegisterEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
    ownerName_ = ownerName;
    eventId_ = eventId;
    handler_ = handler;

    return IARM_RESULT_SUCCESS; 
}

IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char *ownerName, IARM_EventId_t eventId) { 
    ownerName_.clear();
    eventId_ = -1;
    handler_ = nullptr;

    return IARM_RESULT_SUCCESS; 
}

IARM_Result_t IARM_Bus_RemoveEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_RegisterCall(const char *methodName, IARM_BusCall_t handler) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Call(const char *ownerName, const char *methodName, void *arg, size_t argLen) {
    if ((strcmp(ownerName, IARMBUS_AUDIOCAPTUREMGR_NAME) == 0)) {
        auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
        param->result = IARM_RESULT_SUCCESS;
    }

    return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Call_with_IPCTimeout(const char *ownerName, const char *methodName, void *arg, size_t argLen, int timeout) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_RegisterEvent(IARM_EventId_t maxEventId) { return IARM_RESULT_SUCCESS; }

void IARM_Bus_WritePIDFile(const char *path) {}