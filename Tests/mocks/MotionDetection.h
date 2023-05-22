#pragma once

#include <exception>
#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <vector>

#define MOTION_DETECTION_LEN_MAX                 32
#define MOTION_DETECTION_SENSITIVITY_IDENTIFIERS 3
#define SENSITIVITY_MODE_INT                     1
#define SENSITIVITY_MODE_LEVELS                  2
#define STR_SENSITIVITY_LOW                      "low"
#define STR_SENSITIVITY_MEDIUM                   "medium"
#define STR_SENSITIVITY_HIGH                     "high"
#define STR_SENSITIVITY_MIN                      "0"
#define STR_SENSITIVITY_MAX                      "100"
#define STR_SENSITIVITY_STEP                     "20"
#define SENSITIVITY_IDENTIFIER_1                 0
#define SENSITIVITY_IDENTIFIER_2                 1
#define SENSITIVITY_IDENTIFIER_3                 2
#define SENSITIVITY_IDENTIFIERS                  3

#define EVENT_TYPE_MOTION_DETECTED               '1'
#define EVENT_TYPE_MOTION_DISAPEARED             '0'

#define MOTION_DETECTOR                          "FP_MD"
#define MOTION_DETECTOR_TYPE                     "PID"
#define MOTION_DETECTION_DESCRIPTION             "The only motion detector"
#define MOTION_DETECTION_ANGLE                   74
#define MOTION_DETECTION_DISTANCE                6000

typedef enum _MOTION_DETECTION_Result_t {
    MOTION_DETECTION_RESULT_SUCCESS,
    MOTION_DETECTION_RESULT_GENERIC_FAILURE,
    MOTION_DETECTION_RESULT_INTI_FAILURE,
    MOTION_DETECTION_RESULT_MODE_ERROR,
    MOTION_DETECTION_RESULT_INDEX_ERROR,
    MOTION_DETECTION_RESULT_SENSITIVITY_RANGE_ERROR,
    MOTION_DETECTION_RESULT_WRONG_INPUT_ERROR,
    MOTION_DETECTION_RESULT_SET_SENSITIVITY_ERROR,
} MOTION_DETECTION_Result_t;

typedef enum _MOTION_DETECTION_Mode_t {
    MOTION_DETECTION_SENSITIVITY_Mode_0, // No Motion Detected
    MOTION_DETECTION_SENSITIVITY_Mode_1, // Motion Detected
} MOTION_DETECTION_Mode_t;

typedef struct _MOTION_DETECTION_EventMessage_t {
    char m_eventType;
    char m_sensorIndex[MOTION_DETECTION_LEN_MAX];
} MOTION_DETECTION_EventMessage_t;

typedef struct _MOTION_DETECTION_CurrentSensorSettings_t {
    unsigned int m_sensitivityMode;
    char         m_sensorIndex[MOTION_DETECTION_LEN_MAX];
    char         m_sensorDescription[MOTION_DETECTION_LEN_MAX];
    char         m_sensorType[MOTION_DETECTION_LEN_MAX];
    unsigned int m_sensorDistance;
    unsigned int m_sensorAngle;
    char         m_sensitivity[MOTION_DETECTION_SENSITIVITY_IDENTIFIERS][MOTION_DETECTION_LEN_MAX];

} MOTION_DETECTION_CurrentSensorSettings_t;

typedef struct _MOTION_DETECTION_Time_t {
    int m_startTime;
    int m_endTime;
} MOTION_DETECTION_Time_t;

typedef struct _MOTION_DETECTION_TimeRange_t {
    MOTION_DETECTION_Time_t* m_timeRangeArray;
    int m_nowTime;
    int m_rangeCount;
} MOTION_DETECTION_TimeRange_t;

typedef MOTION_DETECTION_Result_t (*MOTION_DETECTION_OnMotionEventCallback)(MOTION_DETECTION_EventMessage_t astEventMessage);

class MotionDetectionImpl {
public:
    virtual ~MotionDetectionImpl() = default;

    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_Platform_Init() = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_Platform_Term() = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_RegisterEventCallback(MOTION_DETECTION_OnMotionEventCallback motionEvent) = 0;

    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_GetMotionDetectors(MOTION_DETECTION_CurrentSensorSettings_t* motionDetectors) = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_ArmMotionDetector(MOTION_DETECTION_Mode_t mode, std::string index) = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_DisarmMotionDetector(std::string index) = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_IsMotionDetectorArmed(std::string index, bool* armState) = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_SetNoMotionPeriod(std::string index, int period) = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_GetNoMotionPeriod(std::string index, unsigned int* period) = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_SetSensitivity(std::string index, std::string sensitivity, int inferredMode ) = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_GetSensitivity(std::string index, char** sensitivity, int* currentMode) = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_SetActivePeriod(std::string index, MOTION_DETECTION_TimeRange_t timeSet) = 0;
    virtual MOTION_DETECTION_Result_t MOTION_DETECTION_GetActivePeriod(MOTION_DETECTION_TimeRange_t* timeSet) = 0;
};

class MotionDetection {
public:
    static MotionDetection& getInstance()
    {
        static MotionDetection instance;
        return instance;
    }

    MotionDetectionImpl* impl;

    static MOTION_DETECTION_Result_t MOTION_DETECTION_Platform_Init()
    {
        return getInstance().impl->MOTION_DETECTION_Platform_Init();
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_Platform_Term()
    {
            return getInstance().impl->MOTION_DETECTION_Platform_Term();
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_RegisterEventCallback(MOTION_DETECTION_OnMotionEventCallback motionEvent)
    {
            return getInstance().impl->MOTION_DETECTION_RegisterEventCallback(motionEvent);
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_GetMotionDetectors(MOTION_DETECTION_CurrentSensorSettings_t* motionDetectors)
    {
        return getInstance().impl->MOTION_DETECTION_GetMotionDetectors(motionDetectors);
    }

	 static MOTION_DETECTION_Result_t MOTION_DETECTION_ArmMotionDetector(MOTION_DETECTION_Mode_t mode, std::string index)
    {
        return getInstance().impl->MOTION_DETECTION_ArmMotionDetector(mode, index);
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_DisarmMotionDetector(std::string index)
    {
            return getInstance().impl->MOTION_DETECTION_DisarmMotionDetector(index);
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_IsMotionDetectorArmed(std::string index, bool* armState)
    {
            return getInstance().impl->MOTION_DETECTION_IsMotionDetectorArmed(index, armState);
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_SetNoMotionPeriod(std::string index, int period)
    {
            return getInstance().impl->MOTION_DETECTION_SetNoMotionPeriod(index, period);
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_GetNoMotionPeriod(std::string index, unsigned int* period)
    {
            return getInstance().impl->MOTION_DETECTION_GetNoMotionPeriod(index, period);
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_SetSensitivity(std::string index, std::string sensitivity, int inferredMode )
    {
            return getInstance().impl->MOTION_DETECTION_SetSensitivity(index, sensitivity, inferredMode);
    }

	 static MOTION_DETECTION_Result_t MOTION_DETECTION_GetSensitivity(std::string index, char** sensitivity, int* currentMode)
    {
            return getInstance().impl->MOTION_DETECTION_GetSensitivity(index, sensitivity, currentMode);
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_SetActivePeriod(std::string index, MOTION_DETECTION_TimeRange_t timeSet)
    {
            return getInstance().impl->MOTION_DETECTION_SetActivePeriod(index, timeSet);
    }

    static MOTION_DETECTION_Result_t MOTION_DETECTION_GetActivePeriod(MOTION_DETECTION_TimeRange_t* timeSet)
    {
            return getInstance().impl->MOTION_DETECTION_GetActivePeriod(timeSet);
    }

};

constexpr auto MOTION_DETECTION_Platform_Init = &MotionDetection::MOTION_DETECTION_Platform_Init;
constexpr auto MOTION_DETECTION_Platform_Term = &MotionDetection::MOTION_DETECTION_Platform_Term;
constexpr auto MOTION_DETECTION_RegisterEventCallback = &MotionDetection::MOTION_DETECTION_RegisterEventCallback;
constexpr auto MOTION_DETECTION_GetMotionDetectors = &MotionDetection::MOTION_DETECTION_GetMotionDetectors;
constexpr auto MOTION_DETECTION_ArmMotionDetector = &MotionDetection::MOTION_DETECTION_ArmMotionDetector;
constexpr auto MOTION_DETECTION_DisarmMotionDetector = &MotionDetection::MOTION_DETECTION_DisarmMotionDetector;
constexpr auto MOTION_DETECTION_IsMotionDetectorArmed = &MotionDetection::MOTION_DETECTION_IsMotionDetectorArmed;
constexpr auto MOTION_DETECTION_SetNoMotionPeriod = &MotionDetection::MOTION_DETECTION_SetNoMotionPeriod;
constexpr auto MOTION_DETECTION_GetNoMotionPeriod = &MotionDetection::MOTION_DETECTION_GetNoMotionPeriod;
constexpr auto MOTION_DETECTION_SetSensitivity = &MotionDetection::MOTION_DETECTION_SetSensitivity;
constexpr auto MOTION_DETECTION_GetSensitivity = &MotionDetection::MOTION_DETECTION_GetSensitivity;
constexpr auto MOTION_DETECTION_SetActivePeriod = &MotionDetection::MOTION_DETECTION_SetActivePeriod;
constexpr auto MOTION_DETECTION_GetActivePeriod = &MotionDetection::MOTION_DETECTION_GetActivePeriod;

