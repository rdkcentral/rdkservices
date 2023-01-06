#pragma once

#include <gmock/gmock.h>


class MotionDetectionImplMock : public MotionDetectionImpl {
public:

        MotionDetectionImplMock():MotionDetectionImpl()
        {
                ON_CALL(*this, MOTION_DETECTION_Platform_Init())
                        .WillByDefault(::testing::Return(MOTION_DETECTION_RESULT_SUCCESS));

                ON_CALL(*this, MOTION_DETECTION_Platform_Term())
                        .WillByDefault(::testing::Return(MOTION_DETECTION_RESULT_SUCCESS));
        }

    virtual ~MotionDetectionImplMock() = default;

    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_Platform_Init, (), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_Platform_Term, (), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_RegisterEventCallback, (MOTION_DETECTION_OnMotionEventCallback motionEvent), (override));

    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_GetMotionDetectors, (MOTION_DETECTION_CurrentSensorSettings_t* motionDetectors), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_ArmMotionDetector, (MOTION_DETECTION_Mode_t mode, std::string index), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_DisarmMotionDetector, (std::string index), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_IsMotionDetectorArmed, (std::string index, bool* armState), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_SetNoMotionPeriod, (std::string index, int period), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_GetNoMotionPeriod, (std::string index, unsigned int* period), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_SetSensitivity, (std::string index, std::string sensitivity, int inferredMode), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_GetSensitivity, (std::string index, char** sensitivity, int* currentMode), (override));
 MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_SetActivePeriod, (std::string index, MOTION_DETECTION_TimeRange_t timeSet), (override));
    MOCK_METHOD(MOTION_DETECTION_Result_t, MOTION_DETECTION_GetActivePeriod, (MOTION_DETECTION_TimeRange_t* timeSet), (override));
};

