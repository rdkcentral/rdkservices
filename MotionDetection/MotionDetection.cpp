/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

//I have put several "TODO(MROLLINS)" in the code below to mark areas of concern I encountered
//  when refactoring the servicemanager's version of displaysettings into this new thunder plugin format

#include "MotionDetection.h"
#include "tracing/Logging.h"
#include <syscall.h>
#include "UtilsJsonRpc.h"

#define NO_DETECTORS_FOUND    "0"
#define MOTION_DETECTOR_INDEX "FP_MD"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

using namespace std;

namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::MotionDetection> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin {

        SERVICE_REGISTRATION(MotionDetection, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        MotionDetection* MotionDetection::_instance = nullptr;

        MOTION_DETECTION_Result_t motiondetection_EventCallback (MOTION_DETECTION_EventMessage_t eventMsg)
        {
            if (!MotionDetection::_instance) {
                LOGERR ("Invalid pointer. Motion Detector is not initialized (yet?). Event is ignored");
                return MOTION_DETECTION_RESULT_INTI_FAILURE;
            } else {
                string index(eventMsg.m_sensorIndex);
                string eventType(1, eventMsg.m_eventType); 
                MotionDetection::_instance->onMotionEvent(index, eventType);
                return MOTION_DETECTION_RESULT_SUCCESS;
            }
        }

        MotionDetection::MotionDetection()
            : PluginHost::JSONRPC()
        {
            LOGINFO("MotionDetection ctor");
            MotionDetection::_instance = this;

            Register("getMotionDetectors", &MotionDetection::getMotionDetectors, this);
            Register("arm", &MotionDetection::arm, this);
            Register("disarm", &MotionDetection::disarm, this);
            Register("isarmed", &MotionDetection::isarmed, this);
            Register("setNoMotionPeriod", &MotionDetection::setNoMotionPeriod, this);
            Register("getNoMotionPeriod", &MotionDetection::getNoMotionPeriod, this);
            Register("setSensitivity", &MotionDetection::setSensitivity, this);
            Register("getSensitivity", &MotionDetection::getSensitivity, this);
            Register("getLastMotionEventElapsedTime", &MotionDetection::getLastMotionEventElapsedTime, this);
            Register("setMotionEventsActivePeriod", &MotionDetection::setMotionEventsActivePeriod, this);
            Register("getMotionEventsActivePeriod", &MotionDetection::getMotionEventsActivePeriod, this);

        }

        MotionDetection::~MotionDetection()
        {
        }

        void setResponseArray(JsonObject& response, const char* key, const vector<string>& items)
        {
            JsonArray arr;
            for(auto& i : items) arr.Add(JsonValue(i));

            response[key] = arr;

            string json;
            response.ToString(json);
        }

        const string MotionDetection::Initialize(PluginHost::IShell* /* service */)
        {
            // On success return empty, to indicate there is no error text.
	    MOTION_DETECTION_Platform_Init();

            MOTION_DETECTION_RegisterEventCallback(motiondetection_EventCallback);

            MOTION_DETECTION_DisarmMotionDetector(MOTION_DETECTOR_INDEX);

            m_lastEventTime = std::chrono::system_clock::now();
            return (string());
        }

        void MotionDetection::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO("MotionDetection Deinitialize");
	    MOTION_DETECTION_Platform_Term();
            MotionDetection::_instance = nullptr;
            Unregister("getMotionDetectors");
            Unregister("arm");
            Unregister("disarm");
            Unregister("isarmed");
            Unregister("setNoMotionPeriod");
            Unregister("getNoMotionPeriod");
            Unregister("setSensitivity");
            Unregister("getSensitivity");
            Unregister("getLastMotionEventElapsedTime");
            Unregister("setMotionEventsActivePeriod");
            Unregister("getMotionEventsActivePeriod");
        }

        //Begin methods
        uint32_t MotionDetection::getMotionDetectors(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
            MOTION_DETECTION_CurrentSensorSettings_t motionDetectors;
            JsonObject sensorData;
            JsonArray dectectorList;
            JsonObject detectorInfo;

            rc = MOTION_DETECTION_GetMotionDetectors(&motionDetectors);

            if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                LOGERR("Failed to fetch list of motion detectors..!");
                response["supportedMotionDetectors"] = NO_DETECTORS_FOUND;
                returnResponse(false);
            }
            sensorData["description"] = string(motionDetectors.m_sensorDescription);
	    sensorData["type"] = string(motionDetectors.m_sensorType);
	    sensorData["distance"] = std::to_string(motionDetectors.m_sensorDistance);
            sensorData["angle"] = std::to_string(motionDetectors.m_sensorAngle);
            sensorData["sensitivityMode"] = std::to_string(motionDetectors.m_sensitivityMode);
            if (motionDetectors.m_sensitivityMode == SENSITIVITY_MODE_LEVELS) {
                std::vector<std::string> sensitivityIdentifiers;
                for (int identifiers = 0; identifiers < SENSITIVITY_IDENTIFIERS; identifiers++) {
                    sensitivityIdentifiers.push_back(string(motionDetectors.m_sensitivity[identifiers]));
                }
                setResponseArray(sensorData, "sensitivities", sensitivityIdentifiers);
            }
            else if (motionDetectors.m_sensitivityMode == SENSITIVITY_MODE_INT) {
                sensorData["min"] = string(motionDetectors.m_sensitivity[SENSITIVITY_IDENTIFIER_1]);
                sensorData["max"] = string(motionDetectors.m_sensitivity[SENSITIVITY_IDENTIFIER_1]);
                sensorData["step"] = string(motionDetectors.m_sensitivity[SENSITIVITY_IDENTIFIER_1]);
            }
            detectorInfo[motionDetectors.m_sensorIndex] = sensorData;
            dectectorList.Add(std::string(motionDetectors.m_sensorIndex));
            response["supportedMotionDetectors"] = dectectorList;
            response["supportedMotionDetectorsInfo"] =  detectorInfo;

            returnResponse(true);
        }

        uint32_t MotionDetection::arm(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string index = parameters["index"].String();
            string sMode = parameters["mode"].String();
            int mode = stoi(sMode);
            MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
            rc = MOTION_DETECTION_ArmMotionDetector((MOTION_DETECTION_Mode_t)mode, index.c_str());

            if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                LOGERR("Failed to arm the motion detector..!");
                returnResponse(false);
            }

            returnResponse(true);
        }

        uint32_t MotionDetection::disarm(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string index = parameters["index"].String();
            MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
            rc = MOTION_DETECTION_DisarmMotionDetector(index.c_str());

            if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                LOGERR("Failed to disarm the motion detector..!");
                returnResponse(false);
            }

            returnResponse(true);
        }

        uint32_t MotionDetection::isarmed(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string index = parameters["index"].String();
            bool armState = false;
            MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
            rc = MOTION_DETECTION_IsMotionDetectorArmed(index.c_str(), &armState);

            if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                LOGERR("Failed to check motion detector status..!");
                response["state"] = armState; 
                returnResponse(false);
            }
            else {
                response["state"] = armState;
            }
            returnResponse(true);

        }

        uint32_t MotionDetection::setNoMotionPeriod(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string index = parameters["index"].String();
            string sPeriod = parameters["period"].String();
            int period = stoi(sPeriod);
            MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
            rc = MOTION_DETECTION_SetNoMotionPeriod(index.c_str(), period);

            if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                LOGERR("Failed to set no motion period..!");
                returnResponse(false);
            }
            returnResponse(true);
        }

        uint32_t MotionDetection::getNoMotionPeriod(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string index = parameters["index"].String();
            MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
            unsigned int period = 0;
            rc = MOTION_DETECTION_GetNoMotionPeriod(index.c_str(), &period);

            if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                LOGERR("Failed to get no motion period..!");
                returnResponse(false);
            }
            response["period"] = std::to_string(period);
            returnResponse(true);
        }

        uint32_t MotionDetection::setSensitivity(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string index = parameters["index"].String();
            string sensitivity;
            int    inferredMode = 0;
            if (parameters.HasLabel("name")) {
                sensitivity = parameters["name"].String();
                inferredMode = 2;
            }

            if (parameters.HasLabel("value")) {
                sensitivity = parameters["value"].String();
                inferredMode = 1;
            }

            if (!inferredMode) {
                LOGERR("Sensitivity cannot be changed in this mode..!");
                returnResponse(false);
            }

            MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
            rc = MOTION_DETECTION_SetSensitivity(index.c_str(), sensitivity.c_str(), inferredMode);

            if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                LOGERR("Failed to set sensitivity..!");
                returnResponse(false);
            }

            returnResponse(true);
        }

        uint32_t MotionDetection::getSensitivity(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string index = parameters["index"].String();
            char *sensitivity = nullptr;
            int currentMode = 0; 
            MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
            rc = MOTION_DETECTION_GetSensitivity(index.c_str(), &sensitivity, &currentMode);

            if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                LOGERR("Failed to get sensitivity..!");
                returnResponse(false);
            }

            string rSensitivity(sensitivity);

            if (currentMode == 1) {
                response["value"] = rSensitivity;
            }
            else if (currentMode == 2) {
                response["name"] = rSensitivity;
            }
            
            if (sensitivity) {
                free(sensitivity);
            }
            returnResponse(true);

        }

        uint32_t MotionDetection::getLastMotionEventElapsedTime(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string index = parameters["index"].String();
            MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
            double eventElapsedTime = 0;
            std::chrono::duration<double> elapsedTime = std::chrono::system_clock::now() - m_lastEventTime;

            eventElapsedTime = elapsedTime.count();

            if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                LOGERR("Failed to get last motion event elapsed time..!");
                returnResponse(false);
            }
            response["time"] = std::to_string(eventElapsedTime);
            returnResponse(true);
        }

        uint32_t MotionDetection::setMotionEventsActivePeriod(const JsonObject& parameters, JsonObject& response)
        {
             LOGINFOMETHOD();
             if (parameters.HasLabel("index") && parameters.HasLabel("nowTime") && parameters.HasLabel("ranges"))
             {
                 MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
                 MOTION_DETECTION_TimeRange_t timeSet;
                 int nowTime = 0;
                 bool parseStatus = true;
                 string index = parameters["index"].String();
                 JsonArray rangeList = parameters["ranges"].Array();
                 getNumberParameterObject(parameters, "nowTime", nowTime);
                 timeSet.m_nowTime = nowTime;
                 timeSet.m_timeRangeArray = (MOTION_DETECTION_Time_t *)malloc(rangeList.Length() * sizeof(MOTION_DETECTION_Time_t));
                 timeSet.m_rangeCount = rangeList.Length();
                 for (int range = 0;  range < rangeList.Length(); range++)
                 {
                     JsonObject rangeObj = rangeList[range].Object();
                     if (rangeObj.HasLabel("startTime") && rangeObj.HasLabel("endTime"))
                     {
                         unsigned int startTime, endTime = 0;
                         getNumberParameterObject(rangeObj, "startTime", startTime);
                         getNumberParameterObject(rangeObj, "endTime", endTime);
                         timeSet.m_timeRangeArray[range].m_startTime = startTime;
                         timeSet.m_timeRangeArray[range].m_endTime = endTime;
                     }
                     else
                     {
                         LOGINFO("Parameters missing in JSON Array");
                         parseStatus = false;
                         break;
                     }
                 }
                 if (parseStatus == true)
                 {
                     rc = MOTION_DETECTION_SetActivePeriod(index.c_str(), timeSet);
                     if (rc != MOTION_DETECTION_RESULT_SUCCESS) 
                     {
                         LOGERR("Failed to set Active Time..!");
                         returnResponse(false);
                     }
                 }
                 else
                 {
                     returnResponse(false);
                 }
                 free(timeSet.m_timeRangeArray);
                 returnResponse(true);
             }
             else
             {
                 LOGINFO("Parameters missing in JSON request");
                 returnResponse(false);
             }
        }


        uint32_t MotionDetection::getMotionEventsActivePeriod(const JsonObject& parameters, JsonObject& response)
        {
             LOGINFOMETHOD();
             MOTION_DETECTION_Result_t rc = MOTION_DETECTION_RESULT_SUCCESS;
             MOTION_DETECTION_TimeRange_t timeSet;
             JsonArray rangeList;
             rc = MOTION_DETECTION_GetActivePeriod(&timeSet);
             if (rc != MOTION_DETECTION_RESULT_SUCCESS) {
                 LOGERR("Failed to get Active Time..!");
                 returnResponse(false);
             }
             if (timeSet.m_rangeCount > 0)
             {
                 for (int range = 0; range < timeSet.m_rangeCount; range++)
                 {
                     JsonObject rangeObj;
                     rangeObj["startTime"] = std::to_string(timeSet.m_timeRangeArray[range].m_startTime);
                     rangeObj["endTime"] = std::to_string(timeSet.m_timeRangeArray[range].m_endTime);
                     rangeList.Add(rangeObj);
                 }
                 response["ranges"] = rangeList;
             }
             else if (timeSet.m_rangeCount == 0)
             {
                 response["message"] = "No Active Periods Set";
             }
             returnResponse(true);
        }
        //End methods

        //Begin events
        void MotionDetection::onMotionEvent(const string& index, const string& eventType)
        {
            JsonObject params;
            params["index"] = index;
            params["mode"] = eventType;
            sendNotify("onMotionEvent", params);

            m_lastEventTime = std::chrono::system_clock::now();
        }
        //End events

    } // namespace Plugin
} // namespace WPEFramework
