/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 Synamedia
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
**/

#include "MotionDetection.h"
#include <gmock/gmock.h>

MotionDetectionImpl* MotionDetection::impl = nullptr;

MotionDetection::MotionDetection() {}

void MotionDetection::setImpl(MotionDetectionImpl* newImpl)
{
   // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
   EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
   impl = newImpl;
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_Platform_Init()
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_Platform_Init();
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_Platform_Term()
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_Platform_Term();
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_RegisterEventCallback(MOTION_DETECTION_OnMotionEventCallback motionEvent)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_RegisterEventCallback(motionEvent);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_GetMotionDetectors(MOTION_DETECTION_CurrentSensorSettings_t* motionDetectors)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_GetMotionDetectors(motionDetectors);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_ArmMotionDetector(MOTION_DETECTION_Mode_t mode, std::string index)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_ArmMotionDetector(mode, index);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_DisarmMotionDetector(std::string index)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_DisarmMotionDetector(index);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_IsMotionDetectorArmed(std::string index, bool* armState)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_IsMotionDetectorArmed(index, armState);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_SetNoMotionPeriod(std::string index, int period)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_SetNoMotionPeriod(index, period);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_GetNoMotionPeriod(std::string index, unsigned int* period)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_GetNoMotionPeriod(index, period);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_SetSensitivity(std::string index, std::string sensitivity, int inferredMode )
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_SetSensitivity(index, sensitivity, inferredMode);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_GetSensitivity(std::string index, char** sensitivity, int* currentMode)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_GetSensitivity(index, sensitivity, currentMode);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_SetActivePeriod(std::string index, MOTION_DETECTION_TimeRange_t timeSet)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_SetActivePeriod(index, timeSet);
}

MOTION_DETECTION_Result_t MotionDetection::MOTION_DETECTION_GetActivePeriod(MOTION_DETECTION_TimeRange_t* timeSet)
{
    EXPECT_NE(impl, nullptr);
    return impl->MOTION_DETECTION_GetActivePeriod(timeSet);
}

MOTION_DETECTION_Result_t(*MOTION_DETECTION_Platform_Init)() = &MotionDetection::MOTION_DETECTION_Platform_Init;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_Platform_Term)() = &MotionDetection::MOTION_DETECTION_Platform_Term;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_RegisterEventCallback)(MOTION_DETECTION_OnMotionEventCallback) = &MotionDetection::MOTION_DETECTION_RegisterEventCallback;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_GetMotionDetectors)(MOTION_DETECTION_CurrentSensorSettings_t*) = &MotionDetection::MOTION_DETECTION_GetMotionDetectors;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_ArmMotionDetector)(MOTION_DETECTION_Mode_t, std::string) = &MotionDetection::MOTION_DETECTION_ArmMotionDetector;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_DisarmMotionDetector)(std::string) = &MotionDetection::MOTION_DETECTION_DisarmMotionDetector;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_IsMotionDetectorArmed)(std::string, bool*) = &MotionDetection::MOTION_DETECTION_IsMotionDetectorArmed;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_SetNoMotionPeriod)(std::string, int) = &MotionDetection::MOTION_DETECTION_SetNoMotionPeriod;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_GetNoMotionPeriod)(std::string, unsigned int*) = &MotionDetection::MOTION_DETECTION_GetNoMotionPeriod;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_SetSensitivity)(std::string, std::string, int) = &MotionDetection::MOTION_DETECTION_SetSensitivity;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_GetSensitivity)(std::string, char**, int*) = &MotionDetection::MOTION_DETECTION_GetSensitivity;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_SetActivePeriod)(std::string, MOTION_DETECTION_TimeRange_t) = &MotionDetection::MOTION_DETECTION_SetActivePeriod;
MOTION_DETECTION_Result_t(*MOTION_DETECTION_GetActivePeriod)(MOTION_DETECTION_TimeRange_t*) = &MotionDetection::MOTION_DETECTION_GetActivePeriod;
