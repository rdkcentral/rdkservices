#pragma once
#include <map>
#include <string>
#include <mutex>
#include <condition_variable>
#include "rialto/ServerManagerServiceFactory.h"
namespace WPEFramework
{
    using namespace rialto::servermanager::service;
    // Shadowing to keep Rialto contained in this
    typedef firebolt::rialto::common::SessionServerState RialtoServerStates;

    class RialtoConnector : public IStateObserver, public std::enable_shared_from_this<RialtoConnector>
    {
    public:
        RialtoConnector() : isInitialized(false) {}
        virtual ~RialtoConnector() = default;
        void initialize(std::string &, const std::string &);
        bool initialized() { return isInitialized; }
        bool waitForStateChange(const std::string &appid, const RialtoServerStates &state, int timeout);
        bool createAppSession(const std::string &callsign, const std::string &displayName, const std::string &appId);
        bool resumeSession(const std::string &callsign);
        bool suspendSession(const std::string &callsign);

        bool deactivateSession(const std::string &callsign);
        void stateChanged(const std::string &appId, const RialtoServerStates &state) override;

        RialtoConnector(const RialtoConnector &) = delete;            // No copying
        RialtoConnector &operator=(const RialtoConnector &) = delete; // No assignment

    private:
        bool isInitialized;
        std::mutex m_stateMutex;
        std::condition_variable m_stateCond;
        std::unique_ptr<IServerManagerService> m_serverManagerService;
        std::map<std::string, RialtoServerStates> appStateMap;

        const RialtoServerStates getCurrentAppState(const std::string &callsign);
    };
} // namespace WPEFramework