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

/**
 *  Utility functions used in plugins.
 *
 */

#include <string.h>
#include <sstream>
#include "utils.h"
#include "libIBus.h"

using namespace WPEFramework;
using namespace std;

bool Utils::IARM::init()
{
    static IARMHelper sIARMHelper;
    return m_connected;
}

Utils::IARM::IARMHelper::IARMHelper()
{
    string memberName = "Thunder_Plugins";
    LOGINFO("%s", memberName.c_str());

    IARM_Result_t res;
    int isRegistered = 0;
    IARM_CHECK(IARM_Bus_IsConnected(memberName.c_str(), &isRegistered));

    m_connected = false;
    if (isRegistered > 0)
    {
        LOGINFO("%s has already connected with IARM", memberName.c_str());
        m_connected = true;
        return;
    }

    IARM_CHECK( IARM_Bus_Init(memberName.c_str()));
    if (res == IARM_RESULT_SUCCESS)
    {
        IARM_CHECK(IARM_Bus_Connect());
        if (res != IARM_RESULT_SUCCESS)
        {
            LOGERR("IARM_Bus_Connect failure");
            IARM_CHECK(IARM_Bus_Term());
            return;
        }
    }
    else
    {
        LOGERR("IARM_Bus_Init failure");
        return;
    }

    LOGINFO("%s inited and connected with IARM", memberName.c_str());
    m_connected = true;
}

Utils::IARM::IARMHelper::~IARMHelper()
{
    IARM_Result_t res;
    LOGINFO("disconnect and terminate Thunder_Plugins connection with IARM");
    IARM_CHECK(IARM_Bus_Disconnect());
    IARM_CHECK(IARM_Bus_Term());
	m_connected = false;
}

bool Utils::IARM::m_connected = false;

std::string Utils::formatIARMResult(IARM_Result_t result)
{
    switch (result) {
        case IARM_RESULT_SUCCESS:       return std::string("IARM_RESULT_SUCCESS [success]");
        case IARM_RESULT_INVALID_PARAM: return std::string("IARM_RESULT_INVALID_PARAM [invalid input parameter]");
        case IARM_RESULT_INVALID_STATE: return std::string("IARM_RESULT_INVALID_STATE [invalid state encountered]");
        case IARM_RESULT_IPCCORE_FAIL:  return std::string("IARM_RESULT_IPCORE_FAIL [underlying IPC failure]");
        case IARM_RESULT_OOM:           return std::string("IARM_RESULT_OOM [out of memory]");
        default:
            std::ostringstream tmp;
            tmp << result << " [unknown IARM_Result_t]";
            return tmp.str();
    }
}

/***
 * @brief	: Execute shell script and get response
 * @param1[in]	: script to be executed with args
 * @return		: string; response.
 */
std::string Utils::cRunScript(const char *cmd)
{
    std::string totalStr = "";
    FILE *pipe = NULL;
    char buff[1024] = {'\0'};

    if ((pipe = popen(cmd, "r"))) {
        memset(buff, 0, sizeof(buff));
        while (fgets(buff, sizeof(buff), pipe)) {
            totalStr += buff;
            memset(buff, 0, sizeof(buff));
        }
        pclose(pipe);
    } else {
        /* popen failed. */
    }
    return totalStr;
}

using namespace WPEFramework;

/***
 * @brief	: Checks that file exists
 * @param1[in]	: pFileName name of file
 * @return		: true if file exists.
 */
bool Utils::fileExists(const char *pFileName)
{
    struct stat fileStat;
    return 0 == stat(pFileName, &fileStat);
}

/***
 * @brief	: Checks that file exists and modified at least pointed seconds ago
 * @param1[in]	: pFileName name of file
 * @param1[in]	: age modification age in seconds
 * @return		: true if file exists and modifies 'age' seconds ago.
 */
bool Utils::isFileExistsAndOlderThen(const char *pFileName, long age /*= -1*/)
{
    struct stat fileStat;
    int res = stat(pFileName, &fileStat);
    if (0 != res)
        return false;

    if (-1 == age)
        return true;

    time_t currentTime = time(nullptr);
    //LOGWARN("current time of %s: %lu", pFileName, currentTime);

    time_t modifiedSecondsAgo = difftime(currentTime, fileStat.st_mtime);
    //LOGWARN("elapsed time is %lu, %s", modifiedSecondsAgo, modifiedSecondsAgo <= age ? "updated recently (doesn't exists)" : "updated long time ago (exists)");

    return modifiedSecondsAgo > age;
}

