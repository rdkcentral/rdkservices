#include <gtest/gtest.h>
#include <iostream>

#include "MotionDetection.h"
#include "FactoriesImplementation.h"
#include "MotionDetectionMock.h"

#include "IarmBusMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;

using ::testing::NiceMock;
using ::testing::Eq;

class MotionDetectionTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::MotionDetection> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    MotionDetectionTest()
        : plugin(Core::ProxyType<Plugin::MotionDetection>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~MotionDetectionTest() = default;
};

class MotionDetectionEventTest : public MotionDetectionTest {
protected:

    NiceMock<MotionDetectionImplMock> motionDetectionImplMock;

    MotionDetectionEventTest()
        : MotionDetectionTest()
    {

            MotionDetection::getInstance().impl = &motionDetectionImplMock;

          ON_CALL(motionDetectionImplMock, MOTION_DETECTION_Platform_Init())
              .WillByDefault(::testing::Return(MOTION_DETECTION_RESULT_SUCCESS));

          ON_CALL(motionDetectionImplMock, MOTION_DETECTION_RegisterEventCallback(::testing::_))
              .WillByDefault(::testing::Return(MOTION_DETECTION_RESULT_SUCCESS));

          ON_CALL(motionDetectionImplMock, MOTION_DETECTION_DisarmMotionDetector(::testing::_))
              .WillByDefault(::testing::Return(MOTION_DETECTION_RESULT_SUCCESS));

           EXPECT_EQ(string(""), plugin->Initialize(nullptr));

    }
    virtual ~MotionDetectionEventTest() override
    {

        plugin->Deinitialize(nullptr);
    }
};

TEST_F(MotionDetectionTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMotionDetectors")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("arm")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("disarm")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isarmed")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setNoMotionPeriod")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getNoMotionPeriod")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setSensitivity")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSensitivity")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLastMotionEventElapsedTime")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setMotionEventsActivePeriod")));
}

TEST_F(MotionDetectionEventTest, getMotionDetectors)
{
		EXPECT_CALL(motionDetectionImplMock, MOTION_DETECTION_GetMotionDetectors(::testing::_))
			.Times(1)
			.WillOnce(::testing::Invoke(   
				[](MOTION_DETECTION_CurrentSensorSettings_t *pSensorStatus) {
                
                memset(pSensorStatus, 0, sizeof(MOTION_DETECTION_CurrentSensorSettings_t));

                strncpy(pSensorStatus->m_sensorIndex, MOTION_DETECTOR, sizeof(pSensorStatus->m_sensorIndex) - 1);
                strncpy(pSensorStatus->m_sensorDescription, MOTION_DETECTION_DESCRIPTION, sizeof(pSensorStatus->m_sensorDescription) - 1);
                strncpy(pSensorStatus->m_sensorType, MOTION_DETECTOR_TYPE, sizeof(pSensorStatus->m_sensorType) - 1);
                pSensorStatus->m_sensorDistance = MOTION_DETECTION_DISTANCE;
                pSensorStatus->m_sensorAngle = MOTION_DETECTION_ANGLE;
                pSensorStatus->m_sensitivityMode = 2;
                
                strncpy(pSensorStatus->m_sensitivity[SENSITIVITY_IDENTIFIER_1], STR_SENSITIVITY_LOW, sizeof(pSensorStatus->m_sensitivity[SENSITIVITY_IDENTIFIER_1]) - 1);
                strncpy(pSensorStatus->m_sensitivity[SENSITIVITY_IDENTIFIER_2], STR_SENSITIVITY_MEDIUM, sizeof(pSensorStatus->m_sensitivity[SENSITIVITY_IDENTIFIER_2]) - 1);
                strncpy(pSensorStatus->m_sensitivity[SENSITIVITY_IDENTIFIER_3], STR_SENSITIVITY_HIGH, sizeof(pSensorStatus->m_sensitivity[SENSITIVITY_IDENTIFIER_3]) - 1);

                return MOTION_DETECTION_RESULT_SUCCESS;
            }));

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMotionDetectors"), _T("{}"), response));
        EXPECT_EQ(response,  string("{\"supportedMotionDetectors\":[\"FP_MD\"],\"supportedMotionDetectorsInfo\":{\"FP_MD\":{\"description\":\"The only motion detector\",\"type\":\"PID\",\"distance\":\"6000\",\"angle\":\"74\",\"sensitivityMode\":\"2\",\"sensitivities\":[\"low\",\"medium\",\"high\"]}},\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, armmotiondetected)
{
	EXPECT_CALL(motionDetectionImplMock, MOTION_DETECTION_ArmMotionDetector(::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
		[](MOTION_DETECTION_Mode_t mode,std::string index) {
                return MOTION_DETECTION_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("arm"), _T("{\"index\":\"FP_MD\",\"mode\":\"1\" }"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, armmotiondetectedInvalid)
{
    EXPECT_CALL(motionDetectionImplMock, MOTION_DETECTION_ArmMotionDetector(::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
		[](MOTION_DETECTION_Mode_t mode, std::string index) {
                return MOTION_DETECTION_RESULT_INDEX_ERROR;
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("arm"), _T("{ \"index\":\"FP_MD\",\"mode\":\"1\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(MotionDetectionEventTest, disarm)
{
	EXPECT_CALL(motionDetectionImplMock, MOTION_DETECTION_DisarmMotionDetector(::testing::_))
		.Times(1)
		.WillOnce(::testing::Invoke(
            [](std::string index) {
                return MOTION_DETECTION_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disarm"), _T("{\"index\":\"FP_MD\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, disarmInvalid)
{
	EXPECT_CALL(motionDetectionImplMock, MOTION_DETECTION_DisarmMotionDetector(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](std::string index) {
                return MOTION_DETECTION_RESULT_INDEX_ERROR;
            }));


    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("disarm"), _T("{\"index\":\"FP_MD\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(MotionDetectionEventTest, isarmed)
{
	EXPECT_CALL(motionDetectionImplMock,MOTION_DETECTION_IsMotionDetectorArmed(::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
            [](std::string index, bool *isArmed) {
                *isArmed = false;
                return MOTION_DETECTION_RESULT_SUCCESS;
            }));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isarmed"), _T("{\"index\"=\"FP_MD\"}"), response));
    EXPECT_EQ(response,  string("{\"state\":false,\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, isarmedInvalid)
{
    EXPECT_CALL(motionDetectionImplMock,MOTION_DETECTION_IsMotionDetectorArmed(::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
			[](std::string index, bool *isArmed) {
                return MOTION_DETECTION_RESULT_INDEX_ERROR;
            }));


    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("isarmed"), _T("{\"index\"=\"FP_MD\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(MotionDetectionEventTest, setNoMotionPeriod)
{
	EXPECT_CALL(motionDetectionImplMock, MOTION_DETECTION_SetNoMotionPeriod(::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
            [](std::string index, unsigned int period) {
                return MOTION_DETECTION_RESULT_SUCCESS;
            }));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setNoMotionPeriod"), _T("{\"index\":\"FP_MD\",\"period\":\"10\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, setNoMotionPeriodInvalid)
{
	EXPECT_CALL(motionDetectionImplMock, MOTION_DETECTION_SetNoMotionPeriod(::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
            [](std::string index, unsigned int period) {
                return MOTION_DETECTION_RESULT_INDEX_ERROR;
            }));


    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setNoMotionPeriod"), _T("{\"index\":\"FP_MD\",\"period\":\"10\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(MotionDetectionEventTest, getNoMotionPeriod)
{
	EXPECT_CALL(motionDetectionImplMock, MOTION_DETECTION_GetNoMotionPeriod(::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(          
			[](std::string index, unsigned int *noMotionPeriod) {
                *noMotionPeriod =(unsigned int)10;
                return MOTION_DETECTION_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getNoMotionPeriod"), _T("{\"index\"=\"FP_MD\"}"), response));
    EXPECT_EQ(response,  string("{\"period\":\"10\",\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, getNoMotionPeriodInvalid)
{
	EXPECT_CALL(motionDetectionImplMock, MOTION_DETECTION_GetNoMotionPeriod(::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
            [](std::string index, unsigned int *noMotionPeriod) {
                return MOTION_DETECTION_RESULT_INDEX_ERROR;
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getNoMotionPeriod"), _T("{\"index\"=\"FP_MD\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(MotionDetectionEventTest, setSensitivity)
{
	EXPECT_CALL(motionDetectionImplMock,MOTION_DETECTION_SetSensitivity(::testing::_,::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
            [](std::string index, std::string sensitivity, int inferredMode) {
                return MOTION_DETECTION_RESULT_SUCCESS;
            }));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setSensitivity"), _T("{\"index\":\"FP_MD\",\"name\":\"high\",\"value\":\"40\"}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, setSensitivityInvalid)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setSensitivity"), _T("{\"index\":\"FP_MD\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(MotionDetectionEventTest, getSensitivity)
{
	 EXPECT_CALL(motionDetectionImplMock,MOTION_DETECTION_GetSensitivity(::testing::_,::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
            [](std::string index, char** sensitivity, int* currentMode) {
                *currentMode = 1;
                *sensitivity = (char *)malloc(sizeof(STR_SENSITIVITY_HIGH));
                memset(*sensitivity, 0, sizeof(STR_SENSITIVITY_HIGH));
                strcpy(*sensitivity, STR_SENSITIVITY_HIGH);
                return MOTION_DETECTION_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSensitivity"), _T("{\"index\":\"FP_MD\"}"), response));
    EXPECT_EQ(response,  string("{\"value\":\"high\",\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, getSensitivityInvalid)
{
     EXPECT_CALL(motionDetectionImplMock,MOTION_DETECTION_GetSensitivity(::testing::_,::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
            [](std::string index, char** sensitivity, int* currentMode) {
                return MOTION_DETECTION_RESULT_INDEX_ERROR;
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getSensitivity"), _T("{\"index\":\"FP_MD\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(MotionDetectionEventTest, getLastMotionEventElapsedTime)
{

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getLastMotionEventElapsedTime"), _T("{}"), response));

    EXPECT_THAT(response, ::testing::MatchesRegex(_T("\\{"
                    "\"time\":\"[0-9]+.[0-9]+\","
                    "\"success\":true"
                    "\\}")));
}

TEST_F(MotionDetectionEventTest, setMotionEventsActivePeriod)
{
	EXPECT_CALL(motionDetectionImplMock,MOTION_DETECTION_SetActivePeriod(::testing::_,::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
            [](std::string index, MOTION_DETECTION_TimeRange_t timeSet) {
                return MOTION_DETECTION_RESULT_SUCCESS;
            }));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMotionEventsActivePeriod"), _T("{\"nowTime\":1023,\"index\":\"FP_MD\",\"ranges\":[{\"startTime\":\"100\", \"endTime\":\"150\"}]}"), response));
    EXPECT_EQ(response,  string("{\"success\":true}"));
}
