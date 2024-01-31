#include "MiracastHDCPState.h"
#include "MiracastLogger.h"

#ifdef __cplusplus
extern "C" {
#endif
	void c_WfdHdcpStateEventCallback(SoC_WfdHdcpHandle handle, SoC_WfdHdcpEventType event)
	{
		MiracastHDCPState::getInstance()->WfdHdcpStateEventCallback(handle, event);
	}
#ifdef __cplusplus
}
#endif

MiracastHDCPState *MiracastHDCPState::m_p_instance = nullptr;
SoC_WfdHdcpHandle MiracastHDCPState::m_handle = nullptr;
SoC_WfdHdcpLevel MiracastHDCPState::m_level = SOC_HDCP_LEVEL_NONE;

void MiracastHDCPState::WfdHdcpStateEventCallback(SoC_WfdHdcpHandle handle, SoC_WfdHdcpEventType event)
{
	MIRACASTLOG_TRACE("Entering ...");
	switch(event)
	{
		case SOC_HDCP_EVENT_INITIALIZATION_COMPLETE:
			MIRACASTLOG_INFO("HDCP EVENT INITIALIZATION COMPLETE!");
			break;
		case SOC_HDCP_EVENT_INITIALIZATION_FAILED:
			MIRACASTLOG_ERROR("HDCP EVENT INITIALIZATION FAILED!");
			break;
		case SOC_HDCP_EVENT_SHUTDOWN_COMPLETE:
			MIRACASTLOG_ERROR("HDCP EVENT SHUTDOWN COMPLETE!");
			break;
		case SOC_HDCP_EVENT_SPEC_VERSION_NOT_SUPPORT:
			MIRACASTLOG_ERROR("HDCP EVENT SPEC VERSION NOT SUPPORT!");
			break;
		case SOC_HDCP_EVENT_AUTH_DISCONNECTED:
			MIRACASTLOG_ERROR("HDCP EVENT AUTH DISCONNECTED!");
			break;
		case SOC_HDCP_EVENT_AUTH_ERROR:
			MIRACASTLOG_ERROR("HDCP EVENT AUTH ERROR!");
			break;
		case SOC_HDCP_EVENT_INTERNAL_ERROR:
			MIRACASTLOG_ERROR("HDCP EVENT INTERNAL ERROR!");
			break;
		default:
			MIRACASTLOG_ERROR("Undefine ERROR!");
	}
	MIRACASTLOG_TRACE("Exiting ...");
}

MiracastHDCPState::MiracastHDCPState(std::string hdcp_version,unsigned int port)
{
	SoC_WfdHdcpResultType ret = SOC_HDCP_RESULT_ERROR_INVALID_PARAMETER;
	MIRACASTLOG_TRACE("Entering ...");
	m_handle = nullptr;
	if ( hdcp_version == "HDCP2.0" )
	{
		m_level = SOC_HDCP_LEVEL_21;
	}
	else if ( hdcp_version == "HDCP2.1" )
	{
		m_level = SOC_HDCP_LEVEL_21;
	}
	else
	{
		m_level = SOC_HDCP_LEVEL_NONE;
	}

	ret = SoC_WfdHdcpInit(nullptr,port, &m_handle);
	MIRACASTLOG_TRACE("ret=%d\n", ret);
	switch(ret)
	{
		case SOC_HDCP_RESULT_SUCCESS:
			MIRACASTLOG_INFO("WFD HDCP INTI SUCCESSFULLY!");
			break;
		case SOC_HDCP_RESULT_ERROR_INVALID_PARAMETER:
			MIRACASTLOG_ERROR("WFD HDCP ERROR INVALID PARAMETER!");
			break;
		case SOC_HDCP_RESULT_ERROR_INVALID_ADDRESS:
			MIRACASTLOG_ERROR("WFD HDCP INVALID ADDRESS!");
			break;
		case SOC_HDCP_RESULT_ERROR_OUT_OF_MEM:
			MIRACASTLOG_ERROR("WFD HDCP OUT OF MEM!");
			break;
		case SOC_HDCP_RESULT_ERROR_SPEC_VERSION:
			MIRACASTLOG_ERROR("WFD HDCP SPEC VERSION ERROR!");
			break;
		case SOC_HDCP_RESULT_ERROR_KEY_ERR:
			MIRACASTLOG_ERROR("WFD HDCP KEY ERROR!");
			break;
		case SOC_HDCP_RESULT_ERROR_AUTH_ERR:
			MIRACASTLOG_ERROR("WFD HDCP AUTH ERROR!");
			break;
		case SOC_HDCP_RESULT_ERROR_DISCONNECTED:
			MIRACASTLOG_ERROR("WFD HDCP DISCONNECTED!");
			break;
		case SOC_HDCP_RESULT_ERROR_TEE_ERR:
			MIRACASTLOG_ERROR("WFD HDCP TEE ERR!");
			break;
		case SOC_HDCP_RESULT_ERROR_DECRYPT_ERR:
			MIRACASTLOG_ERROR("WFD HDCP DECRYPT ERR!");
			break;
		case SOC_HDCP_RESULT_ERROR_INTERNAL:
			MIRACASTLOG_ERROR("WFD HDCP INTERNAL ERROR!");
			break;

		default:
			MIRACASTLOG_ERROR("Undefine ERROR!");
	}
	MIRACASTLOG_TRACE("Exiting ...");
}

MiracastHDCPState::~MiracastHDCPState()
{
	MIRACASTLOG_TRACE("Entering ...");
	if(m_handle)
	{
		SoC_WfdHdcpResultType ret = SoC_WfdHdcpDeinit(m_handle);
		MIRACASTLOG_TRACE("SoC_WfdHdcpDeinit ret=%d\n", ret);
		m_handle = nullptr;
	}
	MIRACASTLOG_TRACE("Exiting ...");
}

MiracastHDCPState* MiracastHDCPState::getInstance(std::string hdcp_version,unsigned int port)
{
	MIRACASTLOG_TRACE("Entering ...");
	if(!m_p_instance)
	{
		m_p_instance = new MiracastHDCPState(hdcp_version,port);
		if ( nullptr == m_handle )
		{
			MIRACASTLOG_ERROR("SoC handle not created");
			destroyInstance();
		}
	}
	MIRACASTLOG_TRACE("Exiting ...");
	return m_p_instance;
}

void MiracastHDCPState::destroyInstance()
{
	MIRACASTLOG_TRACE("Entering ...");
	if(m_p_instance)
	{
		delete m_p_instance;
		m_p_instance = nullptr;
	}
	MIRACASTLOG_TRACE("Exiting ...");
}