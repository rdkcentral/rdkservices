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

#include <cstdio>
#include <cstdlib>
#include <sys/shm.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <dirent.h>
#include "Module.h"
#include <core/core.h>
#include <plugins/Configuration.h>
#include "L2testController.h"

#ifndef CMAKE_INSTALL_PREFIX
#error CMAKE_INSTALL_PREFIX must be defined
#endif

#define L2TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);

#define THUNDER_ACCESS                  _T("THUNDER_ACCESS")            /* WPEFramework network address variable. */
#define THUNDER_ADDRESS                 "127.0.0.1"                     /* WPEFramework network address. */
#define THUNDER_ADDRESS_LENGTH           25                             /* WPEFramework network address length in bytes. */
#ifndef THUNDER_PORT
#define THUNDER_PORT                    "9998"                          /* Default WPEFramework port (as a string). */
#endif
#define TEST_CALLSIGN                   _T("thunder_tests.1")           /* Test module callsign. */
#define L2TEST_CALLSIGN                   _T("org.rdk.L2Tests.1")       /* L2 Test module callsign. */
#define THUNDER_CMD_LENGTH                 1024                          /* Command length to run thunder in isolation . */

#ifndef TEST_COMPLETION_TIMEOUT
#define TEST_COMPLETION_TIMEOUT                  600000                  /* Overall L2 Test completion timeout in milliseconds. */
#endif

using namespace WPEFramework;
using namespace std;
// initializing instancePtr with NULL
L2testController* L2testController ::instancePtr = NULL;


L2testController* L2testController::getInstance()
{
  /*If there is no instance of class
   then we can create an instance */
  if (instancePtr == NULL)
  {
    instancePtr = new L2testController();
    return instancePtr;
  }
  else
  {
    /* if instancePtr != NULL that means
     the class already have an instance.
     So, we are returning that instance
     and not creating new one.*/
    return instancePtr;
  }
}
bool L2testController::StartThunder()
{
    char command[THUNDER_CMD_LENGTH];
    char address[THUNDER_ADDRESS_LENGTH];

    /* Spawn the Thunder process. */
    snprintf(command, sizeof(command), "WPEFramework -c %s/../etc/WPEFramework/config.json", CMAKE_INSTALL_PREFIX);
    m_fp = popen(command, "w");
    if (nullptr == m_fp)
    {
        printf("Failed to start WPEFramework. Ensure that PATH and LD_LIRARY_PATH are set\n"
               "For example:\n"
               "$ export PATH=`pwd`/install/usr/bin:$PATH\n"
               "$ export LD_LIBRARY_PATH=`pwd`/install/usr/lib:$LD_LIBRARY_PATH\n");
        return false;
    }

    /* Allow time for the server to start. */
    (void)sleep(1U);

    /* Configure the JSONRPC connection, as the test starts before WPEFramework. */
    snprintf(address, sizeof(address), "%s:%s", THUNDER_ADDRESS, THUNDER_PORT);
    WPEFramework::Core::SystemInfo::SetEnvironment(THUNDER_ACCESS, address);

    return true;
}

void L2testController::StopThunder()
{

    if (nullptr != m_fp)
    {
        /* Terminate the Thunder process and wait for it to exit. */
        fprintf(m_fp, "Q\n");
        (void)pclose(m_fp);
        m_fp = nullptr;
    }

}

/**
 * @brief Initialize L2 test, calls first method of L2Tests plugin
 *
 * @param[in] params Method parameters
 * @param[out] results Method results
 * @return Zero (Core::ERROR_NONE) on succes or another value on error
 */
uint32_t L2testController::PerformL2Tests(JsonObject &params, JsonObject &results)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(std::string(L2TEST_CALLSIGN), TEST_CALLSIGN);
    std::string message;
    std::string reply;
    uint32_t status = Core::ERROR_GENERAL;

    params.ToString(message);
    L2TEST_LOG("Invoking %s.parameters %s", L2TEST_CALLSIGN, message.c_str());

    results = JsonObject();
    status = jsonrpc.Invoke<JsonObject, JsonObject>(TEST_COMPLETION_TIMEOUT, std::string(_T("PerformL2Tests")), params, results);

    results.ToString(reply);
    L2TEST_LOG("Status %u, results %s", status, reply.c_str());

    return status;
}

/**
 * @brief Find and replace a string in a file
 *
 * @param[in] fileName File name
 * @return Zero (EXIT_SUCCESS) on success or another value on error
 */
int find_replace_string(string fileName) {
    string filePath = "./install/etc/WPEFramework/plugins/";
    string fileFullName = filePath + fileName;
//    string fileFullName = "./install/etc/WPEFramework/plugins/test.json";
    string search = "\"autostart\":true";
    string replace = "\"autostart\":false";

    std::cout << "fileFullName: " << fileFullName << std::endl;
    // Open the file
    ifstream inputFile(fileFullName, ifstream::binary);
    if (!inputFile) {
        cout << "Error: Unable to open input file." << endl;
        return 1;
    }

    // Read the file into a string
    string content((istreambuf_iterator<char>(inputFile)), (istreambuf_iterator<char>()));
    inputFile.close();

    // Find and replace the string
    size_t pos = content.find(search);
    while (pos != string::npos) {
        content.replace(pos, search.length(), replace);
        pos = content.find(search, pos + replace.length());
    }

    // Open the file for writing
    ofstream outputFile(fileFullName);
    if (!outputFile) {
        cout << "Error: Unable to open output file." << endl;
        return 1;
    }

    // Write the modified content back to the file
    outputFile << content;
    outputFile.close();
    cout << "String replaced successfully." << endl;
    return 0;
}

/**

    * @brief Set autostart to false for all plugins
    * Some of the plugins are autostarted by default and when IARM calls are made in Initialize()
    * it will cause segmentation fault since the iarm mocks are not ready.
    * This function will set autostart to false for all plugins except L2Tests.json
    * @return Zero (EXIT_SUCCESS) on success or another value on error
    *
*/
int setAutostartToFalse()
{
    std::string path = "./install/etc/WPEFramework/plugins/";
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(path.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_type == DT_REG) {
                std::cout << ent->d_name << std::endl;
                if (strcmp("L2Tests.json", ent->d_name) == 0)
                    continue;
                find_replace_string(ent->d_name);
            }
        }
        closedir(dir);
    } else {
        std::cerr << "Error opening directory" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    JsonObject params;
    JsonObject result;
    uint32_t status = Core::ERROR_GENERAL;
    int arguments=1;
    std::string message;
    int return_status = -1;
    L2testController* L2testobj
      = L2testController ::getInstance();

    // Set autostart to false for all plugins except L2Tests.json
    setAutostartToFalse();

    L2TEST_LOG("Starting Thunder");
    if (false == L2testobj->StartThunder())
    {
        L2TEST_LOG("Failed to start thunder \n");
        return return_status;
    }

    L2TEST_LOG("Argument count = %d\n",argc);

    /* L2 tests can be run with different options
    ** RdkServicesL2Test  - To run all suits
    ** RdkServicesL2Test TestSuiteXXXX (seperated with space )
    ** Ex: RdkServicesL2Test Systemservice_L2test --> Runs only systemservice suite test
    **     RdkServicesL2Test Systemservice_L2test UsbAccess_L2test--> Runs systemservice and UsbAccesssuite tests
    **/
    if(argc > 1)  /* First argument is always executable */
    {

        /* To run gtest for specific suite we need to provide gtest filter.
        **  Filter is set inside L2test plugin, so we are sending it as paramter.
        **  Filters can be in following format
        ** ::testing::GTEST_FLAG(filter) = "Testsuit1*:Testsuite2*"
        **/

        message = std::string(argv[arguments]) + std::string("*");
        L2TEST_LOG("Argument passed = %s, message = %s\n",argv[arguments],message.c_str());
        arguments++;
        while(arguments<argc)
        {
           message = (message+std::string(":")+std::string(argv[arguments])+std::string("*"));
           L2TEST_LOG("Argument passed = %s\n",argv[arguments]);
           arguments++;
        }
        params["test_suite_list"] = message;
        L2TEST_LOG("gtest filter = %s\n",message.c_str());
    }
    else
    {
        L2TEST_LOG("No Extra Argument passed");
    }

    L2TEST_LOG("PerformL2Tests from Test Controller");
    status = L2testobj->PerformL2Tests(params, result);
    if(status != Core::ERROR_NONE)
    {
       L2TEST_LOG("Issue with starting L2 test in test plugin");
    }
    else
    {
       L2TEST_LOG("Successfully ran L2 tests");
       return_status = 0;
    }

    L2TEST_LOG("Stoppng Thunder");
    L2testobj->StopThunder();

    return return_status;

}

