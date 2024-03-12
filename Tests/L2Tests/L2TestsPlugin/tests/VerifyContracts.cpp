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
    // system("pwd");
    //get short hash from long hash in GITHUB_SHA
    string git_hash_str = "";
    char const *l_hash = getenv("GITHUB_SHA");
    if (l_hash == NULL)
    {
        cout << "GITHUB_SHA is NULL\n";
    }
    else
    {
        cout << "GITHUB_SHA: " << l_hash << "\n";
        std::string s(l_hash);
        git_hash_str = s.substr(0, 7);
    }
    cout << "git_hash: " << git_hash_str << "\n";

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
}
