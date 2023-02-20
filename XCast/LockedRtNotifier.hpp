#pragma once

/*
This wrapper can be used to synchronize rtremote callbacks in XCast
*/

#include "RtNotifier.h"
#include <mutex>
#include "UtilsSynchro.hpp"

class LockedRtNotifier : public RtNotifier {
    RtNotifier *wrapped;
    std::recursive_mutex& mtx;
public:
    LockedRtNotifier(RtNotifier *wrapped, std::recursive_mutex& mtx) : wrapped(wrapped), mtx(mtx) {
    }
    void onRtServiceDisconnected() {
        std::unique_lock<std::recursive_mutex> lock(mtx);
        wrapped->onRtServiceDisconnected();
    };
    void onXcastApplicationLaunchRequest(string appName, string parameter) {
        std::unique_lock<std::recursive_mutex> lock(mtx);
        wrapped->onXcastApplicationLaunchRequest(appName, parameter);
    };
    void onXcastApplicationLaunchRequestWithLaunchParam (string appName, string strPayLoad, string strQuery, string strAddDataUrl) {
        std::unique_lock<std::recursive_mutex> lock(mtx);
        wrapped->onXcastApplicationLaunchRequestWithLaunchParam(appName, strPayLoad, strQuery, strAddDataUrl);
    };
    void onXcastApplicationStopRequest(string appName, string appID) {
        std::unique_lock<std::recursive_mutex> lock(mtx);
        wrapped->onXcastApplicationStopRequest(appName, appID);
    };
    void onXcastApplicationHideRequest(string appName, string appID) {
        std::unique_lock<std::recursive_mutex> lock(mtx);
        wrapped->onXcastApplicationHideRequest(appName, appID);
    };
    void onXcastApplicationResumeRequest(string appName, string appID) {
        std::unique_lock<std::recursive_mutex> lock(mtx);
        wrapped->onXcastApplicationResumeRequest(appName, appID);
    };
    void onXcastApplicationStateRequest(string appName, string appID) {
        std::unique_lock<std::recursive_mutex> lock(mtx);
        wrapped->onXcastApplicationStateRequest(appName, appID);
    };
};