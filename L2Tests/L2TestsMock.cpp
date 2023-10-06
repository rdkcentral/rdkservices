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
    RfcApi::getInstance().impl = &rfcApiImplMock;
    IarmBus::getInstance().impl = &iarmBusImplMock;
    ProcImpl::getInstance().impl = &ReadprocImplMock;

    thunder_address = THUNDER_ADDRESS + std::string(THUNDER_PORT);
   (void)Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), thunder_address);
}

/* L2TestMock Destructor */
L2TestMocks::~L2TestMocks()
{

   RfcApi::getInstance().impl = nullptr;
   IarmBus::getInstance().impl = nullptr;
   ProcImpl::getInstance().impl = nullptr;
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


