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

#include "CoPilot.h"

#include <algorithm>

#include "libIBus.h"
#include "sysMgr.h"

#include "utils.h"

#define SERVICE_NAME "CoPilotService"

#define COPILOT_METHOD_TERMINATE "terminate"
#define COPILOT_METHOD_GET_STATE "getState"
#define COPILOT_EVT_ON_STATE_CHANGE "onStateChange"

#define DEVICE_PROPERTIES "/etc/device.properties"

#define VNC_MIPSEL "vnc-Comcast-mipsel"
#define VNC_X86 "vnc-Comcast-x86"
#define VNC_ARM "vnc-Comcast-armcortex"

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(CoPilot, 1, 0);

        CoPilot* CoPilot::_instance = nullptr;

        CoPilot::CoPilot()
        : AbstractPlugin()
        {
            LOGINFO();
            CoPilot::_instance = this;

            registerMethod(COPILOT_METHOD_TERMINATE, &CoPilot::terminateWrapper, this);
            registerMethod(COPILOT_METHOD_GET_STATE, &CoPilot::getStateWrapper, this);
        }

        CoPilot::~CoPilot()
        {
            LOGINFO();
            CoPilot::_instance = nullptr;

            if (m_terminator.joinable())
                m_terminator.join();
        }

        void CoPilot::threadRun(CoPilot *coPilot)
        {
            coPilot->terminate();
        }

        uint32_t CoPilot::terminateWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            LOGWARN("Received request to terminate CoPilot");

            // asynchronously
            if (m_terminator.joinable())
                m_terminator.join();

            m_terminator = std::thread(threadRun, this);

            returnResponse(true);
        }

        void CoPilot::terminate()
        {
            const char *processName = getVncName();
            bool firstTry = true;
            int time = 0;
            const int delay = 500;
            const int timeToHardTerminate = 5000;
            do {
                std::string checkScript = "pgrep -f ";
                checkScript += processName;

                int exitStatus = runScript(checkScript.c_str());

                if (exitStatus != 0)
                {
                    LOGINFO("%s is not running", processName);
                    break;
                }

                LOGWARN("terminating %s", processName);

                if (firstTry)
                    stateChanged(CopilotStateTerminating);
                else
                    usleep(delay * 1000);

                std::string killScript = "pkill ";
                killScript += time < timeToHardTerminate ? "" : "-9 ";
                killScript += "-f ";
                killScript += processName;

                exitStatus = runScript(killScript.c_str());

                if (exitStatus != 0)
                {
                    LOGINFO("%s did not terminate (is it running?)", processName);
                }
                else
                {
                    LOGINFO("terminated %s", processName);
                }

                LOGINFO("elapsed %d ms", time);

                firstTry = false;
                time += delay;
            } while (true);
            stateChanged(CopilotStateUninitialized);
        }

        /**
         * @brief Gets the brightness of the specified LED.
         *
         * @param[in] argList List of arguments (Not used).
         *
         * @return Returns a ServiceParams object containing brightness value and function result.
         * @ingroup SERVMGR_CoPilot_API
         */
        uint32_t CoPilot::getStateWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            uint32_t state = getState();
            response["state"] = state;
            returnResponse(true);
        }

        CoPilot::CopilotState CoPilot::getState()
        {
            CopilotState state;

            // synchronously
            const char *processName = getVncName();
            std::string script = "pgrep -f ";
            script += processName;
            int exitStatus = runScript(script.c_str());
            if (exitStatus != 0)
            {
                LOGINFO("%s is not running", processName);
                state = CopilotStateUninitialized;
            }
            else
            {
                LOGINFO("%s is running", processName);
                state = CopilotStateInProgress;
            }
            return state;
        }

        int CoPilot::runScript(const char *script)
        {
            IARM_Bus_SYSMgr_RunScript_t runScriptParam;
            runScriptParam.return_value = -1;
            strcpy(runScriptParam.script_path, script);
            IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_RunScript, &runScriptParam, sizeof(runScriptParam));

            int returnValue = runScriptParam.return_value;
            int exitStatus = returnValue == -1 ? -1 : WEXITSTATUS(returnValue);

            LOGWARN("script '%s' exit status: %d", script, exitStatus);
            return exitStatus;
        }


        const char *CoPilot::getVncName()
        {
            const char *vncName = VNC_MIPSEL;

            FILE *f = fopen(DEVICE_PROPERTIES, "r");
            if (NULL == f)
            {
                LOGERR("Failed to open %s:%s", DEVICE_PROPERTIES, strerror(errno));
                return vncName;
            }

            std::vector <char> buf;
            buf.resize(1024);

            while (fgets(buf.data(), buf.size(), f))
            {
                std::string line(buf.data());
                if (line.find("CPU_ARCH") != std::string::npos)
                {
                    std::size_t eq = line.find_first_of("=");
                    if (std::string::npos != eq)
                    {
                        if (line.find("mipsel", eq) != std::string::npos)
                            vncName = VNC_MIPSEL;
                        if (line.find("x86", eq) != std::string::npos)
                            vncName = VNC_X86;
                        if (line.find("ARM", eq) != std::string::npos)
                            vncName = VNC_ARM;
                    }

                    break;
                }
            }

            fclose(f);

            return vncName;
        }

        void CoPilot::stateChanged(CopilotState state)
        {
            LOGINFO();

            JsonObject params;
            params["state"] = (uint32_t) state;
            sendNotify(COPILOT_EVT_ON_STATE_CHANGE, params);
        }

    } // namespace Plugin
} // namespace WPEFramework
