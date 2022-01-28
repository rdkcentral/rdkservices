/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
 */

#include <condition_variable>
#include <fstream>
#include <memory>
#include <tuple>
#include <utility>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "Module.h"

#ifdef WEBKIT_GLIB_API
#include <wpe/webkit.h>
#else
#include <WPE/WebKit.h>
#include <WPE/WebKit/WKCookieManagerSoup.h>
#include <WPE/WebKit/WKGeolocationManager.h> // TODO: add ref to this header in WebKit.h?
#include <WPE/WebKit/WKGeolocationPermissionRequest.h>
#include <WPE/WebKit/WKGeolocationPosition.h>
#include <WPE/WebKit/WKNotification.h>
#include <WPE/WebKit/WKNotificationManager.h>
#include <WPE/WebKit/WKNotificationPermissionRequest.h>
#include <WPE/WebKit/WKNotificationProvider.h>
#include <WPE/WebKit/WKPreferencesRef.h>
#include <WPE/WebKit/WKSoupSession.h>
#include <WPE/WebKit/WKUserMediaPermissionRequest.h>
#include <WPE/WebKit/WKErrorRef.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "BrowserConsoleLog.h"
#include "InjectedBundle/Tags.h"
#include "tracing/Logging.h"

#define URL_LOAD_RESULT_TIMEOUT_MS                                   (15 * 1000)

#ifdef __cplusplus
extern "C" {
#endif

typedef pid_t WKProcessID;
typedef void (*WKPageIsWebProcessResponsiveFunction)(bool isWebProcessResponsive, void* context);
WK_EXPORT void WKPageIsWebProcessResponsive(WKPageRef page, void* context, WKPageIsWebProcessResponsiveFunction function);
WK_EXPORT WKProcessID WKPageGetProcessIdentifier(WKPageRef page);

#ifdef __cplusplus
}
#endif

#endif

#include <wpe/wpe.h>

#include <glib.h>

#include "HTML5Notification.h"
#include "WebKitBrowser.h"

namespace WPEFramework {
namespace Plugin {

namespace {

void onURLChangeWorkarounds(const std::string &url, WKPageRef page)
{
    const char* activateKeyboardEventDelay = getenv("ACTIVATE_KEYBOARD_EVENT_DELAY");

    if (activateKeyboardEventDelay) {
        if (url.find("https://www.live.bbctvapps.co.uk/tap/sounds") != std::string::npos) {
            SYSLOG(Logging::Notification, (_T("Activating key event delays")));
            setenv("ACTIVATE_KEYBOARD_EVENT_DELAY", "1", 1);
            WKContextSetEnv(WKPageGetContext(page), WKStringCreateWithUTF8CString("ACTIVATE_KEYBOARD_EVENT_DELAY"), WKStringCreateWithUTF8CString("1"), true, false);
        } else {
            if (activateKeyboardEventDelay[0] != '0') {
                SYSLOG(Logging::Notification, (_T("Deactivating key event delays")));
                setenv("ACTIVATE_KEYBOARD_EVENT_DELAY", "0", 1);
                WKContextSetEnv(WKPageGetContext(page), WKStringCreateWithUTF8CString("ACTIVATE_KEYBOARD_EVENT_DELAY"), WKStringCreateWithUTF8CString("0"), true, false);
            }
        }
    }
    // This is workaround in case of applications which use ShakaPlayer in version >= 3.0.0 and <= 3.0.3
    if (url.find("https://app.10ft.itv.com/virginmedia") != std::string::npos) {
        SYSLOG(Logging::Notification, (_T("Activating convert playready key ID for Shaka")));
        WKContextSetEnv(WKPageGetContext(page), WKStringCreateWithUTF8CString("CONVERT_PLAYREADY_KEY_ID_FOR_SHAKA"), WKStringCreateWithUTF8CString("1"), true, false);
    } else {
        SYSLOG(Logging::Notification, (_T("Deactivating convert playready key ID for Shaka")));
        WKContextSetEnv(WKPageGetContext(page), WKStringCreateWithUTF8CString("CONVERT_PLAYREADY_KEY_ID_FOR_SHAKA"), WKStringCreateWithUTF8CString("0"), true, false);
    }
}

} // namespace

    static string consoleLogPrefix;

#ifndef WEBKIT_GLIB_API
    static void onDidReceiveSynchronousMessageFromInjectedBundle(WKContextRef context, WKStringRef messageName,
        WKTypeRef messageBodyObj, WKTypeRef* returnData, const void* clientInfo);
    static void onNotificationShow(WKPageRef page, WKNotificationRef notification, const void* clientInfo);
    static void didStartProvisionalNavigation(WKPageRef page, WKNavigationRef navigation, WKTypeRef userData, const void* clientInfo);
    static void didFinishDocumentLoad(WKPageRef page, WKNavigationRef navigation, WKTypeRef userData, const void* clientInfo);
    static void onFrameDisplayed(WKViewRef view, const void* clientInfo);
    static void didSameDocumentNavigation(const OpaqueWKPage* page, const OpaqueWKNavigation* nav, unsigned int count, const void* clientInfo, const void* info);
    static void requestClosure(const void* clientInfo);
    static void didRequestAutomationSession(WKContextRef context, WKStringRef sessionID, const void* clientInfo);
    static WKPageRef onAutomationSessionRequestNewPage(WKWebAutomationSessionRef session, const void* clientInfo);
    static void decidePolicyForNavigationResponse(WKPageRef, WKNavigationResponseRef response, WKFramePolicyListenerRef listener, WKTypeRef, const void* clientInfo);
    static void didFailProvisionalNavigation(WKPageRef page, WKNavigationRef, WKErrorRef error, WKTypeRef, const void *clientInfo);
    static void didFailNavigation(WKPageRef page, WKNavigationRef, WKErrorRef error, WKTypeRef, const void *clientInfo);
    static void webProcessDidCrash(WKPageRef page, const void* clientInfo);

    struct GCharDeleter {
        void operator()(gchar* ptr) const { g_free(ptr); }
    };

    // -----------------------------------------------------------------------------------------------------
    // Hide all NASTY C details that come with the POC libraries !!!!!
    // -----------------------------------------------------------------------------------------------------
    static WKPageNavigationClientV0 _handlerWebKit = {
        { 0, nullptr },
        // decidePolicyForNavigationAction
        [](WKPageRef, WKNavigationActionRef, WKFramePolicyListenerRef listener, WKTypeRef, const void*) {
            WKFramePolicyListenerUse(listener);
        },
        decidePolicyForNavigationResponse,
        nullptr, // decidePolicyForPluginLoad
        didStartProvisionalNavigation,
        nullptr, // didReceiveServerRedirectForProvisionalNavigation
        didFailProvisionalNavigation,
        nullptr, // didCommitNavigation
        nullptr, // didFinishNavigation
        didFailNavigation,
        nullptr, // didFailProvisionalLoadInSubframe
        didFinishDocumentLoad,
        didSameDocumentNavigation, // didSameDocumentNavigation
        nullptr, // renderingProgressDidChange
        nullptr, // canAuthenticateAgainstProtectionSpace
        nullptr, // didReceiveAuthenticationChallenge
        webProcessDidCrash,
        nullptr, // copyWebCryptoMasterKey
        nullptr, // didBeginNavigationGesture
        nullptr, // willEndNavigationGesture
        nullptr, // didEndNavigationGesture
        nullptr, // didRemoveNavigationGestureSnapshot
    };

    static WKContextInjectedBundleClientV1 _handlerInjectedBundle = {
        { 1, nullptr },
        nullptr, // didReceiveMessageFromInjectedBundle
        // didReceiveSynchronousMessageFromInjectedBundle
        onDidReceiveSynchronousMessageFromInjectedBundle,
        nullptr, // getInjectedBundleInitializationUserData
    };

    WKGeolocationProviderV0 _handlerGeolocationProvider = {
        { 0, nullptr },
        // startUpdating
        [](WKGeolocationManagerRef geolocationManager, const void*) {
            std::cerr << "in WKGeolocationProviderV0::startUpdating" << std::endl;
            WKGeolocationPositionRef position = WKGeolocationPositionCreate(0.0, 51.49, 4.40, 1.0);
            WKGeolocationManagerProviderDidChangePosition(geolocationManager, position);
        },
        nullptr, // stopUpdating
    };

    WKPageUIClientV8 _handlerPageUI = {
        { 8, nullptr },
        nullptr, // createNewPage_deprecatedForUseWithV0
        nullptr, // showPage
        // close
        [](const OpaqueWKPage*, const void* clientInfo) {
            requestClosure(clientInfo);
        },
        nullptr, // takeFocus
        nullptr, // focus
        nullptr, // unfocus
        nullptr, // runJavaScriptAlert_deprecatedForUseWithV0
        nullptr, // runJavaScriptConfirm_deprecatedForUseWithV0
        nullptr, // runJavaScriptPrompt_deprecatedForUseWithV0
        nullptr, // setStatusText
        nullptr, // mouseDidMoveOverElement_deprecatedForUseWithV0
        nullptr, // missingPluginButtonClicked_deprecatedForUseWithV0
        nullptr, // didNotHandleKeyEvent
        nullptr, // didNotHandleWheelEvent
        nullptr, // toolbarsAreVisible
        nullptr, // setToolbarsAreVisible
        nullptr, // menuBarIsVisible
        nullptr, // setMenuBarIsVisible
        nullptr, // statusBarIsVisible
        nullptr, // setStatusBarIsVisible
        nullptr, // isResizable
        nullptr, // setIsResizable
        nullptr, // getWindowFrame
        nullptr, // setWindowFrame
        nullptr, // runBeforeUnloadConfirmPanel
        nullptr, // didDraw
        nullptr, // pageDidScroll
        nullptr, // exceededDatabaseQuota
        nullptr, // runOpenPanel
        // decidePolicyForGeolocationPermissionRequest
        [](WKPageRef, WKFrameRef, WKSecurityOriginRef, WKGeolocationPermissionRequestRef permissionRequest, const void*) {
            WKGeolocationPermissionRequestAllow(permissionRequest);
        },
        nullptr, // headerHeight
        nullptr, // footerHeight
        nullptr, // drawHeader
        nullptr, // drawFooter
        nullptr, // printFrame
        nullptr, // runModal
        nullptr, // unused1
        nullptr, // saveDataToFileInDownloadsFolder
        nullptr, // shouldInterruptJavaScript_unavailable
        nullptr, // createNewPage_deprecatedForUseWithV1
        nullptr, // mouseDidMoveOverElement
        // decidePolicyForNotificationPermissionRequest
        [](WKPageRef, WKSecurityOriginRef, WKNotificationPermissionRequestRef permissionRequest, const void*) {
            WKNotificationPermissionRequestAllow(permissionRequest);
        },
        nullptr, // unavailablePluginButtonClicked_deprecatedForUseWithV1
        nullptr, // showColorPicker
        nullptr, // hideColorPicker
        nullptr, // unavailablePluginButtonClicked
        nullptr, // pinnedStateDidChange
        nullptr, // didBeginTrackingPotentialLongMousePress
        nullptr, // didRecognizeLongMousePress
        nullptr, // didCancelTrackingPotentialLongMousePress
        nullptr, // isPlayingAudioDidChange
        // decidePolicyForUserMediaPermissionRequest
        [](WKPageRef, WKFrameRef, WKSecurityOriginRef, WKSecurityOriginRef, WKUserMediaPermissionRequestRef permission, const void*) {
            auto audioDevices = WKUserMediaPermissionRequestAudioDeviceUIDs(permission);
            auto videoDevices = WKUserMediaPermissionRequestVideoDeviceUIDs(permission);
            auto audioDevice = WKStringCreateWithUTF8CString("NO AUDIO DEVICE FOUND");
            if (WKArrayGetSize(audioDevices) > 0)
                audioDevice = static_cast<WKStringRef>(WKArrayGetItemAtIndex(audioDevices, 0));
            auto videoDevice = WKStringCreateWithUTF8CString("NO VIDEO DEVICE FOUND");
            if (WKArrayGetSize(videoDevices) > 0)
                videoDevice = static_cast<WKStringRef>(WKArrayGetItemAtIndex(videoDevices, 0));
            WKUserMediaPermissionRequestAllow(permission, audioDevice, videoDevice);
        },
        nullptr, // didClickAutoFillButton
        nullptr, // runJavaScriptAlert
        nullptr, // runJavaScriptConfirm
        nullptr, // runJavaScriptPrompt
        nullptr, // mediaSessionMetadataDidChange
        nullptr, // createNewPage
        nullptr, // runJavaScriptAlert
        nullptr, // runJavaScriptConfirm
        nullptr, // runJavaScriptPrompt
        nullptr, // checkUserMediaPermissionForOrigin
        nullptr, // runBeforeUnloadConfirmPanel
        nullptr, // fullscreenMayReturnToInline
        // willAddDetailedMessageToConsole
        [](WKPageRef, WKStringRef, WKStringRef, uint64_t line, uint64_t column, WKStringRef message, WKStringRef url, const void* clientInfo) {
          if (WPEFramework::Trace::TraceType<BrowserConsoleLog, &WPEFramework::Core::System::MODULE_NAME>::IsEnabled() == false)
            return;
          string urlStr = WebKit::Utils::WKStringToString(url);
          string messageStr = WebKit::Utils::WKStringToString(message);
          fprintf(stderr, "[%s]:%s:%llu,%llu %s\n", consoleLogPrefix.c_str(), Core::FileNameOnly(urlStr.c_str()), line, column, messageStr.c_str());
        },
    };

    WKNotificationProviderV0 _handlerNotificationProvider = {
        { 0, nullptr },
        // show
        onNotificationShow,
        nullptr, // cancel
        nullptr, // didDestroyNotification
        nullptr, // addNotificationManager
        nullptr, // removeNotificationManager
        nullptr, // notificationPermissions
        nullptr, // clearNotifications
    };

    WKViewClientV0 _viewClient = {
        { 0, nullptr },
        // frameDisplayed
        onFrameDisplayed,
    };

    WKContextAutomationClientV0 _handlerAutomation = {
        { 0, nullptr },
        // allowsRemoteAutomation
        [](WKContextRef, const void*) -> bool {
            return true;
        },
        didRequestAutomationSession,
        // browserName
        [](WKContextRef, const void*) -> WKStringRef {
            return WKStringCreateWithUTF8CString("WPEWebKitBrowser");
        },
        // browserVersion
        [](WKContextRef, const void*) -> WKStringRef {
            return WKStringCreateWithUTF8CString("1.0");
        }
    };

    WKWebAutomationsessionClientV0 _handlerAutomationSession = {
        { 0, nullptr },
        // requestNewPage
        onAutomationSessionRequestNewPage
    };

    static string WKStringToString(WKStringRef wkStringRef) {
        size_t bufferSize = WKStringGetMaximumUTF8CStringSize(wkStringRef);
        std::unique_ptr<char[]> buffer(new char[bufferSize]);
        size_t stringLength = WKStringGetUTF8CString(wkStringRef, buffer.get(), bufferSize);
        return Core::ToString(buffer.get(), stringLength - 1);
    }
    static std::vector<string> ConvertWKArrayToStringVector(WKArrayRef array) {
            size_t arraySize = WKArrayGetSize(array);

            std::vector<string> stringVector;

            stringVector.reserve(arraySize);
            for (unsigned int index = 0; index < arraySize; ++index) {
                stringVector.emplace_back(WKStringToString(static_cast<WKStringRef>(WKArrayGetItemAtIndex(array, index))));
            }

            return stringVector;
    }

    static string GetPageActiveURL(WKPageRef page)
    {
        string activeURL;
        WKURLRef urlRef = WKPageCopyActiveURL(page);
        if (urlRef) {
            WKStringRef urlStringRef = WKURLCopyString(urlRef);
            activeURL = WKStringToString(urlStringRef);
            WKRelease(urlStringRef);
            WKRelease(urlRef);
        }
        return activeURL;
    }

    /* ---------------------------------------------------------------------------------------------------
struct CustomLoopHandler
{
    GSource source;
    uint32_t attentionPending;
};
static gboolean source_prepare(GSource*, gint*)
{
    return (false);
}
static gboolean source_check(GSource* mySource)
{
    return (static_cast<CustomLoopHandler*>(mySource)->attentionPending != 0);
}
static gboolean source_dispatch (GSource*, GSourceFunc callback, gpointer)
{
    uint32_t attention (static_cast<CustomLoopHandler*>(mySource)->attentionPending);

}
static GSourceFuncs _handlerIntervention =
{
    source_prepare,
    source_check,
    source_dispatch,
    nullptr
};
--------------------------------------------------------------------------------------------------- */
#endif // !WEBKIT_GLIB_API

    static Exchange::IWebBrowser* implementation = nullptr;

    static void CloseDown()
    {
        // Seems we are destructed.....If we still have a pointer to the implementation, Kill it..
        if (implementation != nullptr) {
            delete implementation;
            implementation = nullptr;
        }
    }

    class WebKitImplementation : public Core::Thread,
                                 public Exchange::IBrowser,
                                 public Exchange::IWebBrowser,
                                 public Exchange::IBrowserResources,
                                 public Exchange::IBrowserSecurity,
                                 public Exchange::IApplication,
                                 public PluginHost::IStateControl {
    public:
        class BundleConfig : public Core::JSON::Container {
        private:
            using BundleConfigMap = std::map<string, Core::JSON::String>;

        public:
            using Iterator = Core::IteratorMapType<const BundleConfigMap, const Core::JSON::String&, const string&, BundleConfigMap::const_iterator>;

            BundleConfig(const BundleConfig&) = delete;
            BundleConfig& operator=(const BundleConfig&) = delete;

            BundleConfig()
                : _configs()
            {
            }
            ~BundleConfig() override
            {
            }

            inline bool Config(const string& index, string& value) const
            {
                BundleConfigMap::const_iterator position(_configs.find(index));
                bool result = (position != _configs.cend());

                if (result == true) {
                    value = position->second.Value();
                }

                return (result);
            }

        private:
            bool Request(const TCHAR label[]) override
            {
                if (_configs.find(label) == _configs.end()) {
                    auto element = _configs.emplace(std::piecewise_construct,
                        std::forward_as_tuple(label),
                        std::forward_as_tuple());
                    Add(element.first->first.c_str(), &(element.first->second));
                }
                return (true);
            }

        private:
            BundleConfigMap _configs;
        };
        class Config : public Core::JSON::Container {
        private:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

        public:
            class JavaScriptSettings : public Core::JSON::Container {
            public:
                JavaScriptSettings(const JavaScriptSettings&) = delete;
                JavaScriptSettings& operator=(const JavaScriptSettings&) = delete;

                JavaScriptSettings()
                    : Core::JSON::Container()
                    , UseLLInt(true)
                    , UseJIT(true)
                    , UseDFG(true)
                    , UseFTL(true)
                    , UseDOM(true)
                    , DumpOptions(_T("0"))
                {
                    Add(_T("useLLInt"), &UseLLInt);
                    Add(_T("useJIT"), &UseJIT);
                    Add(_T("useDFG"), &UseDFG);
                    Add(_T("useFTL"), &UseFTL);
                    Add(_T("useDOM"), &UseDOM);
                    Add(_T("dumpOptions"), &DumpOptions);
                }
                ~JavaScriptSettings()
                {
                }

            public:
                Core::JSON::Boolean UseLLInt;
                Core::JSON::Boolean UseJIT;
                Core::JSON::Boolean UseDFG;
                Core::JSON::Boolean UseFTL;
                Core::JSON::Boolean UseDOM;
                Core::JSON::String DumpOptions;
            };

        public:
            class SecurityProfileProperty : public Core::JSON::Container {
            private:
                SecurityProfileProperty& operator=(const SecurityProfileProperty&) = delete;

            public:
                SecurityProfileProperty()
                    : Core::JSON::Container()
                    , Name()
                    , CipherPrio()
                {
                    Add(_T("name"), &Name);
                    Add(_T("cipherprio"), &CipherPrio);
                }
                SecurityProfileProperty(const SecurityProfileProperty& rhs)
                    : Core::JSON::Container()
                    , Name(rhs.Name)
                    , CipherPrio(rhs.CipherPrio)
                {
                    Add(_T("name"), &Name);
                    Add(_T("cipherprio"), &CipherPrio);
                }

            public:
                Core::JSON::String Name;
                Core::JSON::String CipherPrio;
            };

        public:
            Config()
                : Core::JSON::Container()
                , WebkitDebug()
                , GstDebug()
                , GstNoColor()
                , MixedContentWhitelist()
                , UserAgent()
                , URL(_T("http://www.google.com"))
                , Whitelist()
                , PageGroup(_T("WPEPageGroup"))
                , CookieStorage()
                , LocalStorage()
                , LocalStorageEnabled(false)
                , LocalStorageSize()
                , Secure(false)
                , InjectedBundle()
                , Transparent(false)
                , Compositor()
                , Inspector()
                , FPS(false)
                , Cursor(false)
                , Touch(false)
                , MSEBuffers()
                , ThunderDecryptorPreference()
                , MemoryProfile()
                , MemoryPressure()
                , MediaContentTypesRequiringHardwareSupport()
                , MediaDiskCache(true)
                , DiskCache()
                , DiskCacheDir()
                , XHRCache(false)
                , Languages()
                , CertificateCheck(true)
                , ClientIdentifier()
                , AllowWindowClose(false)
                , NonCompositedWebGLEnabled(false)
                , EnvironmentOverride(false)
                , Automation(false)
                , WebGLEnabled(true)
                , ThreadedPainting()
                , Width(1280)
                , Height(720)
                , PTSOffset(0)
                , ScaleFactor(1.0)
                , MaxFPS(60)
                , ExecPath()
                , HTTPProxy()
                , HTTPProxyExclusion()
                , TCPKeepAlive(false)
                , ClientCert()
                , ClientCertKey()
                , LogToSystemConsoleEnabled(false)
                , WatchDogCheckTimeoutInSeconds(0)
                , WatchDogHangThresholdInSeconds(0)
                , LoadBlankPageOnSuspendEnabled(false)
            {
                Add(_T("webkitdebug"), &WebkitDebug);
                Add(_T("gstdebug"), &GstDebug);
                Add(_T("gstnocolor"), &GstNoColor);
                Add(_T("mixedcontentwhitelist"), &MixedContentWhitelist);
                Add(_T("useragent"), &UserAgent);
                Add(_T("url"), &URL);
                Add(_T("whitelist"), &Whitelist);
                Add(_T("pagegroup"), &PageGroup);
                Add(_T("cookiestorage"), &CookieStorage);
                Add(_T("localstorage"), &LocalStorage);
                Add(_T("localstorageenabled"), &LocalStorageEnabled);
                Add(_T("localstoragesize"), &LocalStorageSize);
                Add(_T("secure"), &Secure);
                Add(_T("injectedbundle"), &InjectedBundle);
                Add(_T("transparent"), &Transparent);
                Add(_T("compositor"), &Compositor);
                Add(_T("inspector"), &Inspector);
                Add(_T("fps"), &FPS);
                Add(_T("cursor"), &Cursor);
                Add(_T("touch"), &Touch);
                Add(_T("msebuffers"), &MSEBuffers);
                Add(_T("thunderdecryptorpreference"), &ThunderDecryptorPreference);
                Add(_T("memoryprofile"), &MemoryProfile);
                Add(_T("memorypressure"), &MemoryPressure);
                Add(_T("mediacontenttypesrequiringhardwaresupport"), &MediaContentTypesRequiringHardwareSupport);
                Add(_T("mediadiskcache"), &MediaDiskCache);
                Add(_T("diskcache"), &DiskCache);
                Add(_T("diskcachedir"), &DiskCacheDir);
                Add(_T("xhrcache"), &XHRCache);
                Add(_T("languages"), &Languages);
                Add(_T("certificatecheck"), &CertificateCheck);
                Add(_T("javascript"), &JavaScript);
                Add(_T("clientidentifier"), &ClientIdentifier);
                Add(_T("windowclose"), &AllowWindowClose);
                Add(_T("noncompositedwebgl"), &NonCompositedWebGLEnabled);
                Add(_T("environmentoverride"), &EnvironmentOverride);
                Add(_T("automation"), &Automation);
                Add(_T("webgl"), &WebGLEnabled);
                Add(_T("threadedpainting"), &ThreadedPainting);
                Add(_T("width"), &Width);
                Add(_T("height"), &Height);
                Add(_T("ptsoffset"), &PTSOffset);
                Add(_T("scalefactor"), &ScaleFactor);
                Add(_T("maxfps"), &MaxFPS);
                Add(_T("bundle"), &Bundle);
                Add(_T("execpath"), &ExecPath);
                Add(_T("proxy"), &HTTPProxy);
                Add(_T("proxyexclusion"), &HTTPProxyExclusion);
                Add(_T("tcpkeepalive"), &TCPKeepAlive);
                Add(_T("clientcert"), &ClientCert);
                Add(_T("clientcertkey"), &ClientCertKey);
                Add(_T("logtosystemconsoleenabled"), &LogToSystemConsoleEnabled);
                Add(_T("watchdogchecktimeoutinseconds"), &WatchDogCheckTimeoutInSeconds);
                Add(_T("watchdoghangthresholdtinseconds"), &WatchDogHangThresholdInSeconds);
                Add(_T("loadblankpageonsuspendenabled"), &LoadBlankPageOnSuspendEnabled);
                Add(_T("securityprofiles"), &SecurityProfiles);
            }
            ~Config()
            {
            }

        public:
            Core::JSON::String WebkitDebug;
            Core::JSON::String GstDebug;
            Core::JSON::Boolean GstNoColor;
            Core::JSON::String MixedContentWhitelist;
            Core::JSON::String UserAgent;
            Core::JSON::String URL;
            Core::JSON::String Whitelist;
            Core::JSON::String PageGroup;
            Core::JSON::String CookieStorage;
            Core::JSON::String LocalStorage;
            Core::JSON::Boolean LocalStorageEnabled;
            Core::JSON::DecUInt16 LocalStorageSize;
            Core::JSON::Boolean Secure;
            Core::JSON::String InjectedBundle;
            Core::JSON::Boolean Transparent;
            Core::JSON::String Compositor;
            Core::JSON::String Inspector;
            Core::JSON::Boolean FPS;
            Core::JSON::Boolean Cursor;
            Core::JSON::Boolean Touch;
            Core::JSON::String MSEBuffers;
            Core::JSON::Boolean ThunderDecryptorPreference;
            Core::JSON::String MemoryProfile;
            Core::JSON::String MemoryPressure;
            Core::JSON::String MediaContentTypesRequiringHardwareSupport;
            Core::JSON::Boolean MediaDiskCache;
            Core::JSON::String DiskCache;
            Core::JSON::String DiskCacheDir;
            Core::JSON::Boolean XHRCache;
            Core::JSON::ArrayType<Core::JSON::String> Languages;
            Core::JSON::Boolean CertificateCheck;
            JavaScriptSettings JavaScript;
            Core::JSON::String ClientIdentifier;
            Core::JSON::Boolean AllowWindowClose;
            Core::JSON::Boolean NonCompositedWebGLEnabled;
            Core::JSON::Boolean EnvironmentOverride;
            Core::JSON::Boolean Automation;
            Core::JSON::Boolean WebGLEnabled;
            Core::JSON::String ThreadedPainting;
            Core::JSON::DecUInt16 Width;
            Core::JSON::DecUInt16 Height;
            Core::JSON::DecSInt16 PTSOffset;
            Core::JSON::DecUInt16 ScaleFactor;
            Core::JSON::DecUInt8 MaxFPS; // A value between 1 and 100...
            BundleConfig Bundle;
            Core::JSON::String ExecPath;
            Core::JSON::String HTTPProxy;
            Core::JSON::String HTTPProxyExclusion;
            Core::JSON::Boolean TCPKeepAlive;
            Core::JSON::String ClientCert;
            Core::JSON::String ClientCertKey;
            Core::JSON::Boolean LogToSystemConsoleEnabled;
            Core::JSON::DecUInt16 WatchDogCheckTimeoutInSeconds;   // How often to check main event loop for responsiveness
            Core::JSON::DecUInt16 WatchDogHangThresholdInSeconds;  // The amount of time to give a process to recover before declaring a hang state
            Core::JSON::Boolean LoadBlankPageOnSuspendEnabled;
            Core::JSON::ArrayType<SecurityProfileProperty> SecurityProfiles;
        };

#ifndef WEBKIT_GLIB_API

        class HangDetector
        {
        private:
            WebKitImplementation& _browser;
            GSource* _timerSource { nullptr };
            std::atomic_int _expiryCount { 0 };

            int _watchDogTimeoutInSeconds { 0 };
            int _watchDogTresholdInSeconds { 0 };

            friend Core::ThreadPool::JobType<HangDetector&>;
            Core::WorkerPool::JobType<HangDetector&> _worker;

            void CheckResponsiveness()
            {
                _expiryCount = 0;
                _browser.CheckWebProcess();
            }

            void Dispatch()
            {
                ++_expiryCount;

                if ( _expiryCount > (_watchDogTresholdInSeconds /  _watchDogTimeoutInSeconds) ) {
                    _browser.DeactivateBrowser(PluginHost::IShell::WATCHDOG_EXPIRED);
                }

                _worker.Schedule(Core::Time::Now().Add(_watchDogTimeoutInSeconds * 1000));
            }

        public:
            ~HangDetector()
            {
                _expiryCount = 0;

                if (_timerSource) {
                    g_source_destroy (_timerSource);
                    g_source_unref (_timerSource);
                }
            }

            HangDetector(WebKitImplementation& browser)
                : _browser(browser)
                , _worker(*this)
            {
                _watchDogTimeoutInSeconds = _browser._config.WatchDogCheckTimeoutInSeconds.Value();
                _watchDogTresholdInSeconds  = _browser._config.WatchDogHangThresholdInSeconds.Value();

                if (_watchDogTimeoutInSeconds == 0 || _watchDogTresholdInSeconds == 0)
                    return;

                GMainContext* ctx = _browser._context;
                _timerSource = g_timeout_source_new_seconds ( _watchDogTimeoutInSeconds );

                g_source_set_callback (
                    _timerSource,
                    [](gpointer data) -> gboolean
                    {
                        static_cast<HangDetector*>(data)->CheckResponsiveness();
                        return G_SOURCE_CONTINUE;
                    },
                    this,
                    nullptr
                    );
                g_source_attach ( _timerSource, ctx );

                #if 0
                auto hangSource = g_timeout_source_new_seconds ( 5 );
                g_source_set_callback (
                    hangSource,
                    [](gpointer data) -> gboolean
                    {
                        g_usleep( G_MAXULONG );
                        return G_SOURCE_REMOVE;
                    },
                    this,
                    nullptr
                    );
                g_source_attach ( hangSource, ctx );
                #endif

                _worker.Schedule(Core::Time::Now().Add(_watchDogTimeoutInSeconds * 1000));
            }

            HangDetector(const HangDetector&) = delete;
            HangDetector& operator=(const HangDetector&) = delete;
        };

#endif //WEBKIT_GLIB_API

    private:
        WebKitImplementation(const WebKitImplementation&) = delete;
        WebKitImplementation& operator=(const WebKitImplementation&) = delete;

    public:
        WebKitImplementation()
            : Core::Thread(0, _T("WebKitBrowser"))
            , _config()
            , _dataPath()
            , _service(nullptr)
            , _headers()
            , _localStorageEnabled(false)
            , _httpStatusCode(-1)
#ifdef WEBKIT_GLIB_API
            , _view(nullptr)
            , _guid(Core::Time::Now().Ticks())
#else
            , _view()
            , _page()
            , _pageGroup()
            , _automationSession(nullptr)
            , _notificationManager()
            , _httpCookieAcceptPolicy(kWKHTTPCookieAcceptPolicyOnlyFromMainDocumentDomain)
#endif
            , _adminLock()
            , _fps(0)
            , _loop(nullptr)
            , _context(nullptr)
            , _notificationClients()
            , _notificationBrowserClients()
            , _stateControlClients()
            , _state(PluginHost::IStateControl::UNINITIALIZED)
            , _hidden(false)
            , _time(0)
            , _compliant(false)
            , _configurationCompleted(false)
            , _webProcessCheckInProgress(false)
            , _unresponsiveReplyNum(0)
            , _frameCount(0)
            , _lastDumpTime(g_get_monotonic_time())
            , _allowMixedContent(true)
            , _userScripts()
            , _userStyleSheets()
            , _securityProfileName()
        {
            // Register an @Exit, in case we are killed, with an incorrect ref count !!
            if (atexit(CloseDown) != 0) {
                TRACE(Trace::Information, (_T("Could not register @exit handler. Error: %d."), errno));
                exit(EXIT_FAILURE);
            }

            // The WebKitBrowser (WPE) can only be instantiated once (it is a process wide singleton !!!!)
            ASSERT(implementation == nullptr);

            // Initialize ODH reporting for WebKitBrowser
            if (odh_error_report_init("WebKitBrowser"))
                TRACE(Trace::Error, (_T("Failed to initialize ODH reporting")));

            implementation = this;
            TRACE_L1("%p", this);
        }
        ~WebKitImplementation() override
        {
            TRACE_L1("%p", this);
            Block();

            odh_error_report_deinit(ODH_ERROR_REPORT_DEINIT_MODE_DEFERRED);

            if (_loop != nullptr)
                g_main_loop_quit(_loop);

            if (Wait(Core::Thread::STOPPED | Core::Thread::BLOCKED, 6000) == false)
                TRACE(Trace::Information, (_T("Bailed out before the end of the WPE main app was reached. %d"), 6000));

            implementation = nullptr;
        }

    public:
#ifdef WEBKIT_GLIB_API
        uint32_t Headers(IStringIterator*& header) const override { return Core::ERROR_UNAVAILABLE; }
        uint32_t Headers(IStringIterator* const header) override { return Core::ERROR_UNAVAILABLE; }
        uint32_t UserScripts(IStringIterator*& uris) const override { return Core::ERROR_UNAVAILABLE; }
        uint32_t UserScripts(IStringIterator* const uris) override { return Core::ERROR_UNAVAILABLE; }
        uint32_t UserStyleSheets(IStringIterator*& uris) const override { return Core::ERROR_UNAVAILABLE; }
        uint32_t UserStyleSheets(IStringIterator* const uris) override { return Core::ERROR_UNAVAILABLE; }
        uint32_t HeaderList(string& headerlist) const override { return Core::ERROR_UNAVAILABLE; }
        uint32_t HeaderList(const string& headerlist) override { return Core::ERROR_UNAVAILABLE; }
        uint32_t UserAgent(string& ua) const override { return Core::ERROR_UNAVAILABLE; }
        uint32_t UserAgent(const string& ua) override { return Core::ERROR_UNAVAILABLE; }
        uint32_t LocalStorageEnabled(bool& enabled) const override { return Core::ERROR_UNAVAILABLE; }
        uint32_t LocalStorageEnabled(const bool enabled) override { return Core::ERROR_UNAVAILABLE; }
        uint32_t HTTPCookieAcceptPolicy(HTTPCookieAcceptPolicyType& policy) const override { return Core::ERROR_UNAVAILABLE; }
        uint32_t HTTPCookieAcceptPolicy(const HTTPCookieAcceptPolicyType policy) override { return Core::ERROR_UNAVAILABLE; }
        uint32_t BridgeReply(const string& payload) override { return Core::ERROR_UNAVAILABLE; }
        uint32_t BridgeEvent(const string& payload) override { return Core::ERROR_UNAVAILABLE; }
        uint32_t MixedContentPolicy(MixedContentPolicyType& policy) const override { return Core::ERROR_UNAVAILABLE; }
        uint32_t MixedContentPolicy(const MixedContentPolicyType policy) override { return Core::ERROR_UNAVAILABLE; }
#else
        uint32_t Headers(IStringIterator*& header) const override
        {
            return Core::ERROR_NONE;
        }

        uint32_t Headers(IStringIterator* const header) override
        {
            return Core::ERROR_NONE;
        }

       uint32_t UserScripts(IStringIterator*& uris) const override
        {
            _adminLock.Lock();
            uris = Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(_userScripts);
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t UserScripts(IStringIterator* const uris) override
        {
            string entry;
            std::vector<string> userScriptsContent;
            std::list<string> userScriptsUris;
            while (uris->Next(entry)) {
                auto content = GetFileContent(entry);
                if (!content.empty()) {
                        userScriptsUris.push_back(entry);
                        userScriptsContent.push_back(content);
                }
                TRACE_L1("Adding user's script (uri: %s, empty: %d)", entry.c_str(), content.empty());
            }
            using SetUserScriptsData = std::tuple<WebKitImplementation*, std::list<string>, std::vector<string>>;
            auto* data = new SetUserScriptsData(this, userScriptsUris, userScriptsContent);

            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<SetUserScriptsData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    std::list<string> scriptsUris = std::get<1>(data);
                    std::vector<string> scriptsContent = std::get<2>(data);

                    object->_adminLock.Lock();
                    object->_userScripts = scriptsUris;
                    object->_adminLock.Unlock();

                    // Remove all user scripts
                    WKPageGroupRemoveAllUserScripts(object->_pageGroup);

                    for (string entry : scriptsContent) {
                        WKPageGroupAddUserScript(
                                object->_pageGroup,
                                WKStringCreateWithUTF8CString(entry.c_str()),
                                nullptr,
                                nullptr,
                                nullptr,
                                kWKInjectInTopFrameOnly,
                                kWKInjectAtDocumentStart);
                    }

                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<SetUserScriptsData*>(customdata);
                });

            return Core::ERROR_NONE;
        }

        uint32_t UserStyleSheets(IStringIterator*& uris) const override
        {
            _adminLock.Lock();
            uris = Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(_userStyleSheets);
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }
        uint32_t UserStyleSheets(IStringIterator* const uris) override
        {
            string entry;
            std::vector<string> userStyleSheetsContent;
            std::list<string> userStyleSheetsUris;
            while (uris->Next(entry)) {
                auto content = GetFileContent(entry);
                if (!content.empty()) {
                        userStyleSheetsUris.push_back(entry);
                        userStyleSheetsContent.push_back(content);
                }
                TRACE_L1("Adding user's style sheet (uri: %s, empty: %d)", entry.c_str(), content.empty());
            }
            using SetUserStyleSheetsData = std::tuple<WebKitImplementation*, std::list<string>, std::vector<string>>;
            auto* data = new SetUserStyleSheetsData(this, userStyleSheetsUris, userStyleSheetsContent);

            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<SetUserStyleSheetsData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    std::list<string> styleSheetsUris = std::get<1>(data);
                    std::vector<string> styleSheetsContent = std::get<2>(data);

                    object->_adminLock.Lock();
                    object->_userStyleSheets = styleSheetsUris;
                    object->_adminLock.Unlock();

                    // Remove all style sheets
                    WKPageGroupRemoveAllUserStyleSheets(object->_pageGroup);

                    for (string entry : styleSheetsContent) {
                        WKPageGroupAddUserStyleSheet(
                                object->_pageGroup,
                                WKStringCreateWithUTF8CString(entry.c_str()),
                                nullptr,
                                nullptr,
                                nullptr,
                                kWKInjectInTopFrameOnly);
                    }

                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<SetUserStyleSheetsData*>(customdata);
                });

            return Core::ERROR_NONE;
        }

        uint32_t HeaderList(string& headerlist) const override
        {
            _adminLock.Lock();
            headerlist = _headers;
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t HeaderList(const string& headerlist) override
        {
            if (_context != nullptr) {
                using SetHeadersData = std::tuple<WebKitImplementation*, string>;
                auto* data = new SetHeadersData(this, headerlist);

                g_main_context_invoke_full(
                    _context,
                    G_PRIORITY_DEFAULT,
                    [](gpointer customdata) -> gboolean {
                        auto& data = *static_cast<SetHeadersData*>(customdata);
                        WebKitImplementation* object = std::get<0>(data);
                        const string& headers = std::get<1>(data);

                        object->_adminLock.Lock();
                        object->_headers = headers;
                        object->_adminLock.Unlock();

                        auto messageName = WKStringCreateWithUTF8CString(Tags::Headers);
                        auto messageBody = WKStringCreateWithUTF8CString(headers.c_str());

                        WKPagePostMessageToInjectedBundle(object->_page, messageName, messageBody);

                        WKRelease(messageBody);
                        WKRelease(messageName);

                        return G_SOURCE_REMOVE;
                    },
                    data,
                    [](gpointer customdata) {
                        delete static_cast<SetHeadersData*>(customdata);
                    });
            }

            return Core::ERROR_NONE;
        }

        uint32_t UserAgent(string& ua) const override
        {
            _adminLock.Lock();
            ua = _config.UserAgent.Value();
            _adminLock.Unlock();

            return Core::ERROR_NONE;
        }

        uint32_t UserAgent(const string& useragent) override
        {
            if (_context == nullptr)
                return Core::ERROR_GENERAL;

            using SetUserAgentData = std::tuple<WebKitImplementation*, string>;
            auto* data = new SetUserAgentData(this, useragent);

            TRACE_L1("New user agent: %s", useragent.c_str());

            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<SetUserAgentData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    const string& useragent = std::get<1>(data);

                    object->_adminLock.Lock();
                    object->_config.UserAgent = useragent;
                    object->_adminLock.Unlock();

                    auto ua = WKStringCreateWithUTF8CString(useragent.c_str());
                    WKPageSetCustomUserAgent(object->_page, ua);
                    WKRelease(ua);
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<SetUserAgentData*>(customdata);
                });

            return Core::ERROR_NONE;
        }

        uint32_t LocalStorageEnabled(bool& enabled) const override
        {
            _adminLock.Lock();
            enabled = _localStorageEnabled;
            _adminLock.Unlock();

            return Core::ERROR_NONE;
        }

        uint32_t LocalStorageEnabled(const bool enabled) override
        {
            if (_context == nullptr)
                return Core::ERROR_GENERAL;

            using SetLocalStorageEnabledData = std::tuple<WebKitImplementation*, bool>;
            auto* data = new SetLocalStorageEnabledData(this, enabled);
            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<SetLocalStorageEnabledData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    bool enabled = std::get<1>(data);

                    object->_adminLock.Lock();
                    object->_localStorageEnabled = enabled;
                    object->_adminLock.Unlock();

                    auto group = WKPageGetPageGroup(object->_page);
                    auto preferences = WKPageGroupGetPreferences(group);
                    WKPreferencesSetLocalStorageEnabled(preferences, enabled);
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<SetLocalStorageEnabledData*>(customdata);
                });

            return Core::ERROR_NONE;
        }

        uint32_t HTTPCookieAcceptPolicy(HTTPCookieAcceptPolicyType& policy) const override
        {
            auto translatePolicy =
                [](WKHTTPCookieAcceptPolicy policy) {
                    switch(policy) {
                        case kWKHTTPCookieAcceptPolicyAlways:
                            return Exchange::IWebBrowser::ALWAYS;
                        case kWKHTTPCookieAcceptPolicyNever:
                            return Exchange::IWebBrowser::NEVER;
                        case kWKHTTPCookieAcceptPolicyOnlyFromMainDocumentDomain:
                            return Exchange::IWebBrowser::ONLY_FROM_MAIN_DOCUMENT_DOMAIN;
                        case kWKHTTPCookieAcceptPolicyExclusivelyFromMainDocumentDomain:
                            return Exchange::IWebBrowser::EXCLUSIVELY_FROM_MAIN_DOCUMENT_DOMAIN;
                    }
                    ASSERT(false);
                    return Exchange::IWebBrowser::ONLY_FROM_MAIN_DOCUMENT_DOMAIN;
                };

            _adminLock.Lock();
            policy = translatePolicy(_httpCookieAcceptPolicy);
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t HTTPCookieAcceptPolicy(const HTTPCookieAcceptPolicyType policy) override
        {
            if (_context == nullptr)
                return Core::ERROR_GENERAL;

            auto translatePolicy =
                [](Exchange::IWebBrowser::HTTPCookieAcceptPolicyType policy) {
                    switch(policy) {
                        case Exchange::IWebBrowser::ALWAYS:
                            return kWKHTTPCookieAcceptPolicyAlways;
                        case Exchange::IWebBrowser::NEVER:
                            return kWKHTTPCookieAcceptPolicyNever;
                        case Exchange::IWebBrowser::ONLY_FROM_MAIN_DOCUMENT_DOMAIN:
                            return kWKHTTPCookieAcceptPolicyOnlyFromMainDocumentDomain;
                        case Exchange::IWebBrowser::EXCLUSIVELY_FROM_MAIN_DOCUMENT_DOMAIN:
                            return kWKHTTPCookieAcceptPolicyExclusivelyFromMainDocumentDomain;
                    }
                    ASSERT(false);
                    return kWKHTTPCookieAcceptPolicyOnlyFromMainDocumentDomain;
                };
            using SetHTTPCookieAcceptPolicyData = std::tuple<WebKitImplementation*, WKHTTPCookieAcceptPolicy>;
            auto* data = new SetHTTPCookieAcceptPolicyData(this, translatePolicy(policy));

            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<SetHTTPCookieAcceptPolicyData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    WKHTTPCookieAcceptPolicy policy =  std::get<1>(data);

                    object->_adminLock.Lock();
                    object->_httpCookieAcceptPolicy = policy;
                    object->_adminLock.Unlock();

                    auto context = WKPageGetContext(object->_page);
                    auto manager = WKContextGetCookieManager(context);
                    WKCookieManagerSetHTTPCookieAcceptPolicy(manager, policy);
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<SetHTTPCookieAcceptPolicyData*>(customdata);
                });

            return Core::ERROR_NONE;
        }

        uint32_t BridgeReply(const string& payload) override
        {
            SendToBridge(Tags::BridgeObjectReply, payload);
            return Core::ERROR_NONE;
        }

        uint32_t BridgeEvent(const string& payload) override
        {
            SendToBridge(Tags::BridgeObjectEvent, payload);
            return Core::ERROR_NONE;
        }

        void SendToBridge(const string& name, const string& payload)
        {
            if (_context == nullptr)
                return;

            using BridgeMessageData = std::tuple<WebKitImplementation*, string, string>;
            auto* data = new BridgeMessageData(this, name, payload);

            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    BridgeMessageData& data = *static_cast<BridgeMessageData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);

                    auto messageName = WKStringCreateWithUTF8CString(std::get<1>(data).c_str());
                    auto messageBody = WKStringCreateWithUTF8CString(std::get<2>(data).c_str());

                    WKPagePostMessageToInjectedBundle(object->_page, messageName, messageBody);

                    WKRelease(messageBody);
                    WKRelease(messageName);

                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<BridgeMessageData*>(customdata);
                });
        }

        uint32_t MixedContentPolicy(MixedContentPolicyType& policy) const override
        {
            _adminLock.Lock();
            policy = _allowMixedContent ? MixedContentPolicyType::ALLOWED : MixedContentPolicyType::BLOCKED;
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t MixedContentPolicy(const MixedContentPolicyType policy) override
        {
            if (_context == nullptr) {
                return Core::ERROR_GENERAL;
            }

            const auto allowMixedContent = (policy == MixedContentPolicyType::ALLOWED);

            using SetMixedContentPolicyData = std::tuple<WebKitImplementation*, bool>;
            auto* data = new SetMixedContentPolicyData(this, allowMixedContent);
            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<SetMixedContentPolicyData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    const bool allowMixedContent = std::get<1>(data);

                    object->_adminLock.Lock();
                    object->_allowMixedContent = allowMixedContent;
                    object->_adminLock.Unlock();

                    auto group = WKPageGetPageGroup(object->_page);
                    auto preferences = WKPageGroupGetPreferences(group);
                    WKPreferencesSetAllowRunningOfInsecureContent(preferences, allowMixedContent);
                    WKPreferencesSetAllowDisplayOfInsecureContent(preferences, allowMixedContent);
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<SetMixedContentPolicyData*>(customdata);
                });
            return Core::ERROR_NONE;
        }
#endif
        uint32_t SecurityProfile(string& profile) const override
        {
            _adminLock.Lock();
            profile = _securityProfileName;
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t SecurityProfile(const string& profile) override
        {
            if (_context == nullptr) {
                return Core::ERROR_GENERAL;
            }

            Core::JSON::ArrayType<Config::SecurityProfileProperty>::Iterator spe(_config.SecurityProfiles.Elements());
            while (spe.Next() == true) {
                if (spe.Current().Name.IsSet() && spe.Current().CipherPrio.IsSet()) {
                    if (profile == spe.Current().Name.Value()) break;
                }
            }

            if (!spe.IsValid()) {
                TRACE(Trace::Error, (_T("Wrong security profile name - %s"), profile.c_str()));
                return Core::ERROR_NOT_SUPPORTED;
            }

            const Config::SecurityProfileProperty securityProfile(spe.Current());
            TRACE_L1(
                "Setting security profile to %s: %s",
                securityProfile.Name.Value().c_str(), securityProfile.CipherPrio.Value().c_str());

            using SetSecurityProfileData = std::tuple<WebKitImplementation*, Config::SecurityProfileProperty>;
            auto* data = new SetSecurityProfileData(this, securityProfile);
            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<SetSecurityProfileData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    const Config::SecurityProfileProperty securityProfile = std::get<1>(data);
                    bool hasProfileChanged = false;

                    object->_adminLock.Lock();
                    if (object->_securityProfileName != securityProfile.Name.Value()) {
                        object->_securityProfileName = securityProfile.Name.Value();
                        hasProfileChanged = true;
                    }
                    object->_adminLock.Unlock();

                    if (hasProfileChanged) {
                        setenv("G_TLS_GNUTLS_PRIORITY", securityProfile.CipherPrio.Value().c_str(), 1);
                        auto context = WKPageGetContext(object->_page);
                        WKContextSetEnv(context, WKStringCreateWithUTF8CString("G_TLS_GNUTLS_PRIORITY"), WKStringCreateWithUTF8CString(securityProfile.CipherPrio.Value().c_str()), false, true);
                    } else
                        SYSLOG(Logging::Notification, (_T("Security profile %s is already set"), object->_securityProfileName.c_str()));
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<SetSecurityProfileData*>(customdata);
                });
            return Core::ERROR_NONE;
        }

        uint32_t CollectGarbage() override
        {
            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                WebKitImplementation* object = static_cast<WebKitImplementation*>(customdata);
#ifdef WEBKIT_GLIB_API
                WebKitWebContext* context = webkit_web_view_get_context(object->_view);
                webkit_web_context_garbage_collect_javascript_objects(context);
#else
                auto context = WKPageGetContext(object->_page);
                WKContextGarbageCollectJavaScriptObjects(context);
#endif
                return G_SOURCE_REMOVE;
            },
            this,
            [](gpointer customdata) {
            });
            return Core::ERROR_NONE;
        }

        uint32_t Visibility(VisibilityType& visible) const override
        {
            _adminLock.Lock();
            visible = (_hidden == true ? VisibilityType::HIDDEN : VisibilityType::VISIBLE);
            _adminLock.Unlock();
            return 0;
        }

        uint32_t Visibility(const VisibilityType visible) override
        {
            Hide(visible == VisibilityType::HIDDEN);
            return 0;
        }

        std::string urlValue() const
        {
            std::unique_lock<std::mutex> lock{urlData_.mutex};
            return urlData_.url;
        }

        std::string urlValue(std::string url)
        {
            std::unique_lock<std::mutex> lock{urlData_.mutex};
            std::swap(urlData_.url, url);
            return url;
        }

        uint32_t URL(const string& URL) override
        {
            using namespace std::chrono;

            TRACE_L1("New URL: %s", URL.c_str());
            ODH_WARNING("New URL: %s", URL.c_str());

            if (!_context) return Core::ERROR_ILLEGAL_STATE;

            using SetURLData = std::tuple<WebKitImplementation *, string>;
            auto *data = new SetURLData(this, URL);

            std::unique_lock<std::mutex> lock{urlData_.mutex};

            urlData_.result = Core::ERROR_TIMEDOUT;
            const auto now = steady_clock::now();

            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean
                {
                    auto &data = *static_cast<SetURLData*>(customdata);
                    WebKitImplementation *object = std::get<0>(data);
                    const string url = std::get<1>(data);

                    object->urlValue(url);

                    object->SetResponseHTTPStatusCode(-1);
#ifdef WEBKIT_GLIB_API
                    webkit_web_view_load_uri(object->_view, url.c_str());
#else
                    auto page = object->GetPage();

                    onURLChangeWorkarounds(url, page);

                    auto shellURL = WKURLCreateWithUTF8CString(url.c_str());
                    WKPageLoadURL(page, shellURL);
                    WKRelease(shellURL);
                    TRACE_L1("URL %s, load requested", url.c_str());
#endif
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata)
                {
                    delete static_cast<SetURLData*>(customdata);
                });

            urlData_.cond.wait_for(
                lock,
                milliseconds{URL_LOAD_RESULT_TIMEOUT_MS},
                [this](){return Core::ERROR_TIMEDOUT != urlData_.result;});

            const auto diff = steady_clock::now() - now;

            TRACE_L1(
                    "URL: %s, load result %s(%d), %dms",
                    urlData_.url.c_str(),
                    Core::ERROR_NONE == urlData_.result ? "OK" : "NOK",
                    int(urlData_.result),
                    int(duration_cast<milliseconds>(diff).count()));

            return urlData_.result;
        }

        uint32_t URL(string& url) const override
        {
            url = urlValue();
            return Core::ERROR_NONE;
        }

        uint32_t FPS(uint8_t& fps) const override
        {
            fps = _fps;
            return 0;
        }

        PluginHost::IStateControl::state State() const override
        {
            return (_state);
        }

        uint32_t Request(PluginHost::IStateControl::command command) override
        {
            uint32_t result = Core::ERROR_ILLEGAL_STATE;

            _adminLock.Lock();

            if (_state == PluginHost::IStateControl::UNINITIALIZED) {
                // Seems we are passing state changes before we reached an operational browser.
                // Just move the state to what we would like it to be :-)
                _state = (command == PluginHost::IStateControl::SUSPEND ? PluginHost::IStateControl::SUSPENDED : PluginHost::IStateControl::RESUMED);
                result = Core::ERROR_NONE;
            } else {
                switch (command) {
                case PluginHost::IStateControl::SUSPEND:
                    if (_state == PluginHost::IStateControl::RESUMED) {
                        Suspend();
                        result = Core::ERROR_NONE;
                    }
                    break;
                case PluginHost::IStateControl::RESUME:
                    if (_state == PluginHost::IStateControl::SUSPENDED) {
                        Resume();
                        result = Core::ERROR_NONE;
                    }
                    break;
                default:
                    break;
                }
            }

            _adminLock.Unlock();

            return (result);
        }

        void Register(PluginHost::IStateControl::INotification* sink)
        {
            _adminLock.Lock();

            // Make sure a sink is not registered multiple times.
            ASSERT(std::find(_stateControlClients.begin(), _stateControlClients.end(), sink) == _stateControlClients.end());

            _stateControlClients.push_back(sink);
            sink->AddRef();

            _adminLock.Unlock();

            TRACE(Trace::Information, (_T("Registered a sink on the browser %p"), sink));
        }

        void Unregister(PluginHost::IStateControl::INotification* sink)
        {
            _adminLock.Lock();

            std::list<PluginHost::IStateControl::INotification*>::iterator index(std::find(_stateControlClients.begin(), _stateControlClients.end(), sink));

            // Make sure you do not unregister something you did not register !!!
            ASSERT(index != _stateControlClients.end());

            if (index != _stateControlClients.end()) {
                (*index)->Release();
                _stateControlClients.erase(index);
                TRACE(Trace::Information, (_T("Unregistered a sink on the browser %p"), sink));
            }

            _adminLock.Unlock();
        }

        void Hide(const bool hidden) override
        {
            if (hidden == true) {
                Hide();
            } else {
                Show();
            }
        }

        void SetURL(const string& url) override
        {
            URL(url);
        }

        string GetURL() const override
        {
            string url;
            URL(url);
            return url;
        }

        uint32_t GetFPS() const override
        {
            uint8_t fps = 0;
            FPS(fps);
            return static_cast<uint32_t>(fps);
        }

        void Register(Exchange::IWebBrowser::INotification* sink) override
        {
            _adminLock.Lock();

            // Make sure a sink is not registered multiple times.
            ASSERT(std::find(_notificationClients.begin(), _notificationClients.end(), sink) == _notificationClients.end());

            _notificationClients.push_back(sink);
            sink->AddRef();

            _adminLock.Unlock();

            TRACE(Trace::Information, (_T("Registered a sink on the browser %p"), sink));
        }

        void Unregister(Exchange::IWebBrowser::INotification* sink) override
        {
            _adminLock.Lock();

            std::list<Exchange::IWebBrowser::INotification*>::iterator index(std::find(_notificationClients.begin(), _notificationClients.end(), sink));

            // Make sure you do not unregister something you did not register !!!
            ASSERT(index != _notificationClients.end());

            if (index != _notificationClients.end()) {
                (*index)->Release();
                _notificationClients.erase(index);
                TRACE(Trace::Information, (_T("Unregistered a sink on the browser %p"), sink));
            }

            _adminLock.Unlock();
        }

        void Register(Exchange::IBrowser::INotification* sink) override
        {
            _adminLock.Lock();

            // Make sure a sink is not registered multiple times.
            ASSERT(std::find(_notificationBrowserClients.begin(), _notificationBrowserClients.end(), sink) == _notificationBrowserClients.end());

            _notificationBrowserClients.push_back(sink);
            sink->AddRef();

            _adminLock.Unlock();

            TRACE(Trace::Information, (_T("Registered a sink on the browser %p"), sink));
        }

        void Unregister(Exchange::IBrowser::INotification* sink) override
        {
            _adminLock.Lock();

            auto index(std::find(_notificationBrowserClients.begin(), _notificationBrowserClients.end(), sink));

            // Make sure you do not unregister something you did not register !!!
            ASSERT(index != _notificationBrowserClients.end());

            if (index != _notificationBrowserClients.end()) {
                (*index)->Release();
                _notificationBrowserClients.erase(index);
                TRACE(Trace::Information, (_T("Unregistered a sink on the browser %p"), sink));
            }

            _adminLock.Unlock();
        }

        // IApplication implementation

        void Register(Exchange::IApplication::INotification* sink) override
        {
            _adminLock.Lock();

            // Make sure a sink is not registered multiple times.
            ASSERT(std::find(_applicationClients.begin(), _applicationClients.end(), sink) == _applicationClients.end());

            _applicationClients.push_back(sink);
            sink->AddRef();

            _adminLock.Unlock();

            TRACE(Trace::Information, (_T("Registered an IApplication sink on the browser %p"), sink));
        }

        void Unregister(Exchange::IApplication::INotification* sink) override
        {
            _adminLock.Lock();

            std::list<Exchange::IApplication::INotification*>::iterator index(std::find(_applicationClients.begin(), _applicationClients.end(), sink));

            // Make sure you do not unregister something you did not register !!!
            ASSERT(index != _applicationClients.end());

            if (index != _applicationClients.end()) {
                (*index)->Release();
                _applicationClients.erase(index);
                TRACE(Trace::Information, (_T("Unregistered an IApplication sink from the browser %p"), sink));
            }

            _adminLock.Unlock();
        }

        uint32_t Reset(const resettype type) override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t Identifier(string& id) const override
        {

            const PluginHost::ISubSystem::IIdentifier* identifier(_service->SubSystems()->Get<PluginHost::ISubSystem::IIdentifier>());
            if (identifier != nullptr) {
                uint8_t buffer[64];

                buffer[0] = static_cast<const PluginHost::ISubSystem::IIdentifier*>(identifier)
                            ->Identifier(sizeof(buffer) - 1, &(buffer[1]));

                if (buffer[0] != 0) {
                    id = Core::SystemInfo::Instance().Id(buffer, ~0);
                }

                identifier->Release();
            }

            return Core::ERROR_NONE;
        }

        uint32_t ContentLink(const string& link) override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t LaunchPoint(launchpointtype& point) const override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t LaunchPoint(const launchpointtype&) override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t Visible(bool& visiblity) const override
        {
            _adminLock.Lock();
            visiblity = (_hidden == false);
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t Visible(const bool& visiblity) override
        {
            Hide(!visiblity);
            return Core::ERROR_NONE;
        }

        uint32_t Language(string& language) const override
        {
            _adminLock.Lock();
            Core::JSON::ArrayType<Core::JSON::String> langsArray = _config.Languages;
            _adminLock.Unlock();

            langsArray.ToString(language);
            return Core::ERROR_NONE;
        }

        uint32_t Language(const string& language) override
        {
            if (_context == nullptr)
                return Core::ERROR_GENERAL;

            Core::OptionalType<Core::JSON::Error> error;
            Core::JSON::ArrayType<Core::JSON::String> array;

            if (!array.FromString(language, error)) {
                TRACE(Trace::Error,
                     (_T("Failed to parse languages array, error='%s', array='%s'\n"),
                      (error.IsSet() ? error.Value().Message().c_str() : "unknown"), language.c_str()));
                return Core::ERROR_GENERAL;
            }

            using SetLanguagesData = std::tuple<WebKitImplementation*, Core::JSON::ArrayType<Core::JSON::String> >;
            auto* data = new SetLanguagesData(this, array);
            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<SetLanguagesData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    Core::JSON::ArrayType<Core::JSON::String> array = std::get<1>(data);

                    object->_adminLock.Lock();
                    object->_config.Languages = array;
                    object->_adminLock.Unlock();

#ifdef WEBKIT_GLIB_API
                    auto* languages = static_cast<char**>(g_new0(char*, array.Length() + 1));
                    Core::JSON::ArrayType<Core::JSON::String>::Iterator index(array.Elements());

                    for (unsigned i = 0; index.Next(); ++i)
                        languages[i] = g_strdup(index.Current().Value().c_str());

                    WebKitWebContext* context = webkit_web_view_get_context(object->_view);
                    webkit_web_context_set_preferred_languages(context, languages);
                    g_strfreev(languages);
#else
                    auto languages = WKMutableArrayCreate();
                    for (auto it = array.Elements(); it.Next();) {
                        if (!it.IsValid())
                            continue;
                        auto itemString = WKStringCreateWithUTF8CString(it.Current().Value().c_str());
                        WKArrayAppendItem(languages, itemString);
                        WKRelease(itemString);
                    }

                    auto context = WKPageGetContext(object->_page);
                    WKSoupSessionSetPreferredLanguages(context, languages);
                    WKRelease(languages);
#endif
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<SetLanguagesData*>(customdata);
                });

            return Core::ERROR_NONE;
        }

        void OnURLChanged(const string& url, bool navigationStart)
        {
            TRACE_L1("%s", url.c_str());

            urlValue(url);

            if(!navigationStart)
            {
                std::unique_lock<std::mutex> lock{urlData_.mutex};
                urlData_.result = Core::ERROR_NONE;
                urlData_.cond.notify_one();
            }

            _adminLock.Lock();

            std::list<Exchange::IWebBrowser::INotification*>::iterator index(_notificationClients.begin());
            {
                while (index != _notificationClients.end()) {
                    (*index)->URLChange(url, false);
                    index++;
                }
            }
            {
                std::list<Exchange::IBrowser::INotification*>::iterator index(_notificationBrowserClients.begin());
                while (index != _notificationBrowserClients.end()) {
                    (*index)->URLChanged(url);
                    index++;
                }
            }

            _adminLock.Unlock();
        }
#ifdef WEBKIT_GLIB_API
        void OnLoadFinished()
        {
            string URL = Core::ToString(webkit_web_view_get_uri(_view));
            OnLoadFinished(URL);
        }
        void OnLoadFinished(const string& url)
        {
#else
        void OnLoadFinished(const string& url, WKNavigationRef navigation)
        {
            const auto currNavRef = NavigationRef();

            TRACE_L1("%s (%p|%p)", url.c_str(), currNavRef, navigation);

            if (currNavRef != navigation) {
                TRACE(Trace::Information, (_T("Ignore 'loadfinished' for previous navigation request")));
                return;
            }
#endif
            urlValue(url);

            {
                std::unique_lock<std::mutex> lock{urlData_.mutex};
                urlData_.result = Core::ERROR_NONE;
                urlData_.cond.notify_one();
            }

            _adminLock.Lock();

            {
                std::list<Exchange::IWebBrowser::INotification*>::iterator index(_notificationClients.begin());

                while (index != _notificationClients.end()) {
                    (*index)->LoadFinished(url, _httpStatusCode);
                    index++;
                }
            }
            {
                std::list<Exchange::IBrowser::INotification*>::iterator index(_notificationBrowserClients.begin());

                while (index != _notificationBrowserClients.end()) {
                    (*index)->LoadFinished(url);
                    index++;
                }
            }

            _adminLock.Unlock();
        }

        void OnLoadFailed()
        {
            const auto url = urlValue();

            TRACE_L1("%s (%p)", url.c_str(), NavigationRef());

            {
                std::unique_lock<std::mutex> lock{urlData_.mutex};
                urlData_.result = Core::ERROR_INCORRECT_URL;
                urlData_.cond.notify_one();
            }

            _adminLock.Lock();

            std::list<Exchange::IWebBrowser::INotification*>::iterator index(_notificationClients.begin());

            while (index != _notificationClients.end()) {
                (*index)->LoadFailed(url);
                index++;
            }

            _adminLock.Unlock();
            ODH_ERROR("Failed to load URL: %s", url.c_str());
        }

        void OnStateChange(const PluginHost::IStateControl::state newState)
        {
            _adminLock.Lock();

            if (_state != newState) {
                _state = newState;

                std::list<PluginHost::IStateControl::INotification*>::iterator index(_stateControlClients.begin());

                while (index != _stateControlClients.end()) {
                    (*index)->StateChange(newState);
                    index++;
                }
            }

            _adminLock.Unlock();
        }
        void Hidden(const bool hidden)
        {
            _adminLock.Lock();

            if (hidden != _hidden) {
                _hidden = hidden;

                {
                    std::list<Exchange::IWebBrowser::INotification*>::iterator index(_notificationClients.begin());

                    while (index != _notificationClients.end()) {
                        (*index)->VisibilityChange(hidden);
                        index++;
                    }
                }
                {
                    std::list<Exchange::IBrowser::INotification*>::iterator index(_notificationBrowserClients.begin());
                    while (index != _notificationBrowserClients.end()) {
                        (*index)->Hidden(hidden);
                        index++;
                    }
                }
                {
                    std::list<Exchange::IApplication::INotification*>::iterator index(_applicationClients.begin());
                    while (index != _applicationClients.end()) {
                        (*index)->VisibilityChange(hidden);
                        index++;
                    }
                }
            }

            _adminLock.Unlock();
        }
        void OnJavaScript(const std::vector<string>& text) const
        {
            for (const string& line : text) {
                std::cout << "  " << line << std::endl;
            }
        }
        void OnBridgeQuery(const string& text)
        {
            _adminLock.Lock();

            std::list<Exchange::IWebBrowser::INotification*>::iterator index(_notificationClients.begin());

            while (index != _notificationClients.end()) {
                (*index)->BridgeQuery(text);
                index++;
            }

            _adminLock.Unlock();
        }

        void SetResponseHTTPStatusCode(int32_t code)
        {
            _httpStatusCode = code;
        }

#if !defined(WEBKITBROWSER_CLIENT_CERTS_PRIV_KEY_PASSWD)
#define WEBKITBROWSER_CLIENT_CERTS_PRIV_KEY_PASSWD ("")
#endif

        bool isASCIISpace(char c)
        {
            return c == ' ' || c == '\t';
        }

        void trimLeadingAndTrailingWs(std::string* s)
        {
            while (!s->empty() && isASCIISpace(s->front())) {
                s->erase(0, 1);
            }

            while(!s->empty() && isASCIISpace(s->back())) {
                s->pop_back();
            }
        }

        std::vector<std::string> tokenize(const std::string& s, const char* delimiters)
        {
            auto lastPos = 0u;
            std::vector<std::string> result;
            auto pos = 0u;
            do {
                pos = s.find_first_of(delimiters, lastPos);
                if (pos != std::string::npos) {
                    auto toAdd = s.substr(lastPos, pos - lastPos);
                    trimLeadingAndTrailingWs(&toAdd);
                    result.push_back(std::move(toAdd));
                    // Skip more delimiters.
                    lastPos = s.find_first_not_of(delimiters, pos + 1);
                } else {
                    auto toAdd = s.substr(lastPos);
                    trimLeadingAndTrailingWs(&toAdd);
                    // No more delimiters in the front so push the last bit.
                    result.push_back(std::move(toAdd));
                }
            } while (pos != std::string::npos && lastPos != std::string::npos);

            return result;
        }

        static int passCb(char* buf, int size, int rwflag, void* u) {
            if (rwflag)
                return -1;

            int passLen =  strlen(WEBKITBROWSER_CLIENT_CERTS_PRIV_KEY_PASSWD);
            if (size < passLen + 1) {
                fprintf(stderr, "Error decrypting private key. Password won't fit to the provided buffer.\n");
                return -1;
            }
            strncpy(buf, WEBKITBROWSER_CLIENT_CERTS_PRIV_KEY_PASSWD, passLen + 1);
            return passLen;
        }

        bool DecryptWithOpenSSL(std::string* key) {
            using AutoMemBio = std::unique_ptr<BIO, std::function<int(BIO*)>>;
            using AutoEVPKey = std::unique_ptr<EVP_PKEY, std::function<void(EVP_PKEY*)>>;

            std::vector<char> readBuf(key->begin(), key->end());
            AutoMemBio memBio(
                    BIO_new_mem_buf(static_cast<void*>(readBuf.data()), readBuf.size()),
                    BIO_free);
            if (!memBio)
                return false;

            AutoEVPKey decryptedKey(
                    PEM_read_bio_PrivateKey(memBio.get(), nullptr, passCb, nullptr),
                    EVP_PKEY_free);
            if (!decryptedKey.get())
                return false;

            memBio.reset(BIO_new(BIO_s_mem()));
            if (!PEM_write_bio_PrivateKey(memBio.get(), decryptedKey.get(),
                        nullptr, nullptr, 0, nullptr,
                        nullptr)) {
                return false;
            }

            char* data = nullptr;
            auto dataSize = BIO_get_mem_data(memBio.get(), &data);
            key->clear();
            key->append(data, dataSize);
            return true;
        }

        void SetupClientCertificates() {
            OpenSSL_add_all_algorithms();

            /* TODO: This configuration is temporary. It'll be moved to plugin config file when architecture will be defined. */
            std::string wpeClientCertsConf {
                "https://ipsecure.int.bbc.co.uk/=/run/certificates/bbc-iplayer-cert.pem,/run/privatekeys/bbc-iplayer-key.pem\r\n"
                "https://ipsecure.test.bbc.co.uk/=/run/certificates/bbc-iplayer-cert.pem,/run/privatekeys/bbc-iplayer-key.pem\r\n"
                "https://ipsecure.stage.bbc.co.uk/=/run/certificates/bbc-iplayer-cert.pem,/run/privatekeys/bbc-iplayer-key.pem\r\n"
                "https://securegate.iplayer.bbc.co.uk/=/run/certificates/bbc-iplayer-cert.pem,/run/privatekeys/bbc-iplayer-key.pem\r\n"
                "https://pac.networking.certification.bbctvapps.co.uk/=/run/certificates/bbc-iplayer-cert.pem,/run/privatekeys/bbc-iplayer-key.pem\r\n"};

            std::string urls;
            constexpr auto kUrlsSep = "|";  // not allowed in URI
            auto lines = tokenize(wpeClientCertsConf, "\r\n");
            for (const auto& line : lines) {
                auto urlToFiles = tokenize(line, "=");
                if (urlToFiles.size() == 2) {
                    auto files = tokenize(urlToFiles[1], ",");
                    if (files.size() == 2) {
                        if (!urls.empty())
                            urls += kUrlsSep;
                        urls += urlToFiles[0];

                        std::string certContents = GetFileContent(files[0]);

                        if (certContents.empty()) {
                            TRACE(Trace::Error, (_T("Empty certificate for %s %s"), files[0].c_str(), urlToFiles[0].c_str()));
                            continue;
                        }
                        std::string keyContents = GetFileContent(files[1]);
                        if (keyContents.empty()) {
                            TRACE(Trace::Error, (_T("Empty private key for %s %s"), files[1].c_str(), urlToFiles[0].c_str()));
                            continue;
                        }
                        if (!DecryptWithOpenSSL(&keyContents)) {
                            TRACE(Trace::Error, (_T("Failed decrypting private key for %s %s"), files[1].c_str(), urlToFiles[0].c_str()));
                            continue;
                        }
                        certContents += "\n" + keyContents;
                        Core::SystemInfo::SetEnvironment(_T(urlToFiles[0].c_str()), certContents.c_str());
                    }
                }
            }

            if (!urls.empty()) {
                Core::SystemInfo::SetEnvironment(_T("WPE_CLIENT_CERTIFICATES_URLS"), urls.c_str());
            }
        }

        uint32_t Configure(PluginHost::IShell* service) override
        {
            consoleLogPrefix = service->Callsign();
            _service = service;

            _dataPath = service->DataPath();

            string configLine = service->ConfigLine();
            Core::OptionalType<Core::JSON::Error> error;
            if (_config.FromString(configLine, error) == false) {
                SYSLOG(Logging::ParsingError,
                       (_T("Failed to parse config line, error: '%s', config line: '%s'."),
                        (error.IsSet() ? error.Value().Message().c_str() : "Unknown"),
                        configLine.c_str()));
                return (Core::ERROR_INCOMPLETE_CONFIG);
            }

            const bool environmentOverride(WebKitBrowser::EnvironmentOverride(_config.EnvironmentOverride.Value()));

            {
                std::string url = _config.URL.Value();

                if(environmentOverride) Core::SystemInfo::GetEnvironment(_T("WPE_WEBKIT_URL"), url);
                urlValue(url);
            }

            Core::SystemInfo::SetEnvironment(_T("QUEUEPLAYER_FLUSH_MODE"), _T("3"), false);
            // Change HOME environment value from config.json PersistentPath to plugins/WebKitBrowser.json DiskCacheDir
            Core::SystemInfo::SetEnvironment(_T("HOME"),
                _config.DiskCacheDir.Value().empty() ? _service->PersistentPath() : _config.DiskCacheDir.Value());

            if (_config.ClientIdentifier.IsSet() == true) {
                string value(service->Callsign() + ',' + _config.ClientIdentifier.Value());
                Core::SystemInfo::SetEnvironment(_T("CLIENT_IDENTIFIER"), value, !environmentOverride);
            } else {
                Core::SystemInfo::SetEnvironment(_T("CLIENT_IDENTIFIER"), service->Callsign(), !environmentOverride);
            }

            // Aways setup initial client certificates to propagate env variable to NetworkProcess
            SetupClientCertificates();
            // NOTE: first valid security profile defined in config is used by default
            Core::JSON::ArrayType<Config::SecurityProfileProperty>::Iterator spe(_config.SecurityProfiles.Elements());
            while (spe.Next() == true) {
                if (spe.Current().Name.IsSet() && spe.Current().CipherPrio.IsSet()) {
                    _securityProfileName = spe.Current().Name.Value();
                    setenv("G_TLS_GNUTLS_PRIORITY", spe.Current().CipherPrio.Value().c_str(), 1);
                    TRACE(Trace::Information, (_T("Setting security profile to %s: %s"),
                                spe.Current().Name.Value().c_str(), spe.Current().CipherPrio.Value().c_str()));
                    break;
                }
            }

            // WEBKIT_DEBUG
            if (_config.WebkitDebug.Value().empty() == false)
               Core::SystemInfo::SetEnvironment(_T("WEBKIT_DEBUG"), _config.WebkitDebug.Value(), !environmentOverride);

            // GST_DEBUG
            if (_config.GstDebug.Value().empty() == false)
               Core::SystemInfo::SetEnvironment(_T("GST_DEBUG"), _config.GstDebug.Value(), !environmentOverride);

            // GST_DEBUG_NO_COLOR
            if (_config.GstNoColor.IsSet())
                Core::SystemInfo::SetEnvironment(_T("GST_DEBUG_NO_COLOR"), _T("1"), !environmentOverride);

            // Allow running mixed content for whitelisted URLs
            if (_config.MixedContentWhitelist.Value().empty() == false)
               Core::SystemInfo::SetEnvironment(_T("WPE_MIXEDCONTENT_WHITELIST"), _config.MixedContentWhitelist.Value(), !environmentOverride);

            // Set dummy window for gst-gl
            Core::SystemInfo::SetEnvironment(_T("GST_GL_WINDOW"), _T("dummy"), !environmentOverride);

            // MSE Buffers
            if (_config.MSEBuffers.Value().empty() == false)
                Core::SystemInfo::SetEnvironment(_T("MSE_MAX_BUFFER_SIZE"), _config.MSEBuffers.Value(), !environmentOverride);

            // Memory Pressure
            if (_config.MemoryPressure.Value().empty() == false)
                Core::SystemInfo::SetEnvironment(_T("WPE_POLL_MAX_MEMORY"), _config.MemoryPressure.Value(), !environmentOverride);

            // Memory Profile
            if (_config.MemoryProfile.Value().empty() == false)
                Core::SystemInfo::SetEnvironment(_T("WPE_RAM_SIZE"), _config.MemoryProfile.Value(), !environmentOverride);

            // GStreamer on-disk buffering
            if (_config.MediaDiskCache.Value() == false)
                Core::SystemInfo::SetEnvironment(_T("WPE_SHELL_DISABLE_MEDIA_DISK_CACHE"), _T("1"), !environmentOverride);
            else {
                // Change WPE_SHELL_MEDIA_DISK_CACHE_PATH environment value from config.json PersistentPath to plugins/WebKitBrowser.json DiskCacheDir
                Core::SystemInfo::SetEnvironment(_T("WPE_SHELL_MEDIA_DISK_CACHE_PATH"),
                    _config.DiskCacheDir.Value().empty() ? _service->PersistentPath() : _config.DiskCacheDir.Value(),  !environmentOverride);
            }

            // Disk Cache
            if (_config.DiskCache.Value().empty() == false)
                Core::SystemInfo::SetEnvironment(_T("WPE_DISK_CACHE_SIZE"), _config.DiskCache.Value(), !environmentOverride);

            // Disk Cache Dir
            if (_config.DiskCacheDir.Value().empty() == false)
               Core::SystemInfo::SetEnvironment(_T("XDG_CACHE_HOME"), _config.DiskCacheDir.Value(), !environmentOverride);

            if (_config.XHRCache.Value() == false)
                Core::SystemInfo::SetEnvironment(_T("WPE_DISABLE_XHR_RESPONSE_CACHING"), _T("1"), !environmentOverride);

            // Enable cookie persistent storage
            if (_config.CookieStorage.Value().empty() == false)
                Core::SystemInfo::SetEnvironment(_T("WPE_SHELL_COOKIE_STORAGE"), _T("1"), !environmentOverride);

            // Use cairo noaa compositor
            if (_config.Compositor.Value().empty() == false)
                Core::SystemInfo::SetEnvironment(_T("CAIRO_GL_COMPOSITOR"), _config.Compositor.Value(), !environmentOverride);

            // WebInspector
            if (_config.Inspector.Value().empty() == false) {
                if (_config.Automation.Value())
                    Core::SystemInfo::SetEnvironment(_T("WEBKIT_INSPECTOR_SERVER"), _config.Inspector.Value(), !environmentOverride);
                else
                    Core::SystemInfo::SetEnvironment(_T("WEBKIT_LEGACY_INSPECTOR_SERVER"), _config.Inspector.Value(), !environmentOverride);
            }

            // RPI mouse support
            if (_config.Cursor.Value() == true)
                Core::SystemInfo::SetEnvironment(_T("WPE_BCMRPI_CURSOR"), _T("1"), !environmentOverride);

            // RPI touch support
            if (_config.Touch.Value() == true)
                Core::SystemInfo::SetEnvironment(_T("WPE_BCMRPI_TOUCH"), _T("1"), !environmentOverride);

            // Rank Thunder Decryptor higher than ClearKey one
            if (_config.ThunderDecryptorPreference.Value() == true)
                Core::SystemInfo::SetEnvironment(_T("WEBKIT_GST_EME_RANK_PRIORITY"), _T("Thunder"), !environmentOverride);

            // WPE allows the LLINT to be used if true
            if (_config.JavaScript.UseLLInt.Value() == false) {
                Core::SystemInfo::SetEnvironment(_T("JSC_useLLInt"), _T("false"), !environmentOverride);
            }

            // WPE allows the baseline JIT to be used if true
            if (_config.JavaScript.UseJIT.Value() == false) {
                Core::SystemInfo::SetEnvironment(_T("JSC_useJIT"), _T("false"), !environmentOverride);
            }

            // WPE allows the DFG JIT to be used if true
            if (_config.JavaScript.UseDFG.Value() == false) {
                Core::SystemInfo::SetEnvironment(_T("JSC_useDFGJIT"), _T("false"), !environmentOverride);
            }

            // WPE allows the FTL JIT to be used if true
            if (_config.JavaScript.UseFTL.Value() == false) {
                Core::SystemInfo::SetEnvironment(_T("JSC_useFTLJIT"), _T("false"), !environmentOverride);
            }

            // WPE allows the DOM JIT to be used if true
            if (_config.JavaScript.UseDOM.Value() == false) {
                Core::SystemInfo::SetEnvironment(_T("JSC_useDOMJIT"), _T("false"), !environmentOverride);
            }

            // WPE DumpOptions
            if (_config.JavaScript.DumpOptions.Value().empty() == false) {
                Core::SystemInfo::SetEnvironment(_T("JSC_dumpOptions"), _config.JavaScript.DumpOptions.Value(), !environmentOverride);
            }

            // ThreadedPainting
            if (_config.ThreadedPainting.Value().empty() == false) {
                Core::SystemInfo::SetEnvironment(_T("WEBKIT_NICOSIA_PAINTING_THREADS"), _config.ThreadedPainting.Value(), !environmentOverride);
            }

            // PTSOffset
            if (_config.PTSOffset.IsSet() == true) {
                string ptsoffset(Core::NumberType<int16_t>(_config.PTSOffset.Value()).Text());
                Core::SystemInfo::SetEnvironment(_T("PTS_REPORTING_OFFSET_MS"), ptsoffset, !environmentOverride);
            }

            if (_config.LocalStorageEnabled.IsSet() == true) {
                _localStorageEnabled = _config.LocalStorageEnabled.Value();
            }

            if (_config.ClientCert.IsSet() == true && _config.ClientCertKey.IsSet() == true) {
                Core::SystemInfo::SetEnvironment(_T("G_TLS_OPENSSL_CLIENT_CERT_PATH"), _config.ClientCert.Value(), !environmentOverride);
                Core::SystemInfo::SetEnvironment(_T("G_TLS_OPENSSL_CLIENT_CERT_KEY_PATH"), _config.ClientCertKey.Value(), !environmentOverride);
            }

            // ExecPath
            if (_config.ExecPath.IsSet() == true) {
                Core::SystemInfo::SetEnvironment(_T("WEBKIT_EXEC_PATH"), _config.ExecPath.Value(), !environmentOverride);
            }

            //  HTTPProxy
            if (_config.HTTPProxy.IsSet() == true) {
                Core::SystemInfo::SetEnvironment(_T("http_proxy"), _config.HTTPProxy.Value(), !environmentOverride);
            }

            // HTTPProxyExclusion
            if (_config.HTTPProxyExclusion.IsSet() == true) {
                Core::SystemInfo::SetEnvironment(_T("no_proxy"), _config.HTTPProxyExclusion.Value(), !environmentOverride);
            }

            // HTTPProxyExclusion
            if (_config.TCPKeepAlive.Value() == true) {
                Core::SystemInfo::SetEnvironment(_T("WEBKIT_TCP_KEEPALIVE"), _T("1"), !environmentOverride);
            }

            string width(Core::NumberType<uint16_t>(_config.Width.Value()).Text());
            string height(Core::NumberType<uint16_t>(_config.Height.Value()).Text());
            string maxFPS(Core::NumberType<uint16_t>(_config.MaxFPS.Value()).Text());
            Core::SystemInfo::SetEnvironment(_T("WEBKIT_RESOLUTION_WIDTH"), width, !environmentOverride);
            Core::SystemInfo::SetEnvironment(_T("WEBKIT_RESOLUTION_HEIGHT"), height, !environmentOverride);
            Core::SystemInfo::SetEnvironment(_T("WEBKIT_MAXIMUM_FPS"), maxFPS, !environmentOverride);

            if (width.empty() == false) {
                Core::SystemInfo::SetEnvironment(_T("GST_VIRTUAL_DISP_WIDTH"), width, !environmentOverride);
            }

            if (height.empty() == false) {
                Core::SystemInfo::SetEnvironment(_T("GST_VIRTUAL_DISP_HEIGHT"), height, !environmentOverride);
            }

            // Oke, so we are good to go.. Release....
            Core::Thread::Run();

            _configurationCompleted.WaitState(true, Core::infinite);

            return (Core::ERROR_NONE);
        }

        void NotifyClosure()
        {
            _adminLock.Lock();

            {

                std::list<Exchange::IWebBrowser::INotification*>::iterator index(_notificationClients.begin());

                while (index != _notificationClients.end()) {
                    (*index)->PageClosure();
                    index++;
                }
            }
            {
                std::list<Exchange::IBrowser::INotification*>::iterator index(_notificationBrowserClients.begin());

                while (index != _notificationBrowserClients.end()) {
                    (*index)->Closure();
                    index++;
                }
            }

            _adminLock.Unlock();
        }

        void SetFPS()
        {
            ++_frameCount;
            gint64 time = g_get_monotonic_time();
            if (time - _lastDumpTime >= G_USEC_PER_SEC) {
                _fps = _frameCount * G_USEC_PER_SEC * 1.0 / (time - _lastDumpTime);
                _frameCount = 0;
                _lastDumpTime = time;
            }
        }

        string GetConfig(const string& key) const
        {
            string value;
            _config.Bundle.Config(key,value);
            return (value);
        }
#ifndef WEBKIT_GLIB_API
        void NavigationRef(WKNavigationRef ref)
        {
            _adminLock.Lock();
            _navigationRef = ref;
            _adminLock.Unlock();
        }

        WKNavigationRef NavigationRef() const
        {
            _adminLock.Lock();
            WKNavigationRef ref = _navigationRef;
            _adminLock.Unlock();
            return ref;
        }

        void OnNotificationShown(uint64_t notificationID) const
        {
            WKNotificationManagerProviderDidShowNotification(_notificationManager, notificationID);
        }

        void OnRequestAutomationSession(WKContextRef context, WKStringRef sessionID)
        {
            _automationSession = WKWebAutomationSessionCreate(sessionID);
            _handlerAutomationSession.base.clientInfo = static_cast<void*>(this);
            WKWebAutomationSessionSetClient(_automationSession, &_handlerAutomationSession.base);
            WKContextSetAutomationSession(context, _automationSession);
        }

        WKPageRef GetPage() const
        {
            _adminLock.Lock();
            WKPageRef page = _page;
            _adminLock.Unlock();
            return page;
        }
#endif
        BEGIN_INTERFACE_MAP(WebKitImplementation)
        INTERFACE_ENTRY(Exchange::IWebBrowser)
        INTERFACE_ENTRY(Exchange::IBrowser)
        INTERFACE_ENTRY(Exchange::IBrowserResources)
        INTERFACE_ENTRY(Exchange::IBrowserSecurity)
        INTERFACE_ENTRY (Exchange::IApplication)
        INTERFACE_ENTRY(PluginHost::IStateControl)
        END_INTERFACE_MAP

    private:
        void Hide()
        {
            if (_context != nullptr) {
                _time = Core::Time::Now().Ticks();
                g_main_context_invoke(
                    _context,
                    [](gpointer customdata) -> gboolean {
                        WebKitImplementation* object = static_cast<WebKitImplementation*>(customdata);
#ifdef WEBKIT_GLIB_API
                        webkit_web_view_hide(object->_view);
#else
                        WKViewSetViewState(object->_view, (object->_state == PluginHost::IStateControl::RESUMED ? kWKViewStateIsInWindow : 0));
#endif
                        object->Hidden(true);
                        TRACE_GLOBAL(Trace::Information, (_T("Internal Hide Notification took %d mS."), static_cast<uint32_t>(Core::Time::Now().Ticks() - object->_time)));

                        return FALSE;
                    },
                    this);
            }
        }
        void Show()
        {
            if (_context != nullptr) {
                _time = Core::Time::Now().Ticks();
                g_main_context_invoke(
                    _context,
                    [](gpointer customdata) -> gboolean {
                        WebKitImplementation* object = static_cast<WebKitImplementation*>(customdata);
#ifdef WEBKIT_GLIB_API
                        webkit_web_view_show(object->_view);
#else
                        WKViewSetViewState(object->_view, (object->_state == PluginHost::IStateControl::RESUMED ? kWKViewStateIsInWindow : 0) | kWKViewStateIsVisible);
#endif
                        object->Hidden(false);

                        TRACE_GLOBAL(Trace::Information, (_T("Internal Show Notification took %d mS."), static_cast<uint32_t>(Core::Time::Now().Ticks() - object->_time)));

                        return FALSE;
                    },
                    this);
            }
        }
        void Suspend()
        {
            if (_context == nullptr) {
                _state = PluginHost::IStateControl::SUSPENDED;
            } else {
                _time = Core::Time::Now().Ticks();
                g_main_context_invoke(
                    _context,
                    [](gpointer customdata) -> gboolean {
                        WebKitImplementation* object = static_cast<WebKitImplementation*>(customdata);
#ifdef WEBKIT_GLIB_API
                        webkit_web_view_suspend(object->_view);
#else
                        if (object->_config.LoadBlankPageOnSuspendEnabled.Value()) {
                            const char kBlankURL[] = "about:blank";
                            if (GetPageActiveURL(object->_page) != kBlankURL)
                                object->SetURL(kBlankURL);
                            ASSERT(object->urlValue() == kBlankURL);
                        }

                        WKViewSetViewState(object->_view, (object->_hidden ? 0 : kWKViewStateIsVisible));
#endif
                        object->OnStateChange(PluginHost::IStateControl::SUSPENDED);

                        TRACE_GLOBAL(Trace::Information, (_T("Internal Suspend Notification took %d mS."), static_cast<uint32_t>(Core::Time::Now().Ticks() - object->_time)));

                        object->CheckWebProcess();

                        return FALSE;
                    },
                    this);
            }
        }
        void Resume()
        {
            if (_context == nullptr) {
                _state = PluginHost::IStateControl::RESUMED;
            } else {
                _time = Core::Time::Now().Ticks();

                g_main_context_invoke(
                    _context,
                    [](gpointer customdata) -> gboolean {
                        WebKitImplementation* object = static_cast<WebKitImplementation*>(customdata);
#ifdef WEBKIT_GLIB_API
                        webkit_web_view_resume(object->_view);
#else
                        WKViewSetViewState(object->_view, (object->_hidden ? 0 : kWKViewStateIsVisible) | kWKViewStateIsInWindow);
#endif
                        object->OnStateChange(PluginHost::IStateControl::RESUMED);

                        TRACE_GLOBAL(Trace::Information, (_T("Internal Resume Notification took %d mS."), static_cast<uint32_t>(Core::Time::Now().Ticks() - object->_time)));

                        return FALSE;
                    },
                    this);
            }
        }
        std::string GetFileContent(const std::string& fileName)
        {
            std::string content;
            auto stream = std::ifstream{fileName};

            if (stream.fail()) {
                TRACE(Trace::Error, (_T("Failed to get content from file: %s"), fileName.c_str()));
            } else {
                content = std::string{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
            }
            return content;
        }
#ifdef WEBKIT_GLIB_API
        static void initializeWebExtensionsCallback(WebKitWebContext* context, WebKitImplementation* browser)
        {
            webkit_web_context_set_web_extensions_directory(context, browser->_dataPath.c_str());
            // FIX it
            GVariant* data = g_variant_new("(smsb)", std::to_string(browser->_guid).c_str(), !browser->_config.Whitelist.Value().empty() ? browser->_config.Whitelist.Value().c_str() : nullptr, browser->_config.LogToSystemConsoleEnabled.Value());
            webkit_web_context_set_web_extensions_initialization_user_data(context, data);
        }
        static void wpeNotifyWPEFrameworkMessageReceivedCallback(WebKitUserContentManager*, WebKitJavascriptResult* message, WebKitImplementation* browser)
        {
            JSCValue* args = webkit_javascript_result_get_js_value(message);
            JSCValue* arrayLengthValue = jsc_value_object_get_property(args, "length");
            int arrayLength = jsc_value_to_int32(arrayLengthValue);
            g_object_unref(arrayLengthValue);

            std::vector<string> messageStrings;
            for (int i = 0; i < arrayLength; ++i) {
                JSCValue* itemValue = jsc_value_object_get_property_at_index(args, i);
                char* itemStr = jsc_value_to_string(itemValue);
                g_object_unref(itemValue);
                messageStrings.push_back(Core::ToString(itemStr));
                g_free(itemStr);
            }

            browser->OnJavaScript(messageStrings);
        }
        static gboolean decidePolicyCallback(WebKitWebView*, WebKitPolicyDecision* decision, WebKitPolicyDecisionType)
        {
            webkit_policy_decision_use(decision);
            return TRUE;
        }
        static void uriChangedCallback(WebKitWebView* webView, GParamSpec*, WebKitImplementation* browser)
        {
            browser->OnURLChanged(Core::ToString(webkit_web_view_get_uri(webView)), false);
        }
        static void loadChangedCallback(WebKitWebView* webView, WebKitLoadEvent loadEvent, WebKitImplementation* browser)
        {
            if (loadEvent == WEBKIT_LOAD_FINISHED)
                browser->OnLoadFinished();
        }
        static void webProcessTerminatedCallback(WebKitWebView* webView, WebKitWebProcessTerminationReason reason)
        {
            switch (reason) {
            case WEBKIT_WEB_PROCESS_CRASHED:
                SYSLOG(Trace::Fatal, (_T("CRASH: WebProcess crashed: exiting ...")));
                break;
            case WEBKIT_WEB_PROCESS_EXCEEDED_MEMORY_LIMIT:
                SYSLOG(Trace::Fatal, (_T("CRASH: WebProcess terminated due to memory limit: exiting ...")));
                break;
            }
            exit(1);
        }
        static void closeCallback(WebKitWebView*, WebKitImplementation* browser)
        {
            browser->NotifyClosure();
        }
        static gboolean decidePermissionCallback(WebKitWebView*, WebKitPermissionRequest* permissionRequest)
        {
            webkit_permission_request_allow(permissionRequest);
            return TRUE;
        }
        static gboolean showNotificationCallback(WebKitWebView*, WebKitNotification* notification, WebKitImplementation* browser)
        {
            TRACE_GLOBAL(HTML5Notification, (_T("%s - %s"), webkit_notification_get_title(notification), webkit_notification_get_body(notification)));
            return FALSE;
        }
        static WebKitWebView* createWebViewForAutomationCallback(WebKitAutomationSession* session, WebKitImplementation* browser)
        {
            return browser->_view;
        }
        static void automationStartedCallback(WebKitWebContext* context, WebKitAutomationSession* session, WebKitImplementation* browser)
        {
            WebKitApplicationInfo *info = webkit_application_info_new();
            webkit_application_info_set_name(info, "WPEWebKitBrowser");
            webkit_application_info_set_version(info, 1, 0, 0);
            webkit_automation_session_set_application_info(session, info);
            webkit_application_info_unref(info);

            g_signal_connect(session, "create-web-view", reinterpret_cast<GCallback>(createWebViewForAutomationCallback), browser);
        }
        uint32_t Worker() override
        {
            _context = g_main_context_new();
            _loop = g_main_loop_new(_context, FALSE);
            g_main_context_push_thread_default(_context);

            bool automationEnabled = _config.Automation.Value();

            WebKitWebContext* context;
            if (automationEnabled) {
                context = webkit_web_context_new_ephemeral();
                webkit_web_context_set_automation_allowed(context, TRUE);
                g_signal_connect(context, "automation-started", reinterpret_cast<GCallback>(automationStartedCallback), this);
            } else {
                gchar* wpeStoragePath;
                if (_config.LocalStorage.IsSet() == true && _config.LocalStorage.Value().empty() == false)
                    wpeStoragePath = g_build_filename(_config.LocalStorage.Value().c_str(), "wpe", "local-storage", nullptr);
                else
                    wpeStoragePath = g_build_filename(g_get_user_cache_dir(), "wpe", "local-storage", nullptr);
                g_mkdir_with_parents(wpeStoragePath, 0700);

                gchar* wpeDiskCachePath;
                if (_config.DiskCacheDir.IsSet() == true && _config.DiskCacheDir.Value().empty() == false)
                    wpeDiskCachePath = g_build_filename(_config.DiskCacheDir.Value().c_str(), "wpe", "disk-cache", nullptr);
                else
                    wpeDiskCachePath = g_build_filename(g_get_user_cache_dir(), "wpe", "disk-cache", nullptr);
                g_mkdir_with_parents(wpeDiskCachePath, 0700);

                auto* websiteDataManager = webkit_website_data_manager_new("local-storage-directory", wpeStoragePath, "disk-cache-directory", wpeDiskCachePath, nullptr);
                g_free(wpeStoragePath);
                g_free(wpeDiskCachePath);

                context = webkit_web_context_new_with_website_data_manager(websiteDataManager);
                g_object_unref(websiteDataManager);
            }

            if (_config.InjectedBundle.Value().empty() == false) {
                // Set up injected bundle. Will be loaded once WPEWebProcess is started.
                g_signal_connect(context, "initialize-web-extensions", G_CALLBACK(initializeWebExtensionsCallback), this);
            }

            if (!webkit_web_context_is_ephemeral(context)) {
                gchar* cookieDatabasePath;
                if (_config.CookieStorage.IsSet() == true && _config.CookieStorage.Value().empty() == false)
                    cookieDatabasePath = g_build_filename(_config.CookieStorage.Value().c_str(), "cookies.db", nullptr);
                else
                    cookieDatabasePath = g_build_filename(g_get_user_cache_dir(), "cookies.db", nullptr);

                auto* cookieManager = webkit_web_context_get_cookie_manager(context);
                webkit_cookie_manager_set_persistent_storage(cookieManager, cookieDatabasePath, WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);
            }

            if (!_config.CertificateCheck)
                webkit_web_context_set_tls_errors_policy(context, WEBKIT_TLS_ERRORS_POLICY_IGNORE);

            auto* languages = static_cast<char**>(g_new0(char*, _config.Languages.Length() + 1));
            Core::JSON::ArrayType<Core::JSON::String>::Iterator index(_config.Languages.Elements());

            for (unsigned i = 0; index.Next(); ++i)
                languages[i] = g_strdup(index.Current().Value().c_str());

            webkit_web_context_set_preferred_languages(context, languages);
            g_strfreev(languages);

            auto* preferences = webkit_settings_new();

            webkit_settings_set_enable_encrypted_media(preferences, TRUE);
            webkit_settings_set_enable_mediasource(preferences, TRUE);

            // Turn on/off WebGL
            webkit_settings_set_enable_webgl(preferences, _config.WebGLEnabled.Value());

            webkit_settings_set_enable_non_composited_webgl(preferences, _config.NonCompositedWebGLEnabled.Value());

            // Media Content Types Requiring Hardware Support
            if (_config.MediaContentTypesRequiringHardwareSupport.IsSet() == true
                && _config.MediaContentTypesRequiringHardwareSupport.Value().empty() == false) {
                webkit_settings_set_media_content_types_requiring_hardware_support(preferences,
                    _config.MediaContentTypesRequiringHardwareSupport.Value().c_str());
            }

            if (_config.UserAgent.IsSet() == true && _config.UserAgent.Value().empty() == false)
                webkit_settings_set_user_agent(preferences, _config.UserAgent.Value().c_str());

            _view = WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
                "backend", webkit_web_view_backend_new(wpe_view_backend_create(), nullptr, nullptr),
                "web-context", context,
                "settings", preferences,
                "is-controlled-by-automation", automationEnabled,
                nullptr));
            g_object_unref(context);
            g_object_unref(preferences);

            unsigned frameDisplayedCallbackID = 0;
            if (_config.FPS.Value() == true) {
                frameDisplayedCallbackID = webkit_web_view_add_frame_displayed_callback(_view, [](WebKitWebView*, gpointer userData) {
                    auto* browser = static_cast<WebKitImplementation*>(userData);
                    browser->SetFPS();
                }, this, nullptr);
            }

            auto* userContentManager = webkit_web_view_get_user_content_manager(_view);
            webkit_user_content_manager_register_script_message_handler_in_world(userContentManager, "wpeNotifyWPEFramework", std::to_string(_guid).c_str());
            g_signal_connect(userContentManager, "script-message-received::wpeNotifyWPEFramework",
                reinterpret_cast<GCallback>(wpeNotifyWPEFrameworkMessageReceivedCallback), this);

            g_signal_connect(_view, "decide-policy", reinterpret_cast<GCallback>(decidePolicyCallback), nullptr);
            g_signal_connect(_view, "notify::uri", reinterpret_cast<GCallback>(uriChangedCallback), this);
            g_signal_connect(_view, "load-changed", reinterpret_cast<GCallback>(loadChangedCallback), this);
            g_signal_connect(_view, "web-process-terminated", reinterpret_cast<GCallback>(webProcessTerminatedCallback), nullptr);
            g_signal_connect(_view, "close", reinterpret_cast<GCallback>(closeCallback), this);
            g_signal_connect(_view, "permission-request", reinterpret_cast<GCallback>(decidePermissionCallback), nullptr);
            g_signal_connect(_view, "show-notification", reinterpret_cast<GCallback>(showNotificationCallback), this);

            _configurationCompleted.SetState(true);

            // Move into the correct state, as requested
            auto* backend = webkit_web_view_backend_get_wpe_backend(webkit_web_view_get_backend(_view));
            _adminLock.Lock();
            if ((_state == PluginHost::IStateControl::SUSPENDED) || (_state == PluginHost::IStateControl::UNINITIALIZED)) {
                _state = PluginHost::IStateControl::UNINITIALIZED;
                wpe_view_backend_add_activity_state(backend, wpe_view_activity_state_visible);
                OnStateChange(PluginHost::IStateControl::SUSPENDED);
            } else {
                _state = PluginHost::IStateControl::UNINITIALIZED;
                wpe_view_backend_add_activity_state(backend, wpe_view_activity_state_visible | wpe_view_activity_state_focused | wpe_view_activity_state_in_window);
                OnStateChange(PluginHost::IStateControl::RESUMED);
            }
            _adminLock.Unlock();

            g_main_loop_run(_loop);

            if (frameDisplayedCallbackID)
                webkit_web_view_remove_frame_displayed_callback(_view, frameDisplayedCallbackID);
            webkit_user_content_manager_unregister_script_message_handler_in_world(userContentManager, "wpeNotifyWPEFramework", std::to_string(_guid).c_str());

            g_clear_object(&_view);
            g_main_context_pop_thread_default(_context);
            g_main_loop_unref(_loop);
            g_main_context_unref(_context);

            return Core::infinite;
        }
#else
        uint32_t Worker() override
        {
            _context = g_main_context_new();
            _loop = g_main_loop_new(_context, FALSE);
            g_main_context_push_thread_default(_context);

            HangDetector hangdetector(*this);

            auto contextConfiguration = WKContextConfigurationCreate();

            if (_config.InjectedBundle.Value().empty() == false) {
                // Set up injected bundle. Will be loaded once WPEWebProcess is started.
                std::unique_ptr<gchar, GCharDeleter> bundlePath(
                    g_build_filename(_dataPath.c_str(), _config.InjectedBundle.Value().c_str(), nullptr) );
                if (g_file_test(bundlePath.get(), G_FILE_TEST_EXISTS) == false) {
                    bundlePath.reset( g_build_filename("/usr/share/WPEFramework/WebKitBrowser/", _config.InjectedBundle.Value().c_str(), nullptr) );
                }
                WKStringRef injectedBundlePathString = WKStringCreateWithUTF8CString(bundlePath.get());
                WKContextConfigurationSetInjectedBundlePath(contextConfiguration, injectedBundlePathString);
                WKRelease(injectedBundlePathString);
            }

            gchar* wpeStoragePath;
            if (_config.LocalStorage.IsSet() == true && _config.LocalStorage.Value().empty() == false)
                wpeStoragePath = g_build_filename(_config.LocalStorage.Value().c_str(), "wpe", "local-storage", nullptr);
            else
                wpeStoragePath = g_build_filename(g_get_user_cache_dir(), "wpe", "local-storage", nullptr);

            g_mkdir_with_parents(wpeStoragePath, 0700);
            auto storageDirectory = WKStringCreateWithUTF8CString(wpeStoragePath);
            g_free(wpeStoragePath);
            WKContextConfigurationSetLocalStorageDirectory(contextConfiguration, storageDirectory);

            if (_config.LocalStorageSize.IsSet() == true && _config.LocalStorageSize.Value() != 0) {
                uint32_t gLocalStorageDatabaseQuotaInBytes = _config.LocalStorageSize.Value() * 1024;
                TRACE(Trace::Information, (_T("Configured LocalStorage Quota  %u bytes"), gLocalStorageDatabaseQuotaInBytes));
                WKContextConfigurationSetLocalStorageQuota(contextConfiguration, gLocalStorageDatabaseQuotaInBytes);
            }

            gchar* wpeDiskCachePath = g_build_filename(g_get_user_cache_dir(), "wpe", "disk-cache", nullptr);
            g_mkdir_with_parents(wpeDiskCachePath, 0700);
            auto diskCacheDirectory = WKStringCreateWithUTF8CString(wpeDiskCachePath);
            g_free(wpeDiskCachePath);
            WKContextConfigurationSetDiskCacheDirectory(contextConfiguration, diskCacheDirectory);

            WKContextRef context = WKContextCreateWithConfiguration(contextConfiguration);
            WKSoupSessionSetIgnoreTLSErrors(context, !_config.CertificateCheck);

            if (_config.Languages.IsSet()) {
                WKMutableArrayRef languages = WKMutableArrayCreate();
                Core::JSON::ArrayType<Core::JSON::String>::Iterator index(_config.Languages.Elements());

                while (index.Next() == true) {
                    WKStringRef itemString = WKStringCreateWithUTF8CString(index.Current().Value().c_str());
                    WKArrayAppendItem(languages, itemString);
                    WKRelease(itemString);
                }

                WKSoupSessionSetPreferredLanguages(context, languages);
                WKRelease(languages);
            }

            WKRelease(contextConfiguration);

            WKGeolocationManagerRef geolocationManager = WKContextGetGeolocationManager(context);
            WKGeolocationManagerSetProvider(geolocationManager, &_handlerGeolocationProvider.base);

            _notificationManager = WKContextGetNotificationManager(context);
            _handlerNotificationProvider.base.clientInfo = static_cast<void*>(this);
            WKNotificationManagerSetProvider(_notificationManager, &_handlerNotificationProvider.base);

            auto pageGroupIdentifier = WKStringCreateWithUTF8CString(_config.PageGroup.Value().c_str());
            _pageGroup = WKPageGroupCreateWithIdentifier(pageGroupIdentifier);
            WKRelease(pageGroupIdentifier);

            auto preferences = WKPreferencesCreate();

            _allowMixedContent = !_config.Secure.Value();
            WKPreferencesSetAllowRunningOfInsecureContent(preferences, _allowMixedContent);
            WKPreferencesSetAllowDisplayOfInsecureContent(preferences, _allowMixedContent);

            // WebSecurity
            WKPreferencesSetWebSecurityEnabled(preferences, !_allowMixedContent);

            // Turn off log message to stdout.
            WKPreferencesSetLogsPageMessagesToSystemConsoleEnabled(preferences, _config.LogToSystemConsoleEnabled.Value());

            // Turn on gamepads.
            WKPreferencesSetGamepadsEnabled(preferences, true);

            // Turn on fullscreen API.
            WKPreferencesSetFullScreenEnabled(preferences, true);

            // Turn on/off allowScriptWindowClose
            WKPreferencesSetAllowScriptsToCloseWindow(preferences, _config.AllowWindowClose.Value());

            // Turn on/off non composited WebGL
            WKPreferencesSetNonCompositedWebGLEnabled(preferences, _config.NonCompositedWebGLEnabled.Value());

            // Turn on/off WebGL
            WKPreferencesSetWebGLEnabled(preferences, _config.WebGLEnabled.Value());

            // Turn on/off local storage
            WKPreferencesSetLocalStorageEnabled(preferences, _localStorageEnabled);

            // Media Content Types Requiring Hardware Support
            if (_config.MediaContentTypesRequiringHardwareSupport.IsSet() == true
                && _config.MediaContentTypesRequiringHardwareSupport.Value().empty() == false) {
                auto contentTypes = WKStringCreateWithUTF8CString(
                    _config.MediaContentTypesRequiringHardwareSupport.Value().c_str());
                WKPreferencesSetMediaContentTypesRequiringHardwareSupport(preferences, contentTypes);
                WKRelease(contentTypes);
            }

            WKPageGroupSetPreferences(_pageGroup, preferences);

            auto pageConfiguration = WKPageConfigurationCreate();
            WKPageConfigurationSetContext(pageConfiguration, context);
            WKPageConfigurationSetPageGroup(pageConfiguration, _pageGroup);

            gchar* cookieDatabasePath;

            if (_config.CookieStorage.IsSet() == true && _config.CookieStorage.Value().empty() == false)
                cookieDatabasePath = g_build_filename(_config.CookieStorage.Value().c_str(), "cookies.db", nullptr);
            else
                cookieDatabasePath = g_build_filename(g_get_user_cache_dir(), "cookies.db", nullptr);

            auto path = WKStringCreateWithUTF8CString(cookieDatabasePath);
            g_free(cookieDatabasePath);
            auto cookieManager = WKContextGetCookieManager(context);
            WKCookieManagerSetCookiePersistentStorage(cookieManager, path, kWKCookieStorageTypeSQLite);
            WKCookieManagerSetHTTPCookieAcceptPolicy(cookieManager, _httpCookieAcceptPolicy);

#ifdef WPE_WEBKIT_DEPRECATED_API
            _view = WKViewCreateWithViewBackend(wpe_view_backend_create(), pageConfiguration);
#else
            _view = WKViewCreate(wpe_view_backend_create(), pageConfiguration);
#endif
            if (_config.FPS.Value() == true) {
                _viewClient.base.clientInfo = static_cast<void*>(this);
                WKViewSetViewClient(_view, &_viewClient.base);
            }

            //_page = WKRetain(WKViewGetPage(_view));
            _page = WKViewGetPage(_view);

            if (_config.Transparent.Value() == true)
                WKPageSetDrawsBackground(_page, false);

            // Register handlers for page navigation and message from injected bundle.
            _handlerWebKit.base.clientInfo = static_cast<void*>(this);
            WKPageSetPageNavigationClient(_page, &_handlerWebKit.base);

            _handlerInjectedBundle.base.clientInfo = static_cast<void*>(this);
            WKContextSetInjectedBundleClient(context, &_handlerInjectedBundle.base);

            WKPageSetProxies(_page, nullptr);

            WKPageSetCustomBackingScaleFactor(_page, _config.ScaleFactor.Value());

            if (_config.Automation.Value()) {
                _handlerAutomation.base.clientInfo = static_cast<void*>(this);
                WKContextSetAutomationClient(context, &_handlerAutomation.base);
            }

            WKPageSetPageUIClient(_page, &_handlerPageUI.base);

            WKPageLoaderClientV0 pageLoadClient;
            memset(&pageLoadClient, 0, sizeof(pageLoadClient));
            pageLoadClient.base.clientInfo = this;
            pageLoadClient.processDidBecomeResponsive = WebKitImplementation::WebProcessDidBecomeResponsive;
            WKPageSetPageLoaderClient(_page, &pageLoadClient.base);

            if (_config.UserAgent.IsSet() == true && _config.UserAgent.Value().empty() == false) {
                auto ua = WKStringCreateWithUTF8CString(_config.UserAgent.Value().c_str());
                WKPageSetCustomUserAgent(_page, ua);
                WKRelease(ua);
                TRACE(Trace::Information, (_T("New user agent: '%s'"), _config.UserAgent.Value().c_str()));
            } else {
                auto ua = WKPageCopyUserAgent(_page);
                _config.UserAgent = WKStringToString(ua);
                WKRelease(ua);
                TRACE(Trace::Information, (_T("Current user agent: '%s'"), _config.UserAgent.Value().c_str()));
            }

            // Move into the correct state, as requested
            _adminLock.Lock();
            if ((_state == PluginHost::IStateControl::SUSPENDED) || (_state == PluginHost::IStateControl::UNINITIALIZED)) {
                _state = PluginHost::IStateControl::UNINITIALIZED;
                Suspend();
            } else {
                _state = PluginHost::IStateControl::UNINITIALIZED;
                OnStateChange(PluginHost::IStateControl::RESUMED);
            }
            _adminLock.Unlock();

            _configurationCompleted.SetState(true);

            g_main_loop_run(_loop);

            // Seems if we stop the mainloop but are not in a suspended state, there is a crash.
            // Force suspended state first.
            if (_state == PluginHost::IStateControl::RESUMED) {
                WKViewSetViewState(_view, 0);
            }

            // WKRelease(_page);
            if (_automationSession) WKRelease(_automationSession);

            WKRelease(_view);
            WKRelease(pageConfiguration);
            WKRelease(_pageGroup);
            WKRelease(context);
            WKRelease(preferences);

            g_main_context_pop_thread_default(_context);
            g_main_loop_unref(_loop);
            g_main_context_unref(_context);

            return Core::infinite;
        }

        void CheckWebProcess()
        {
            if ( _webProcessCheckInProgress )
                return;
            _webProcessCheckInProgress = true;

            WKPageIsWebProcessResponsive(
                _page,
                this,
                [](bool isWebProcessResponsive, void* customdata) {
                    WebKitImplementation* object = static_cast<WebKitImplementation*>(customdata);
                    object->DidReceiveWebProcessResponsivenessReply(isWebProcessResponsive);
                });
        }

        void DidReceiveWebProcessResponsivenessReply(bool isWebProcessResponsive)
        {
            if (_config.WatchDogHangThresholdInSeconds.Value() == 0 || _config.WatchDogCheckTimeoutInSeconds.Value() == 0)
                return;

            // How many unresponsive replies to ignore before declaring WebProcess hang state
            const uint32_t kWebProcessUnresponsiveReplyDefaultLimit =
                _config.WatchDogHangThresholdInSeconds.Value() / _config.WatchDogCheckTimeoutInSeconds.Value();

            if (!_webProcessCheckInProgress)
                return;
            _webProcessCheckInProgress = false;

            if (isWebProcessResponsive && _unresponsiveReplyNum == 0)
                return;

            std::string activeURL = GetPageActiveURL(GetPage());
            pid_t webprocessPID = WKPageGetProcessIdentifier(GetPage());

            if (isWebProcessResponsive)
            {
                SYSLOG(Logging::Notification, (_T("WebProcess recovered after %d unresponsive replies, pid=%u, url=%s\n"),
                                            _unresponsiveReplyNum, webprocessPID, activeURL.c_str()));
                _unresponsiveReplyNum = 0;
            }
            else
            {
                ++_unresponsiveReplyNum;
                SYSLOG(Logging::Notification, (_T("WebProcess is unresponsive, pid=%u, reply num=%d(max=%d), url=%s\n"),
                                            webprocessPID, _unresponsiveReplyNum, kWebProcessUnresponsiveReplyDefaultLimit,
                                            activeURL.c_str()));
            }

            if (!isWebProcessResponsive && _state == PluginHost::IStateControl::SUSPENDED)
            {
                SYSLOG(Logging::Notification, (_T("Killing unresponsive suspended WebProcess, pid=%u, reply num=%d(max=%d), url=%s\n"),
                                            webprocessPID, _unresponsiveReplyNum, kWebProcessUnresponsiveReplyDefaultLimit,
                                            activeURL.c_str()));
                if (_unresponsiveReplyNum <= kWebProcessUnresponsiveReplyDefaultLimit)
                {
                    _unresponsiveReplyNum = kWebProcessUnresponsiveReplyDefaultLimit;
                    Logging::DumpSystemFiles(webprocessPID);
                    if (syscall(__NR_tgkill, webprocessPID, webprocessPID, SIGFPE) == -1)
                    {
                        SYSLOG(Trace::Error, (_T("tgkill failed, signal=%d process=%u errno=%d (%s)"), SIGFPE, webprocessPID, errno, strerror(errno)));
                    }
                }
                else
                {
                    DeactivateBrowser(PluginHost::IShell::FAILURE);
                }
                return;
            }

            if (_unresponsiveReplyNum == kWebProcessUnresponsiveReplyDefaultLimit)
            {
                Logging::DumpSystemFiles(webprocessPID);

                if (syscall(__NR_tgkill, webprocessPID, webprocessPID, SIGFPE) == -1)
                {
                    SYSLOG(Trace::Error, (_T("tgkill failed, signal=%d process=%u errno=%d (%s)"), SIGFPE, webprocessPID, errno, strerror(errno)));
                }
            }
            else if (_unresponsiveReplyNum == (2 * kWebProcessUnresponsiveReplyDefaultLimit))
            {
                DeactivateBrowser(PluginHost::IShell::WATCHDOG_EXPIRED);
            }
        }

        static void WebProcessDidBecomeResponsive(WKPageRef page, const void* clientInfo)
        {
            auto &self = *const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
            if (self._unresponsiveReplyNum > 0)
            {
                std::string activeURL = GetPageActiveURL(page);
                pid_t webprocessPID = WKPageGetProcessIdentifier(page);
                SYSLOG(Logging::Notification, (_T("WebProcess recovered after %d unresponsive replies, pid=%u, url=%s\n"),
                                            self._unresponsiveReplyNum, webprocessPID, activeURL.c_str()));
                self._unresponsiveReplyNum = 0;
            }
        }
#endif // WEBKIT_GLIB_API

        void DeactivateBrowser(PluginHost::IShell::reason reason) {
            ASSERT(_service != nullptr);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, reason));
        }

    private:
        Config _config;

        struct {
            mutable std::mutex mutex;
            std::condition_variable cond;
            string url;
            uint32_t result = Core::ERROR_TIMEDOUT;
        } urlData_;

        string _dataPath;
        PluginHost::IShell* _service;
        string _headers;
        bool _localStorageEnabled;
        int32_t _httpStatusCode;

#ifdef WEBKIT_GLIB_API
        WebKitWebView* _view;
        uint64_t _guid;
#else
        WKViewRef _view;
        WKPageRef _page;
        WKPageGroupRef _pageGroup;
        WKWebAutomationSessionRef _automationSession;
        WKNotificationManagerRef _notificationManager;
        WKHTTPCookieAcceptPolicy _httpCookieAcceptPolicy;
        WKNavigationRef _navigationRef{nullptr};
#endif
        mutable Core::CriticalSection _adminLock;
        uint32_t _fps;
        GMainLoop* _loop;
        GMainContext* _context;
        std::list<Exchange::IWebBrowser::INotification*> _notificationClients;
        std::list<Exchange::IBrowser::INotification*> _notificationBrowserClients;
        std::list<PluginHost::IStateControl::INotification*> _stateControlClients;
        std::list<Exchange::IApplication::INotification*> _applicationClients;
        PluginHost::IStateControl::state _state;
        bool _hidden;
        uint64_t _time;
        bool _compliant;
        Core::StateTrigger<bool> _configurationCompleted;
        bool _webProcessCheckInProgress;
        uint32_t _unresponsiveReplyNum;
        unsigned _frameCount;
        gint64 _lastDumpTime;
        bool _allowMixedContent;
        std::list<string> _userScripts;
        std::list<string> _userStyleSheets;
        string _securityProfileName;
    };

    SERVICE_REGISTRATION(WebKitImplementation, 1, 0);

#ifndef WEBKIT_GLIB_API
    // Handles synchronous messages from injected bundle.
    /* static */ void onDidReceiveSynchronousMessageFromInjectedBundle(WKContextRef context, WKStringRef messageName,
        WKTypeRef messageBodyObj, WKTypeRef* returnData, const void* clientInfo)
    {
        int configLen = strlen(Tags::Config);
        const WebKitImplementation* browser = static_cast<const WebKitImplementation*>(clientInfo);

        string name = WKStringToString(messageName);

        // Depending on message name, select action.
        if (name == Tags::Notification) {
            // Message contains strings from custom JS handler "NotifyWebbridge".
            WKArrayRef messageLines = static_cast<WKArrayRef>(messageBodyObj);

            std::vector<string> messageStrings = ConvertWKArrayToStringVector(messageLines);
            const_cast<WebKitImplementation*>(browser)->OnJavaScript(messageStrings);
        } else if (name == Tags::BridgeObjectQuery) {
            WKStringRef messageBodyStr = static_cast<WKStringRef>(messageBodyObj);
            string messageText = WKStringToString(messageBodyStr);
            const_cast<WebKitImplementation*>(browser)->OnBridgeQuery(messageText);
        } else if (name == Tags::URL) {
            string url;
            static_cast<const WebKitImplementation*>(browser)->URL(url);
            *returnData = WKStringCreateWithUTF8CString(url.c_str());
        } else if (name.compare(0, configLen, Tags::Config) == 0) {
            // Second part of this string is the key we are looking for, extract it...
            std::string utf8Json = Core::ToString(browser->GetConfig(name.substr(configLen)));
            *returnData = WKStringCreateWithUTF8CString(utf8Json.c_str());
        } else {
            // Unexpected message name.
            std::cerr << "WebBridge received synchronous message (" << name << "), but didn't process it." << std::endl;
        }
    }

    /* static */ void didStartProvisionalNavigation(WKPageRef page, WKNavigationRef navigation, WKTypeRef, const void* clientInfo)
    {
        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));

        WKURLRef urlRef = WKPageCopyActiveURL(page);
        WKStringRef urlStringRef = WKURLCopyString(urlRef);

        string url = WKStringToString(urlStringRef);

        browser->NavigationRef(navigation);
        browser->OnURLChanged(url, true);

        WKRelease(urlRef);
        WKRelease(urlStringRef);
    }

    /* static */ void didSameDocumentNavigation(const OpaqueWKPage* page, const OpaqueWKNavigation*, WKSameDocumentNavigationType type, const void*, const void* info)
    {
        if (type == kWKSameDocumentNavigationAnchorNavigation) {
            WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(info));

            WKURLRef urlRef = WKPageCopyActiveURL(page);
            WKStringRef urlStringRef = WKURLCopyString(urlRef);

            string url = WKStringToString(urlStringRef);

            browser->OnURLChanged(url, false);

            WKRelease(urlRef);
            WKRelease(urlStringRef);
        }
    }

    /* static */ void didFinishDocumentLoad(WKPageRef page, WKNavigationRef navigation, WKTypeRef, const void* clientInfo)
    {

        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));

        WKURLRef urlRef = WKPageCopyActiveURL(page);
        WKStringRef urlStringRef = WKURLCopyString(urlRef);

        string url = WKStringToString(urlStringRef);

        browser->OnLoadFinished(url, navigation);

        WKRelease(urlRef);
        WKRelease(urlStringRef);
    }

    /* static */ void requestClosure(const void*)
    {
        // WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
        // TODO: @Igalia, make sure the clientInfo is actually holding the correct clientINfo, currently it is nullptr. For
        // now we use the Singleton, this is fine as long as there is only 1 instance (in process) or it is always fine if we
        // are running out-of-process..
        WebKitImplementation* realBrowser = static_cast<WebKitImplementation*>(implementation);
        realBrowser->NotifyClosure();
    }

    /* static */ void onNotificationShow(WKPageRef, WKNotificationRef notification, const void* clientInfo)
    {
        const WebKitImplementation* browser = static_cast<const WebKitImplementation*>(clientInfo);

        WKStringRef titleRef = WKNotificationCopyTitle(notification);
        WKStringRef bodyRef = WKNotificationCopyBody(notification);

        string title = WKStringToString(titleRef);
        string body = WKStringToString(bodyRef);

        TRACE_GLOBAL(HTML5Notification, (_T("%s - %s"), title.c_str(), body.c_str()));

        // Tell page we've "shown" the notification.
        uint64_t notificationID = WKNotificationGetID(notification);
        browser->OnNotificationShown(notificationID);

        WKRelease(bodyRef);
        WKRelease(titleRef);
    }

    /* static */ void onFrameDisplayed(WKViewRef, const void* clientInfo)
    {
        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
        browser->SetFPS();
    }

    /* static */ void didRequestAutomationSession(WKContextRef context, WKStringRef sessionID, const void* clientInfo)
    {
        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
        browser->OnRequestAutomationSession(context, sessionID);
    }

    /* static */ WKPageRef onAutomationSessionRequestNewPage(WKWebAutomationSessionRef, const void* clientInfo)
    {
        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
        return browser->GetPage();
    }

    /* static */ void decidePolicyForNavigationResponse(WKPageRef, WKNavigationResponseRef response, WKFramePolicyListenerRef listener, WKTypeRef, const void* clientInfo)
    {
        WKFramePolicyListenerUse(listener);
        if (WKNavigationResponseIsMainFrame(response))
        {
            WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
            WKURLResponseRef urlResponse = WKNavigationResponseGetURLResponse(response);
            browser->SetResponseHTTPStatusCode(WKURLResponseHTTPStatusCode(urlResponse));
            // WKRelease(urlResponse);
        }
    }

    /* static */ void didFailProvisionalNavigation(WKPageRef page, WKNavigationRef navigation, WKErrorRef error, WKTypeRef userData, const void *clientInfo)
    {
        didFailNavigation(page, navigation, error, userData, clientInfo);
    }

    /* static */ void didFailNavigation(WKPageRef, WKNavigationRef, WKErrorRef error, WKTypeRef, const void *clientInfo)
    {
        const int WebKitNetworkErrorCancelled = 302;
        auto errorDomain = WKErrorCopyDomain(error);
        bool isCanceled =
            errorDomain &&
            WKStringIsEqualToUTF8CString(errorDomain, "WebKitNetworkError") &&
            WebKitNetworkErrorCancelled == WKErrorGetErrorCode(error);
        WKRelease(errorDomain);

        if (isCanceled)
            return;

        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
        browser->OnLoadFailed();
    }

    /* static */ void webProcessDidCrash(WKPageRef, const void*)
    {
        SYSLOG(Trace::Fatal, (_T("CRASH: WebProcess crashed, exiting...")));
        exit(1);
    }
#endif // !WEBKIT_GLIB_API
} // namespace Plugin

namespace WebKitBrowser {

    // TODO: maybe nice to expose this in the config.json
    static const TCHAR* mandatoryProcesses[] = {
        _T("WPENetworkProcess"),
        _T("WPEWebProcess")
    };

    static constexpr uint16_t RequiredChildren = (sizeof(mandatoryProcesses) / sizeof(mandatoryProcesses[0]));
    class MemoryObserverImpl : public Exchange::IMemory {
    private:
        MemoryObserverImpl();
        MemoryObserverImpl(const MemoryObserverImpl&);
        MemoryObserverImpl& operator=(const MemoryObserverImpl&);

        enum { TYPICAL_STARTUP_TIME = 10 }; /* in Seconds */
    public:
        MemoryObserverImpl(const RPC::IRemoteConnection* connection)
            : _main(connection == nullptr ? Core::ProcessInfo().Id() : connection->RemoteId())
            , _children(_main.Id())
            , _startTime(connection == nullptr ? 0 : Core::Time::Now().Add(TYPICAL_STARTUP_TIME * 1000).Ticks())
        { // IsOperation true till calculated time (microseconds)
        }
        ~MemoryObserverImpl()
        {
        }

    public:
        uint64_t Resident() const override
        {
            uint32_t result(0);

            if (_startTime != 0) {
                if (_children.Count() < RequiredChildren) {
                    _children = Core::ProcessInfo::Iterator(_main.Id());
                }

                result = _main.Resident();

                _children.Reset();

                while (_children.Next() == true) {
                    result += _children.Current().Resident();
                }
            }

            return (result);
        }
        uint64_t Allocated() const override
        {
            uint32_t result(0);

            if (_startTime != 0) {
                if (_children.Count() < RequiredChildren) {
                    _children = Core::ProcessInfo::Iterator(_main.Id());
                }

                result = _main.Allocated();

                _children.Reset();

                while (_children.Next() == true) {
                    result += _children.Current().Allocated();
                }
            }

            return (result);
        }
        uint64_t Shared() const override
        {
            uint32_t result(0);

            if (_startTime != 0) {
                if (_children.Count() < RequiredChildren) {
                    _children = Core::ProcessInfo::Iterator(_main.Id());
                }

                result = _main.Shared();

                _children.Reset();

                while (_children.Next() == true) {
                    result += _children.Current().Shared();
                }
            }

            return (result);
        }
        uint8_t Processes() const override
        {
            // Refresh the children list !!!
            _children = Core::ProcessInfo::Iterator(_main.Id());
            return ((_startTime == 0) || (_main.IsActive() == true) ? 1 : 0) + _children.Count();
        }
        const bool IsOperational() const override
        {
            uint32_t requiredProcesses = 0;

            if (_startTime != 0) {

                //!< We can monitor a max of 32 processes, every mandatory process represents a bit in the requiredProcesses.
                // In the end we check if all bits are 0, what means all mandatory processes are still running.
                requiredProcesses = (0xFFFFFFFF >> (32 - RequiredChildren));

                if (_children.Count() < RequiredChildren) {
                    // Refresh the children list !!!
                    _children = Core::ProcessInfo::Iterator(_main.Id());
                }
                //!< If there are less children than in the the mandatoryProcesses struct, we are done and return false.
                if (_children.Count() >= RequiredChildren) {

                    _children.Reset();

                    //!< loop over all child processes as long as we are operational.
                    while ((requiredProcesses != 0) && (true == _children.Next())) {

                        uint8_t count(0);
                        string name(_children.Current().Name());

                        while ((count < RequiredChildren) && (name != mandatoryProcesses[count])) {
                            ++count;
                        }

                        //<! this is a mandatory process and if its still active reset its bit in requiredProcesses.
                        //   If not we are not completely operational.
                        if ((count < RequiredChildren) && (_children.Current().IsActive() == true)) {
                            requiredProcesses &= (~(1 << count));
                        }
                    }
                }
            }

            return (((requiredProcesses == 0) || (true == IsStarting())) && (true == _main.IsActive()));
        }

        BEGIN_INTERFACE_MAP(MemoryObserverImpl)
        INTERFACE_ENTRY(Exchange::IMemory)
        END_INTERFACE_MAP

    private:
        inline const bool IsStarting() const
        {
            return (_startTime == 0) || (Core::Time::Now().Ticks() < _startTime);
        }

    private:
        Core::ProcessInfo _main;
        mutable Core::ProcessInfo::Iterator _children;
        uint64_t _startTime; // !< Reference for monitor
    };

    Exchange::IMemory* MemoryObserver(const RPC::IRemoteConnection* connection)
    {
        Exchange::IMemory* result = Core::Service<MemoryObserverImpl>::Create<Exchange::IMemory>(connection);
        return (result);
    }
} // namespace WebKitBrowser
} // namespace WPEFramework
