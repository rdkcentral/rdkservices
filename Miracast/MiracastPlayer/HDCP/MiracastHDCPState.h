#ifndef __MIRACAST_HDCP_STATE_H__
#define __MIRACAST_HDCP_STATE_H__

#include <SoC_abstraction.h>
#include <string>

using namespace std;

class MiracastHDCPState;

class MiracastHDCPState {
public:
       MiracastHDCPState();
       MiracastHDCPState(std::string hdcp_version,unsigned int port);
       ~MiracastHDCPState();

       static MiracastHDCPState* getInstance(std::string hdcp_version = "none",unsigned int port = 0);
       static void destroyInstance();
       static void WfdHdcpStateEventCallback(SoC_WfdHdcpHandle handle, SoC_WfdHdcpEventType event);
private:
       static MiracastHDCPState *m_p_instance;
       static SoC_WfdHdcpHandle m_handle;
       static SoC_WfdHdcpLevel m_level;
};
#endif //end of __MIRACAST_HDCP_STATE_H__