#ifndef _TTS_RFC_H_
#define _TTS_RFC_H_
#include "TTSCommon.h"
#include "TTSConfiguration.h"

namespace TTS {

class RFCURLObserver {
public:
    static RFCURLObserver* getInstance();
    void triggerRFC(TTSConfiguration*);
    ~RFCURLObserver();

private:
    RFCURLObserver(){};
    RFCURLObserver(const RFCURLObserver&) = delete;
    RFCURLObserver& operator=(const RFCURLObserver&) = delete;

    void fetchURLFromConfig();
    void registerNotification();
	
    void onDeviceMgtUpdateReceivedHandler(const JsonObject& parameters);

    WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>* m_systemService{nullptr};
    bool m_eventRegistered {false};
    TTSConfiguration *m_defaultConfig;
};

}
#endif
