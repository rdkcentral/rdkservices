#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;
using namespace std;

/* Contract test verification test class declaration */
class CTVerifier_Test : public L2TestMocks {
protected:
    CTVerifier_Test();
    virtual ~CTVerifier_Test() override;

public:
    void createDeviceFiles();
    void enableTTS();
    // Add all supported plugins here
    vector<string> vtrPlugins = {
        "org.rdk.System"};
};

/**
 * @brief Constructor for CTVerifier_Test class
 */
CTVerifier_Test::CTVerifier_Test()
        : L2TestMocks()
{
    createDeviceFiles();
    /* Activate all plugins */
    for (vector<string>::iterator i = vtrPlugins.begin(); i != vtrPlugins.end(); i++)
    {
        cout << "Activating plugin: " << *i  << endl;
        uint32_t status = ActivateService((*i).c_str());
        EXPECT_EQ(Core::ERROR_NONE, status);
    }
}

/**
 * @brief Destructor for CTVerifier_Test class
 */
CTVerifier_Test::~CTVerifier_Test()
{
    /* Deactivate all plugins */
    for (vector<string>::iterator i = vtrPlugins.begin(); i != vtrPlugins.end(); i++)
    {
        cout << "Deactivating plugin: " << *i  << endl;
        uint32_t status = DeactivateService((*i).c_str());
        EXPECT_EQ(Core::ERROR_NONE, status);
    }
}

/* Contract test verification test class declaration */
class CTVerifier_CallMocks : public CTVerifier_Test {

protected:
    CTVerifier_CallMocks()
    {
    }
    virtual ~CTVerifier_CallMocks() override
    {
    }
};

void CTVerifier_Test::createDeviceFiles()
{
    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties << "BUILD_TYPE=dev\n";
    deviceProperties << "FORCE_MTLS=true\n";
    deviceProperties << "DEVICE_NAME=PLATCO\nDEVICE_TYPE=mediaclient\nMFG_NAME=Element\nWIFI_SUPPORT=true\n";
    deviceProperties << "MOCA_INTERFACE=true\nWIFI_INTERFACE=wlan0\nMOCA_INTERFACE=eth0\nETHERNET_INTERFACE=eth0\n";
    deviceProperties.close();

    std::ofstream version("/version.txt");
    version << "imagename:XUSHTC11MWR_VBN_2401_sprint_20240211231329sdy_NG\n";
    version << "VERSION=7.0.0.0\nBUILD_TIME=2024-02-11 23:13:29\n";
    version.close();
}

class CTVerifierMain : public CTVerifier_CallMocks {
};

/********************************************************
************Test case Details **************************
** 1. Run verify contracts using the pact_verifier_cli
*******************************************************/

TEST_F(CTVerifierMain, VerifyContractsOnAllPlugins)
{
    //cout current directory
    system("pwd");
    //get the git short hash value
    string git_hash_cmd = "git -C ./rdkservices rev-parse --short HEAD";
    string git_hash_str;
    FILE *fp = popen(git_hash_cmd.c_str(), "r");
    if (fp == NULL)
    {
        cout << "Failed to run git hash for rdkservices\n";
    }
    char git_hash[100];
    if (fgets(git_hash, 100, fp) != NULL)
    {
        git_hash_str = git_hash;
        if (!git_hash_str.empty() && git_hash_str[git_hash_str.length()-1] == '\n') {
            git_hash_str.erase(git_hash_str.length()-1);
        }
        cout << "git_hash: " << git_hash_str << "\n";
    }
    pclose(fp);

    string test_pact_cmd = "~/bin/pact_verifier_cli --version";
    system(test_pact_cmd.c_str());

    string pact_verify_cmd = "~/bin/pact_verifier_cli ";
    pact_verify_cmd += "--loglevel=info --provider-name=rdk_service ";
    pact_verify_cmd += "--broker-url=https://skyai.pactflow.io --token=gT7_sLzGNs-ElhMY2pDkBQ ";
    pact_verify_cmd += "--hostname=127.0.0.1 --port=9998 ";
    pact_verify_cmd += "--transport=websocket --publish ";
    pact_verify_cmd += "--provider-version=0.1.0-" + git_hash_str + " --provider-branch=test-github-rdkv ";
    pact_verify_cmd += "--filter-consumer ripple ";
    //pact_verify_cmd += R"(--consumer-version-selectors="{\"tag\": \"0.1.0-4a18973\", \"latest\": true}" )";
    pact_verify_cmd += R"(--consumer-version-selectors="{\"mainBranch\": true}" )";

    cout << "pact_verify_cmd: " << pact_verify_cmd << "\n";
    int stat = system(pact_verify_cmd.c_str());
    cout << "pact_verify_cmd stat: " << stat << "\n";
    EXPECT_GE(stat, 0);

    // while(1)
    // {
    //     std::this_thread::sleep_for(std::chrono::seconds(5));
    //     std::cout << "Sleeping for 5 seconds\n";
    // }


    // uint32_t status = Core::ERROR_GENERAL;
    // JsonObject params;
    // JsonObject result;
    // string strPrint;

    // //getDeviceInfo no params 
    // status = InvokeServiceMethod("org.rdk.System.1", "getDeviceInfo", params, result);
    // result.ToString(strPrint);
    // cout << "getDeviceInfo result1: " << strPrint << "\n";

    // // Test the setValue method.
    // JsonArray arr;
    // JsonObject estb_params;
    // arr.Add("estb_mac");
    // estb_params["params"] = arr;
    // estb_params.ToString(strPrint);
    // cout << "getDeviceInfo params: " << strPrint << "\n";
    // status = InvokeServiceMethod("org.rdk.System.1", "getDeviceInfo", estb_params, result);
    // result.ToString(strPrint);
    // cout << "getDeviceInfo result2: " << strPrint << "\n";

    // JsonObject modelName;
    // modelName["params"] = "modelName";
    // modelName.ToString(strPrint);
    // cout << "getDeviceInfo params - modelName: " << strPrint << "\n";
    // status = InvokeServiceMethod("org.rdk.System.2", "getDeviceInfo", modelName, result);
    // result.ToString(strPrint);
    // cout << "getDeviceInfo - modelName result: " << strPrint << "\n";

    // status = InvokeServiceMethod("org.rdk.System.1", "getSystemVersions", params, result);
    // result.ToString(strPrint);
    // cout << "getSystemVersions result1: " << strPrint << "\n";

}
