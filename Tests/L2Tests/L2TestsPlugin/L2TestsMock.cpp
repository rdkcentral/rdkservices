/* If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#include <string>
#include <sstream>

#include "L2TestsMock.h"


#define TEST_CALLSIGN _T("org.rdk.L2Tests.1")  /* Test module callsign. */
#define INVOKE_TIMEOUT 1000                        /* Method invoke timeout in milliseconds. */
#define THUNDER_ADDRESS _T("127.0.0.1:")
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);

#ifndef THUNDER_PORT
#define THUNDER_PORT "9998"
#endif

using namespace WPEFramework;

/* L2TestMock consturctor */
L2TestMocks::L2TestMocks()
{
    p_rfcApiImplMock    = new NiceMock <RfcApiImplMock>;
    p_iarmBusImplMock   = new NiceMock <IarmBusImplMock>;
    p_udevImplMock      = new NiceMock <UdevImplMock>;
    p_wrapsImplMock     = new NiceMock <WrapsImplMock>;
    p_hostImplMock      = new NiceMock <HostImplMock>;
    p_videoOutputPortConfigImplMock = new NiceMock <VideoOutputPortConfigImplMock>;
    p_managerImplMock   = new NiceMock <ManagerImplMock>;
    p_videoOutputPortMock = new NiceMock <VideoOutputPortMock>;
    p_rBusApiImplMock   = new NiceMock <RBusApiImplMock>;
    p_telemetryApiImplMock  = new NiceMock <TelemetryApiImplMock>;

    IarmBus::setImpl(p_iarmBusImplMock);
    RfcApi::setImpl(p_rfcApiImplMock);
    Udev::setImpl(p_udevImplMock);
    Wraps::setImpl(p_wrapsImplMock);
    RBusApi::setImpl(p_rBusApiImplMock);
    TelemetryApi::setImpl(p_telemetryApiImplMock);
    device::Host::setImpl(p_hostImplMock);
    device::VideoOutputPortConfig::setImpl(p_videoOutputPortConfigImplMock);
    device::Manager::setImpl(p_managerImplMock);
    device::VideoOutputPort::setImpl(p_videoOutputPortMock);

    thunder_address = THUNDER_ADDRESS + std::string(THUNDER_PORT);
    (void)Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), thunder_address);
}

/* L2TestMock Destructor */
L2TestMocks::~L2TestMocks()
{
   IarmBus::setImpl(nullptr);
   if (p_iarmBusImplMock != nullptr)
   {
        delete p_iarmBusImplMock;
        p_iarmBusImplMock = nullptr;
   }
   RfcApi::setImpl(nullptr);
   if (p_rfcApiImplMock != nullptr)
   {
        delete p_rfcApiImplMock;
        p_rfcApiImplMock = nullptr;
   }
   RBusApi::setImpl(nullptr);
   if (p_rBusApiImplMock != nullptr)
   {
        delete p_rBusApiImplMock;
        p_rBusApiImplMock = nullptr;
   }

   TelemetryApi::setImpl(nullptr);
   if (p_telemetryApiImplMock != nullptr)
   {
        delete p_telemetryApiImplMock;
        p_telemetryApiImplMock = nullptr;
   }
   device::Host::setImpl(nullptr);
   if (p_hostImplMock != nullptr)
   {
      delete p_hostImplMock;
      p_hostImplMock = nullptr;
   }
   device::VideoOutputPortConfig::setImpl(nullptr);
   if (p_videoOutputPortConfigImplMock != nullptr)
   {
      delete p_videoOutputPortConfigImplMock;
      p_videoOutputPortConfigImplMock = nullptr;
   }
   device::Manager::setImpl(nullptr);
   if (p_managerImplMock != nullptr)
   {
      delete p_managerImplMock;
      p_managerImplMock = nullptr;
   }
   device::VideoOutputPort::setImpl(nullptr);
   if (p_videoOutputPortMock != nullptr)
   {
      delete p_videoOutputPortMock;
      p_videoOutputPortMock = nullptr;
   }

   Udev::setImpl(nullptr);
   if (p_udevImplMock != nullptr)
   {
        delete p_udevImplMock;
        p_udevImplMock = nullptr;
   }
   
   Wraps::setImpl(nullptr);
   if (p_wrapsImplMock != nullptr)
   {
        delete p_wrapsImplMock;
        p_wrapsImplMock = nullptr;
   }
}

/**
 * @brief Invoke a service method
 *
 * @param[in] callsign Service callsign
 * @param[in] method Method name
 * @param[in] params Method parameters
 * @param[out] results Method results
 * @return Zero (Core::ERROR_NONE) on succes or another value on error
 */
uint32_t L2TestMocks::InvokeServiceMethod(const char *callsign, const char *method, JsonObject &params, JsonObject &results)
{

   JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(std::string(callsign), TEST_CALLSIGN);
   std::string message;
   std::string reply;
   uint32_t status = Core::ERROR_NONE;

   params.ToString(message);
   TEST_LOG("Invoking %s.%s, parameters %s\n", callsign, method, message.c_str());

   results = JsonObject();
   status = jsonrpc.Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT, std::string(method), params, results);

   results.ToString(reply);
   TEST_LOG("Status %u, results %s", status, reply.c_str());

   return status;
}

/**
 * @brief Invoke a service method
 *
 * @param[in] callsign Service callsign
 * @param[in] method Method name
 * @param[in] params Method parameters
 * @param[out] results Method results
 * @return Zero (Core::ERROR_NONE) on succes or another value on error
 */
uint32_t L2TestMocks::InvokeServiceMethod(const char *callsign, const char *method, JsonObject &params, Core::JSON::String &results)
{

   JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(std::string(callsign), TEST_CALLSIGN);
   std::string message;
   std::string reply;
   uint32_t status = Core::ERROR_NONE;

   params.ToString(message);
   TEST_LOG("Invoking %s.%s, parameters %s\n", callsign, method, message.c_str());

   status = jsonrpc.Invoke<JsonObject, Core::JSON::String>(INVOKE_TIMEOUT, std::string(method), params, results);

   results.ToString(reply);
   TEST_LOG("Status %u, results %s", status, reply.c_str());

   return status;

}

/**
 * @brief Invoke a service method
 *
 * @param[in] callsign Service callsign
 * @param[in] method Method name
 * @param[in] params Method parameters
 * @param[out] results Method results with string format
 * @return Zero (Core::ERROR_NONE) on succes or another value on error
 */
uint32_t L2TestMocks::InvokeServiceMethod(const char *callsign, const char *method, JsonObject &params, Core::JSON::Boolean &results)
{

   JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(std::string(callsign), TEST_CALLSIGN);
   std::string message;
   std::string reply;
   uint32_t status = Core::ERROR_NONE;

   params.ToString(message);
   TEST_LOG("Invoking %s.%s, parameters %s\n", callsign, method, message.c_str());

   status = jsonrpc.Invoke<JsonObject, Core::JSON::Boolean>(INVOKE_TIMEOUT, std::string(method), params, results);

   results.ToString(reply);
   TEST_LOG("Status %u, results %s", status, reply.c_str());

   return status;

}

/**
 * @brief Invoke a service method
 *
 * @param[in] callsign Service callsign
 * @param[in] method Method name
 * @param[out] results Method results
 * @return Zero (Core::ERROR_NONE) on succes or another value on error
 */
uint32_t L2TestMocks::InvokeServiceMethod(const char *callsign, const char *method, Core::JSON::Boolean &results)
{

   JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(std::string(callsign), TEST_CALLSIGN);
   std::string reply;
   uint32_t status = Core::ERROR_NONE;

   TEST_LOG("Invoking %s.%s \n", callsign, method);

   status = jsonrpc.Invoke<void, Core::JSON::Boolean>(INVOKE_TIMEOUT, std::string(method), results);

   results.ToString(reply);
   TEST_LOG("Status %u, results %s", status, reply.c_str());

   return status;
}

/**
 * @brief Invoke a service method
 *
 * @param[in] callsign Service callsign
 * @param[in] method Method name
 * @param[out] results Method results
 * @return Zero (Core::ERROR_NONE) on succes or another value on error
 */
uint32_t L2TestMocks::InvokeServiceMethod(const char *callsign, const char *method, Core::JSON::String &results)
{

   JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(std::string(callsign), TEST_CALLSIGN);
   std::string reply;
   uint32_t status = Core::ERROR_NONE;

   TEST_LOG("Invoking %s.%s \n", callsign, method);

   status = jsonrpc.Invoke<void, Core::JSON::String>(INVOKE_TIMEOUT, std::string(method), results);

   results.ToString(reply);
   TEST_LOG("Status %u, results %s", status, reply.c_str());

   return status;

}

uint32_t L2TestMocks::InvokeServiceMethod(const char *callsign, const char *method, Core::JSON::Double &results)
{
   JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(std::string(callsign), TEST_CALLSIGN);
   std::string reply;
   uint32_t status = Core::ERROR_NONE;

   TEST_LOG("Invoking %s.%s \n", callsign, method);

   status = jsonrpc.Invoke<void, Core::JSON::Double>(INVOKE_TIMEOUT, std::string(method), results);

   results.ToString(reply);
   TEST_LOG("Status %u, results %s", status, reply.c_str());

   return status;
}

/**
* @brief Activate a service plugin
*
* @param[in] callsign Service callsign
* @return Zero (Core::ERROR_NONE) on succes or another value on error
*/
uint32_t L2TestMocks::ActivateService(const char *callsign)
{
   JsonObject params;
   JsonObject result;
   uint32_t status = Core::ERROR_GENERAL;

   if(callsign != NULL)
   {
     params["callsign"] = callsign;
     status = InvokeServiceMethod("Controller.1", "activate", params, result);
   }

   return status;
}

/**
* @brief Deactivate a service plugin
*
* @param[in] callsign Service callsign
* @return Zero (Core::ERROR_NONE) on succes or another value on error
*/
uint32_t L2TestMocks::DeactivateService(const char *callsign)
{
   JsonObject params;
   JsonObject result;
   uint32_t status = Core::ERROR_GENERAL;

   if(callsign != NULL)
   {
      params["callsign"] = callsign;
      status = InvokeServiceMethod("Controller.1", "deactivate", params, result);
   }
   return status;
}


