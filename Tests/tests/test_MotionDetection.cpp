#include <gtest/gtest.h>

#include "MotionDetection.h"
#include "FactoriesImplementation.h"
#include "MotionDetectionMock.h"

#include "IarmBusMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;

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

    MotionDetectionImplMock motionDetectionImplMock;

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
          ON_CALL(motionDetectionImplMock, MOTION_DETECTION_GetMotionDetectors(::testing::_))
        .WillByDefault(::testing::Invoke(
            [](MOTION_DETECTION_CurrentSensorSettings_t *pSensorStatus) {
                pSensorStatus = (MOTION_DETECTION_CurrentSensorSettings_t *)malloc(sizeof(MOTION_DETECTION_CurrentSensorSettings_t));
                memset(pSensorStatus, 0, sizeof(MOTION_DETECTION_CurrentSensorSettings_t));

                strcpy(pSensorStatus->m_sensorIndex, MOTION_DETECTOR);
                strcpy(pSensorStatus->m_sensorDescription, MOTION_DETECTION_DESCRIPTION);
                strcpy(pSensorStatus->m_sensorType, MOTION_DETECTOR_TYPE);
                pSensorStatus->m_sensorDistance = MOTION_DETECTION_DISTANCE;
                pSensorStatus->m_sensorAngle = MOTION_DETECTION_ANGLE;
                pSensorStatus->m_sensitivityMode = 2;

                strcpy(pSensorStatus->m_sensitivity[SENSITIVITY_IDENTIFIER_1], STR_SENSITIVITY_LOW);
                strcpy(pSensorStatus->m_sensitivity[SENSITIVITY_IDENTIFIER_2], STR_SENSITIVITY_MEDIUM);
                strcpy(pSensorStatus->m_sensitivity[SENSITIVITY_IDENTIFIER_3], STR_SENSITIVITY_HIGH);

                return MOTION_DETECTION_RESULT_SUCCESS;
            }));

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMotionDetectors"), _T("{}"), response));
        EXPECT_EQ(response,  string("{\"supportedMotionDetectors\":[\"\x1F\"],\"supportedMotionDetectorsInfo\":{\"\x1F\":{\"description\":\"\",\"type\":\"\",\"distance\":\"0\",\"angle\":\"0\",\"sensitivityMode\":\"4278187664\"}},\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, armnomotiondetected)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("arm"), _T("{\"index\":\"FP_MD\",\"mode\":\"0\" }"), response));
        EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, armmotiondetected)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("arm"), _T("{\"index\":\"FP_MD\",\"mode\":\"1\" }"), response));
        EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, armErrmotiondetected)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("arm"), _T("{\"index\":\"FP_MD\",\"mode\":\"10\" }"), response));
        EXPECT_EQ(response,  string("{\"success\":false}"));
}


TEST_F(MotionDetectionEventTest, disarm)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disarm"), _T("{\"index\":\"FP_MD\"}"), response));
        EXPECT_EQ(response,  string("{\"success\":true}"));
}
TEST_F(MotionDetectionEventTest, disarm)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disarm"), _T("{\"index\":\"FP_MD\"}"), response));
        EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, disarmError)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disarm"), _T("{\"index\":\"FP_XX\"}"), response));
        EXPECT_EQ(response,  string("{\"success\":false}"));
}


TEST_F(MotionDetectionEventTest, isarmed)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isarmed"), _T("{\"index\"=\"FP_MD\"}"), response));
        EXPECT_EQ(response,  string("{\"state\":false,\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, setNoMotionPeriod)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setNoMotionPeriod"), _T("{\"index\":\"FP_MD\",\"period\":\"10\"}"), response));
        EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, getNoMotionPeriod)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getNoMotionPeriod"), _T("{\"index\"=\"FP_MD\"}"), response));
        EXPECT_EQ(response,  string("{\"period\":\"0\",\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, setSensitivity)
{

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setSensitivity"), _T("{\"index\":\"FP_MD\",\"name\":\"high\",\"value\":\"2\"}"), response));
        EXPECT_EQ(response,  string("{\"success\":true}"));
}

TEST_F(MotionDetectionEventTest, getSensitivity)
{
          ON_CALL(motionDetectionImplMock, MOTION_DETECTION_GetSensitivity(::testing::_,::testing::_,::testing::_))
        .WillByDefault(::testing::Invoke(
            [](std::string index, char** sensitivity, int* currentMode) {
                *currentMode = 1;
                *sensitivity = (char *)malloc(sizeof(STR_SENSITIVITY_LOW));
                memset(*sensitivity, 0, sizeof(STR_SENSITIVITY_LOW));
                strcpy(*sensitivity, STR_SENSITIVITY_LOW);
                return MOTION_DETECTION_RESULT_SUCCESS;
            }));

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSensitivity"), _T("{\"index\":\"FP_MD\"}"), response));
        EXPECT_EQ(response,  string("{\"value\":\"high\",\"success\":true}"));
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
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMotionEventsActivePeriod"), _T("{\"nowTime\":1023,\"index\":\"FP_MD\",\"ranges\":[{\"startTime\":\"100\", \"endTime\":\"150\"}]}"), response));
        EXPECT_EQ(response,  string("{\"success\":true}"));
}

