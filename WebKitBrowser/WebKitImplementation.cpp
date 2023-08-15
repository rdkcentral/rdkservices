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

#include <memory>
#include <utility>
#include <tuple>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "Module.h"

#ifdef WEBKIT_GLIB_API
#include <wpe/webkit.h>
#include "Tags.h"
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
#include <WPE/WebKit/WKSoupSession.h>
#include <WPE/WebKit/WKUserMediaPermissionRequest.h>
#include <WPE/WebKit/WKErrorRef.h>

#include "BrowserConsoleLog.h"
#include "Tags.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef pid_t WKProcessID;
typedef void (*WKPageIsWebProcessResponsiveFunction)(bool isWebProcessResponsive, void* context);
WK_EXPORT void WKPageIsWebProcessResponsive(WKPageRef page, void* context, WKPageIsWebProcessResponsiveFunction function);
WK_EXPORT WKProcessID WKPageGetProcessIdentifier(WKPageRef page);
WK_EXPORT void WKPreferencesSetPageCacheEnabled(WKPreferencesRef preferences, bool enabled);

#ifdef __cplusplus
}
#endif

#endif

#include <wpe/wpe.h>

#include <glib.h>

#include "HTML5Notification.h"
#include "WebKitBrowser.h"

#if defined(ENABLE_CLOUD_COOKIE_JAR)
#include "CookieJar.h"
#include <libsoup/soup.h>
#endif

#if defined(ENABLE_LOGGING_UTILS)
#include "LoggingUtils.h"
#endif


#if !WEBKIT_GLIB_API
#define HAS_MEMORY_PRESSURE_SETTINGS_API 0
#else
#define HAS_MEMORY_PRESSURE_SETTINGS_API WEBKIT_CHECK_VERSION(2, 38, 0)
#endif


namespace WPEFramework {
namespace Plugin {

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
    static void willAddDetailedMessageToConsole(WKPageRef, WKStringRef source, WKStringRef, uint64_t line, uint64_t column, WKStringRef message, WKStringRef, const void* clientInfo);

    struct GCharDeleter {
        void operator()(gchar* ptr) const { g_free(ptr); }
    };

    // -----------------------------------------------------------------------------------------------------
    // Hide all NASTY C details that come with the POC libraries !!!!!
    // -----------------------------------------------------------------------------------------------------
    static WKPageNavigationClientV0 _handlerWebKit = {
        { 0, nullptr },
        // decidePolicyForNavigationAction
        [](WKPageRef, WKNavigationActionRef, WKFramePolicyListenerRef listener, WKTypeRef, const void* customData) {
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
        [](WKGeolocationManagerRef geolocationManager, const void* clientInfo) {
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
        [](WKPageRef page, WKFrameRef frame, WKSecurityOriginRef origin, WKGeolocationPermissionRequestRef permissionRequest, const void* clientInfo) {
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
        [](WKPageRef page, WKSecurityOriginRef origin, WKNotificationPermissionRequestRef permissionRequest, const void* clientInfo) {
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
            if (WKArrayGetSize(audioDevices) > 0) {
                audioDevice = static_cast<WKStringRef>(WKArrayGetItemAtIndex(audioDevices, 0));
            }
            auto videoDevice = WKStringCreateWithUTF8CString("NO VIDEO DEVICE FOUND");
            if (WKArrayGetSize(videoDevices) > 0) {
                videoDevice = static_cast<WKStringRef>(WKArrayGetItemAtIndex(videoDevices, 0));
            }
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
        willAddDetailedMessageToConsole,
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
                                 public Exchange::IApplication,
                                 public Exchange::IBrowserScripting,
                                 #if defined(ENABLE_CLOUD_COOKIE_JAR)
                                 public Exchange::IBrowserCookieJar,
                                 #endif
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
            class EnvironmentVariable : public Core::JSON::Container {
            public:
                EnvironmentVariable(const EnvironmentVariable& origin)
                    : Core::JSON::Container()
                    , Name(origin.Name)
                    , Value(origin.Value)
                {
                    Add(_T("name"), &Name);
                    Add(_T("value"), &Value);
                }
                EnvironmentVariable& operator=(const EnvironmentVariable&) = delete;

                EnvironmentVariable()
                    : Core::JSON::Container()
                    , Name("")
                    , Value("")
                {
                    Add(_T("name"), &Name);
                    Add(_T("value"), &Value);
                }
                ~EnvironmentVariable() = default;

            public:
                Core::JSON::String Name;
                Core::JSON::String Value;
            };

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
            class MemorySettings : public Core::JSON::Container {
            public:
                class Settings : public Core::JSON::Container {
                public:
                    Settings(const Settings&) = delete;
                    Settings& operator=(const Settings&) = delete;

                    Settings()
                        : Core::JSON::Container()
                        , PollInterval()
                        , Limit()
                    {
                        Add(_T("pollinterval"), &PollInterval);
                        Add(_T("limit"), &Limit);
                    }
                    ~Settings()
                    {
                    }

                public:
                    Core::JSON::DecUInt32 PollInterval;
                    Core::JSON::DecUInt32 Limit;
                };

                class WebProcess : public Settings {
                public:
                    WebProcess(const WebProcess&) = delete;
                    WebProcess& operator=(const WebProcess&) = delete;

                    WebProcess()
                        : Settings()
                        , GPULimit()
                        , GPUFile()
                    {
                        Add(_T("gpulimit"), &GPULimit);
                        Add(_T("gpufile"), &GPUFile);
                    }
                    ~WebProcess()
                    {
                    }

                public:
                    Core::JSON::DecUInt32 GPULimit;
                    Core::JSON::String GPUFile;
                };
            public:
                MemorySettings(const MemorySettings&) = delete;
                MemorySettings& operator=(const MemorySettings&) = delete;

                MemorySettings()
                    : Core::JSON::Container()
                    , WebProcessSettings()
                    , NetworkProcessSettings()
                {
                    Add(_T("webprocesssettings"), &WebProcessSettings);
                    Add(_T("networkprocesssettings"), &NetworkProcessSettings);
                }
                ~MemorySettings()
                {
                }

            public:
                WebProcess WebProcessSettings;
                Settings NetworkProcessSettings;
            };

        public:
            Config()
                : Core::JSON::Container()
                , UserAgent()
                , URL(_T("http://www.google.com"))
                , Whitelist()
                , PageGroup(_T("WPEPageGroup"))
                , CookieStorage()
                , CloudCookieJarEnabled(false)
                , LocalStorage()
                , LocalStorageEnabled(false)
                , LocalStorageSize()
                , IndexedDBEnabled(false)
                , IndexedDBPath()
                , IndexedDBSize()
                , Secure(false)
                , InjectedBundle()
                , Transparent(false)
                , Compositor()
                , Inspector()
                , InspectorNative()
                , FPS(false)
                , Cursor(false)
                , Touch(false)
                , MSEBuffers()
                , ThunderDecryptorPreference()
                , MemoryProfile()
                , Memory()
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
                , ExtensionDir("Extension")
                , HTTPProxy()
                , HTTPProxyExclusion()
                , TCPKeepAlive(false)
                , ClientCert()
                , ClientCertKey()
                , LogToSystemConsoleEnabled(false)
                , WatchDogCheckTimeoutInSeconds(0)
                , WatchDogHangThresholdInSeconds(0)
                , LoadBlankPageOnSuspendEnabled(false)
                , UserScripts()
                , AllowFileURLsCrossAccess()
                , SpatialNavigation()
                , CookieAcceptPolicy()
                , EnvironmentVariables()
                , ContentFilter()
                , LoggingTarget()
                , WebAudioEnabled(false)
            {
                Add(_T("useragent"), &UserAgent);
                Add(_T("url"), &URL);
                Add(_T("whitelist"), &Whitelist);
                Add(_T("pagegroup"), &PageGroup);
                Add(_T("cookiestorage"), &CookieStorage);
                Add(_T("cloudcookiejarenabled"), &CloudCookieJarEnabled);
                Add(_T("localstorage"), &LocalStorage);
                Add(_T("localstorageenabled"), &LocalStorageEnabled);
                Add(_T("localstoragesize"), &LocalStorageSize);
                Add(_T("indexeddbenabled"), &IndexedDBEnabled);
                Add(_T("indexeddbpath"), &IndexedDBPath);
                Add(_T("indexeddbsize"), &IndexedDBSize);
                Add(_T("secure"), &Secure);
                Add(_T("injectedbundle"), &InjectedBundle);
                Add(_T("transparent"), &Transparent);
                Add(_T("compositor"), &Compositor);
                Add(_T("inspector"), &Inspector);
                Add(_T("inspectornative"), &InspectorNative);
                Add(_T("fps"), &FPS);
                Add(_T("cursor"), &Cursor);
                Add(_T("touch"), &Touch);
                Add(_T("msebuffers"), &MSEBuffers);
                Add(_T("thunderdecryptorpreference"), &ThunderDecryptorPreference);
                Add(_T("memoryprofile"), &MemoryProfile);
                Add(_T("memory"), &Memory);
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
                Add(_T("extensiondir"), &ExtensionDir);
                Add(_T("proxy"), &HTTPProxy);
                Add(_T("proxyexclusion"), &HTTPProxyExclusion);
                Add(_T("tcpkeepalive"), &TCPKeepAlive);
                Add(_T("clientcert"), &ClientCert);
                Add(_T("clientcertkey"), &ClientCertKey);
                Add(_T("logtosystemconsoleenabled"), &LogToSystemConsoleEnabled);
                Add(_T("watchdogchecktimeoutinseconds"), &WatchDogCheckTimeoutInSeconds);
                Add(_T("watchdoghangthresholdtinseconds"), &WatchDogHangThresholdInSeconds);
                Add(_T("loadblankpageonsuspendenabled"), &LoadBlankPageOnSuspendEnabled);
                Add(_T("userscripts"), &UserScripts);
                Add(_T("allowfileurlscrossaccess"), &AllowFileURLsCrossAccess);
                Add(_T("spatialnavigation"), &SpatialNavigation);
                Add(_T("cookieacceptpolicy"), &CookieAcceptPolicy);
                Add(_T("environmentvariables"), &EnvironmentVariables);
                Add(_T("contentfilter"), &ContentFilter);
                Add(_T("loggingtarget"), &LoggingTarget);
                Add(_T("webaudio"), &WebAudioEnabled);
            }
            ~Config()
            {
            }

        public:
            Core::JSON::String UserAgent;
            Core::JSON::String URL;
            Core::JSON::String Whitelist;
            Core::JSON::String PageGroup;
            Core::JSON::String CookieStorage;
            Core::JSON::Boolean CloudCookieJarEnabled;
            Core::JSON::String LocalStorage;
            Core::JSON::Boolean LocalStorageEnabled;
            Core::JSON::DecUInt16 LocalStorageSize;
            Core::JSON::Boolean IndexedDBEnabled;
            Core::JSON::String IndexedDBPath;
            Core::JSON::DecUInt16 IndexedDBSize; // [KB]
            Core::JSON::Boolean Secure;
            Core::JSON::String InjectedBundle;
            Core::JSON::Boolean Transparent;
            Core::JSON::String Compositor;
            Core::JSON::String Inspector;
            Core::JSON::Boolean InspectorNative;
            Core::JSON::Boolean FPS;
            Core::JSON::Boolean Cursor;
            Core::JSON::Boolean Touch;
            Core::JSON::String MSEBuffers;
            Core::JSON::Boolean ThunderDecryptorPreference;
            Core::JSON::String MemoryProfile;
            MemorySettings Memory;
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
            Core::JSON::String ExtensionDir;
            Core::JSON::String HTTPProxy;
            Core::JSON::String HTTPProxyExclusion;
            Core::JSON::Boolean TCPKeepAlive;
            Core::JSON::String ClientCert;
            Core::JSON::String ClientCertKey;
            Core::JSON::Boolean LogToSystemConsoleEnabled;
            Core::JSON::DecUInt16 WatchDogCheckTimeoutInSeconds;   // How often to check main event loop for responsiveness
            Core::JSON::DecUInt16 WatchDogHangThresholdInSeconds;  // The amount of time to give a process to recover before declaring a hang state
            Core::JSON::Boolean LoadBlankPageOnSuspendEnabled;
            Core::JSON::ArrayType<Core::JSON::String> UserScripts;
            Core::JSON::Boolean AllowFileURLsCrossAccess;
            Core::JSON::Boolean SpatialNavigation;
            Core::JSON::EnumType<HTTPCookieAcceptPolicyType> CookieAcceptPolicy;
            Core::JSON::ArrayType<EnvironmentVariable> EnvironmentVariables;
            Core::JSON::String ContentFilter;
            Core::JSON::String LoggingTarget;
            Core::JSON::Boolean WebAudioEnabled;
        };

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

                _worker.Reschedule(Core::Time::Now().Add(_watchDogTimeoutInSeconds * 1000));
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

                _worker.Reschedule(Core::Time::Now().Add(_watchDogTimeoutInSeconds * 1000));
            }

            HangDetector(const HangDetector&) = delete;
            HangDetector& operator=(const HangDetector&) = delete;
        };

    private:
        WebKitImplementation(const WebKitImplementation&) = delete;
        WebKitImplementation& operator=(const WebKitImplementation&) = delete;

    public:
        WebKitImplementation()
            : Core::Thread(0, _T("WebKitBrowser"))
            , _config()
            , _URL()
            , _dataPath()
            , _service(nullptr)
            , _headers()
            , _localStorageEnabled(false)
            , _httpStatusCode(-1)
#ifdef WEBKIT_GLIB_API
            , _view(nullptr)
            , _guid(Core::Time::Now().Ticks())
            , _httpCookieAcceptPolicy(WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY)
            , _webprocessPID(-1)
            , _extensionPath()
            , _ignoreLoadFinishedOnce(false)
#else
            , _view()
            , _page()
            , _automationSession(nullptr)
            , _notificationManager()
            , _httpCookieAcceptPolicy(kWKHTTPCookieAcceptPolicyOnlyFromMainDocumentDomain)
            , _navigationRef(nullptr)
            , _userContentController(nullptr)
#endif
            , _adminLock()
            , _fps(0)
            , _loop(nullptr)
            , _context(nullptr)
            , _notificationClients()
            , _notificationBrowserClients()
            , _stateControlClients()
            , _applicationClients()
            , _state(PluginHost::IStateControl::UNINITIALIZED)
            , _hidden(false)
            , _time(0)
            , _compliant(false)
            , _configurationCompleted(false)
            , _webProcessCheckInProgress(false)
            , _unresponsiveReplyNum(0)
            , _frameCount(0)
            , _lastDumpTime(g_get_monotonic_time())
        {
            // Register an @Exit, in case we are killed, with an incorrect ref count !!
            if (atexit(CloseDown) != 0) {
                TRACE(Trace::Information, (_T("Could not register @exit handler. Error: %d."), errno));
                exit(EXIT_FAILURE);
            }

            // The WebKitBrowser (WPE) can only be instantiated once (it is a process wide singleton !!!!)
            ASSERT(implementation == nullptr);

            implementation = this;
        }
        ~WebKitImplementation() override
        {
            Block();

            if (_loop != nullptr) {
                if (g_main_loop_is_running(_loop) == FALSE) {
                    g_main_context_invoke(_context, [](gpointer data) -> gboolean {
                        g_main_loop_quit(reinterpret_cast<GMainLoop*>(data));
                        return G_SOURCE_REMOVE;
                    }, _loop);
                }
                g_main_loop_quit(_loop);
            }

            if (Wait(Core::Thread::STOPPED | Core::Thread::BLOCKED, 6000) == false) {
                TRACE(Trace::Information, (_T("Bailed out before the end of the WPE main app was reached. %d"), 6000));
            }

            implementation = nullptr;
        }

    public:
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
#ifdef WEBKIT_GLIB_API
                        webkit_web_view_send_message_to_page(object->_view,
                                webkit_user_message_new(Tags::Headers, g_variant_new("s", headers.c_str())),
                                nullptr, nullptr, nullptr);
#else
                        auto messageName = WKStringCreateWithUTF8CString(Tags::Headers);
                        auto messageBody = WKStringCreateWithUTF8CString(headers.c_str());

                        WKPagePostMessageToInjectedBundle(object->_page, messageName, messageBody);

                        WKRelease(messageBody);
                        WKRelease(messageName);
#endif
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

            TRACE(Trace::Information, (_T("New user agent: %s"), useragent.c_str()));

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
#ifdef WEBKIT_GLIB_API
                    WebKitSettings* settings = webkit_web_view_get_settings(object->_view);
                    webkit_settings_set_user_agent(settings, useragent.c_str());
#else
                    auto ua = WKStringCreateWithUTF8CString(useragent.c_str());
                    WKPageSetCustomUserAgent(object->_page, ua);
                    WKRelease(ua);
#endif
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

#ifdef WEBKIT_GLIB_API
                    WebKitSettings* settings = webkit_web_view_get_settings(object->_view);
                    webkit_settings_set_enable_html5_local_storage(settings, enabled);
#else
                    auto group = WKPageGetPageGroup(object->_page);
                    auto preferences = WKPageGroupGetPreferences(group);
                    WKPreferencesSetLocalStorageEnabled(preferences, enabled);
#endif
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
#ifdef WEBKIT_GLIB_API
            auto translatePolicy =
                [](WebKitCookieAcceptPolicy policy) {
                    switch(policy) {
                        case WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS:
                            return Exchange::IWebBrowser::ALWAYS;
                        case WEBKIT_COOKIE_POLICY_ACCEPT_NEVER:
                            return Exchange::IWebBrowser::NEVER;
                        case WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY:
                            return Exchange::IWebBrowser::ONLY_FROM_MAIN_DOCUMENT_DOMAIN;
                    }
                    ASSERT(false);
                    return Exchange::IWebBrowser::ONLY_FROM_MAIN_DOCUMENT_DOMAIN;
                };
#else
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
#endif
            _adminLock.Lock();
            policy = translatePolicy(_httpCookieAcceptPolicy);
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t HTTPCookieAcceptPolicy(const HTTPCookieAcceptPolicyType policy) override
        {
            if (_context == nullptr)
                return Core::ERROR_GENERAL;

#ifdef WEBKIT_GLIB_API
            auto translatePolicy =
                [](Exchange::IWebBrowser::HTTPCookieAcceptPolicyType policy) {
                    switch(policy) {
                        case Exchange::IWebBrowser::ALWAYS:
                            return WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS;
                        case Exchange::IWebBrowser::NEVER:
                            return WEBKIT_COOKIE_POLICY_ACCEPT_NEVER;
                        case Exchange::IWebBrowser::ONLY_FROM_MAIN_DOCUMENT_DOMAIN:
                        case Exchange::IWebBrowser::EXCLUSIVELY_FROM_MAIN_DOCUMENT_DOMAIN:
                            return WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY;
                    }
                    ASSERT(false);
                    return WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY;
                };
            using SetHTTPCookieAcceptPolicyData = std::tuple<WebKitImplementation*, WebKitCookieAcceptPolicy>;
#else
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
#endif
            auto* data = new SetHTTPCookieAcceptPolicyData(this, translatePolicy(policy));

            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<SetHTTPCookieAcceptPolicyData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
#ifdef WEBKIT_GLIB_API
                    WebKitCookieAcceptPolicy policy =  std::get<1>(data);

                    object->_adminLock.Lock();
                    object->_httpCookieAcceptPolicy = policy;
                    object->_adminLock.Unlock();

                    WebKitWebContext* context = webkit_web_view_get_context(object->_view);
                    WebKitCookieManager* manager = webkit_web_context_get_cookie_manager(context);
                    webkit_cookie_manager_set_accept_policy(manager, policy);
#else
                    WKHTTPCookieAcceptPolicy policy =  std::get<1>(data);

                    object->_adminLock.Lock();
                    object->_httpCookieAcceptPolicy = policy;
                    object->_adminLock.Unlock();

                    auto context = WKPageGetContext(object->_page);
                    auto manager = WKContextGetCookieManager(context);
                    WKCookieManagerSetHTTPCookieAcceptPolicy(manager, policy);
#endif
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

#ifdef WEBKIT_GLIB_API
                    auto messageName = std::get<1>(data).c_str();
                    auto messageBody = std::get<2>(data).c_str();

                    webkit_web_view_send_message_to_page(object->_view,
                            webkit_user_message_new(messageName, g_variant_new("s", messageBody)),
                            nullptr, nullptr, nullptr);
#else
                    auto messageName = WKStringCreateWithUTF8CString(std::get<1>(data).c_str());
                    auto messageBody = WKStringCreateWithUTF8CString(std::get<2>(data).c_str());

                    WKPagePostMessageToInjectedBundle(object->_page, messageName, messageBody);

                    WKRelease(messageBody);
                    WKRelease(messageName);
#endif
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<BridgeMessageData*>(customdata);
                });
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
            [](gpointer) {
            });
            return Core::ERROR_NONE;
        }

        uint32_t RunJavaScript(const string& script) override
        {
            if (_context == nullptr)
                return Core::ERROR_GENERAL;

            using RunJavaScriptData = std::tuple<WebKitImplementation*, string>;
            auto* data = new RunJavaScriptData(this, script);

            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<RunJavaScriptData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    auto& script = std::get<1>(data);
#ifdef WEBKIT_GLIB_API
                    webkit_web_view_run_javascript(object->_view, script.c_str(), nullptr, nullptr, nullptr);
#else
                    auto scriptRef = WKStringCreateWithUTF8CString(script.c_str());
                    WKPageRunJavaScriptInMainFrame(object->_page, scriptRef, nullptr, [](WKSerializedScriptValueRef, WKErrorRef, void*){});
                    WKRelease(scriptRef);
#endif
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<RunJavaScriptData*>(customdata);
                });
            return Core::ERROR_NONE;
        }

        uint32_t AddUserScript(const string& script, bool topFrameOnly) override
        {
            if (_context == nullptr)
                return Core::ERROR_GENERAL;
            using AddUserScriptData = std::tuple<WebKitImplementation*, string, bool>;
            auto* data = new AddUserScriptData(this, script, topFrameOnly);
            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    auto& data = *static_cast<AddUserScriptData*>(customdata);
                    WebKitImplementation* object = std::get<0>(data);
                    const auto& scriptContent = std::get<1>(data);
                    const bool topFrameOnly = std::get<2>(data);
                    object->AddUserScriptImpl(scriptContent.c_str(), topFrameOnly);
                    return G_SOURCE_REMOVE;
                },
                data,
                [](gpointer customdata) {
                    delete static_cast<AddUserScriptData*>(customdata);
                });
            return Core::ERROR_NONE;
        }

        uint32_t RemoveAllUserScripts() override
        {
            if (_context == nullptr)
                return Core::ERROR_GENERAL;

            g_main_context_invoke_full(
                _context,
                G_PRIORITY_DEFAULT,
                [](gpointer customdata) -> gboolean {
                    WebKitImplementation* object = static_cast<WebKitImplementation*>(customdata);
#ifdef WEBKIT_GLIB_API
                    auto* userContentManager = webkit_web_view_get_user_content_manager(object->_view);
                    webkit_user_content_manager_remove_all_scripts(userContentManager);
#else
                    WKUserContentControllerRemoveAllUserScripts(object->_userContentController);
#endif
                    return G_SOURCE_REMOVE;
                },
                this,
                nullptr);
            return Core::ERROR_NONE;
        }

#if defined(ENABLE_CLOUD_COOKIE_JAR)
        uint32_t CookieJar(uint32_t& version /* @out */, uint32_t& checksum /* @out */, string& payload /* @out */) const override
        {
            uint32_t result = Core::ERROR_GENERAL;

            if (_context == nullptr)
                return result;

            _adminLock.Lock();
            if (_cookieJar.IsStale()) {
                _adminLock.Unlock();

                g_main_context_invoke(
                    _context,
                    [](gpointer customdata) -> gboolean {
                        WebKitImplementation& object =
                            *static_cast<WebKitImplementation*>(customdata);
                        object.RefreshCookieJar();
                        return G_SOURCE_REMOVE;
                    },
                    const_cast<WebKitImplementation*>(this));

                if (!_cookieJar.WaitForRefresh(1000))
                    return Core::ERROR_TIMEDOUT;

                _adminLock.Lock();
            }
            result = _cookieJar.Pack(version, checksum, payload);
            _adminLock.Unlock();

            return result;
        }

        uint32_t CookieJar(const uint32_t version, const uint32_t checksum, const string& payload) override
        {
            uint32_t result = Core::ERROR_GENERAL;
            if (_context == nullptr)
                return result;

            _adminLock.Lock();
            result = _cookieJar.Unpack(version, checksum, payload);
            _adminLock.Unlock();

            if (result == Core::ERROR_NONE)
            {
                g_main_context_invoke(
                    _context,
                    [](gpointer customdata) -> gboolean {
                        auto& object = *static_cast<WebKitImplementation*>(customdata);

                        std::vector<std::string> cookies;
                        object._adminLock.Lock();
                        cookies = object._cookieJar.GetCookies();
                        object._adminLock.Unlock();

                        object.SetCookies(cookies);
                        return G_SOURCE_REMOVE;
                    },
                    this);
            }

            return result;
        }

        void Register(Exchange::IBrowserCookieJar::INotification* sink) override
        {
            _adminLock.Lock();

            // Make sure a sink is not registered multiple times.
            ASSERT(std::find(_cookieJarClients.begin(), _cookieJarClients.end(), sink) == _cookieJarClients.end());

            _cookieJarClients.push_back(sink);
            sink->AddRef();

            TRACE(Trace::Information, (_T("Registered cookie jar notification client %p"), sink));

            _adminLock.Unlock();
        }

        void Unregister(Exchange::IBrowserCookieJar::INotification* sink) override
        {
            _adminLock.Lock();

            auto index(std::find(_cookieJarClients.begin(), _cookieJarClients.end(), sink));

            // Make sure you do not unregister something you did not register !!!
            ASSERT(index != _cookieJarClients.end());

            if (index != _cookieJarClients.end()) {
                (*index)->Release();
                _cookieJarClients.erase(index);
                TRACE(Trace::Information, (_T("Unregistered cookie jar notification client %p"), sink));
            }

            _adminLock.Unlock();
        }

        void NotifyCookieJarChanged()
        {
            _adminLock.Lock();

            _cookieJar.MarkAsStale();

            auto index(_cookieJarClients.begin());
            while (index != _cookieJarClients.end()) {
                (*index)->CookieJarChanged();
                index++;
            }

            _adminLock.Unlock();
        }

        void RefreshCookieJar()
        {
            #ifdef WEBKIT_GLIB_API
            WebKitWebContext* context = webkit_web_view_get_context(_view);
            WebKitCookieManager* manager = webkit_web_context_get_cookie_manager(context);
            webkit_cookie_manager_get_cookie_jar(manager, NULL, [](GObject* object, GAsyncResult* result, gpointer user_data) {
                GList* cookies_list = webkit_cookie_manager_get_cookie_jar_finish(WEBKIT_COOKIE_MANAGER(object), result, nullptr);

                std::vector<std::string> cookieVector;
                cookieVector.reserve(g_list_length(cookies_list));
                for (GList* it = cookies_list; it != NULL; it = g_list_next(it)) {
                    SoupCookie* soupCookie = (SoupCookie*)it->data;
                    gchar *cookieHeader = soup_cookie_to_set_cookie_header(soupCookie);
                    cookieVector.push_back(cookieHeader);
                    g_free(cookieHeader);
                }

                WebKitImplementation& browser = *static_cast< WebKitImplementation*>(user_data);
                browser._adminLock.Lock();
                browser._cookieJar.SetCookies(std::move(cookieVector));
                browser._adminLock.Unlock();
            }, this);
            #else
            static const auto toSoupCookie = [](WKCookieRef cookie) -> SoupCookie*
            {
                auto name   = WKCookieGetName(cookie);
                auto value  = WKCookieGetValue(cookie);
                auto domain = WKCookieGetDomain(cookie);
                auto path   = WKCookieGetPath(cookie);
                SoupCookie* soupCookie = soup_cookie_new(
                    WKStringToString(name).c_str(),
                    WKStringToString(value).c_str(),
                    WKStringToString(domain).c_str(),
                    WKStringToString(path).c_str(),
                    -1);
                SoupDate* expires = soup_date_new_from_time_t(WKCookieGetExpires(cookie) / 1000.0);
                soup_cookie_set_expires(soupCookie, expires);
                soup_date_free(expires);
                soup_cookie_set_http_only(soupCookie, WKCookieGetHttpOnly(cookie));
                soup_cookie_set_secure(soupCookie, WKCookieGetSecure(cookie));
                WKRelease(path);
                WKRelease(domain);
                WKRelease(value);
                WKRelease(name);
                return soupCookie;
            };

            WKCookieManagerGetCookies(
                _cookieManager, this, [](WKArrayRef cookies, WKErrorRef error, void* clientInfo) {
                WebKitImplementation& browser = *static_cast< WebKitImplementation*>(clientInfo);
                if (error) {
                    auto errorDomain = WKErrorCopyDomain(error);
                    auto errorDescription = WKErrorCopyLocalizedDescription(error);
                    TRACE_GLOBAL(Trace::Error,
                                 (_T("GetCookies failed, error(code=%d, domain=%s, message=%s)"),
                                     WKErrorGetErrorCode(error),
                                     WKStringToString(errorDomain).c_str(),
                                     WKStringToString(errorDescription).c_str()));
                    WKRelease(errorDescription);
                    WKRelease(errorDomain);
                    return;
                }
                std::vector<std::string> cookieVector;
                size_t size = cookies ? WKArrayGetSize(cookies) : 0;
                if (size > 0)
                {
                    cookieVector.reserve(size);
                    for (size_t i = 0; i < size; ++i)
                    {
                        WKCookieRef cookie = static_cast<WKCookieRef>(WKArrayGetItemAtIndex(cookies, i));
                        if (WKCookieGetSession(cookie))
                            continue;
                        SoupCookie* soupCookie = toSoupCookie(cookie);
                        gchar *cookieHeader = soup_cookie_to_set_cookie_header(soupCookie);
                        cookieVector.push_back(cookieHeader);
                        soup_cookie_free(soupCookie);
                        g_free(cookieHeader);
                    }
                    cookieVector.shrink_to_fit();
                }
                browser._adminLock.Lock();
                browser._cookieJar.SetCookies(std::move(cookieVector));
                browser._adminLock.Unlock();
            });
            #endif
        }

        void SetCookies(const std::vector<std::string>& cookies)
        {
            #ifdef WEBKIT_GLIB_API
            GList* cookies_list = nullptr;
            for (auto& cookie : cookies) {
                SoupCookie* sc = soup_cookie_parse(cookie.c_str(), nullptr);
                if (!sc)
                    continue;
                const char* domain = soup_cookie_get_domain(sc);
                if (!domain)
                    continue;

                // soup_cookie_parse() may prepend '.' to the domain,
                // check the original cookie string and remove '.' if needed
                if (domain[0] == '.')
                {
                    const char kDomainNeedle[] = "domain=";
                    const size_t kDomainNeedleLength = sizeof(kDomainNeedle) - 1;
                    auto it = std::search(
                        cookie.begin(), cookie.end(), kDomainNeedle, kDomainNeedle + kDomainNeedleLength,
                        [](const char c1, const char c2) {
                            return ::tolower(c1) == c2;
                        });
                    if (it != cookie.end())
                        it += kDomainNeedleLength;
                    if (it != cookie.end() && *it != '.' && *it != ';')
                    {
                        char* adjustedDomain = g_strdup(domain + 1);
                        soup_cookie_set_domain(sc, adjustedDomain);
                    }
                }
                cookies_list = g_list_prepend(cookies_list, sc);
            }

            WebKitWebContext* context = webkit_web_view_get_context(_view);
            WebKitCookieManager* manager = webkit_web_context_get_cookie_manager(context);
            webkit_cookie_manager_set_cookie_jar(manager, g_list_reverse(cookies_list), nullptr, nullptr, nullptr);

            g_list_free_full(cookies_list, reinterpret_cast<GDestroyNotify>(soup_cookie_free));
            #else
            auto toWKCookie = [](SoupCookie* cookie) -> WKCookieRef
            {
                SoupDate* expires = soup_cookie_get_expires(cookie);
                auto name   = WKStringCreateWithUTF8CString(soup_cookie_get_name(cookie));
                auto value  = WKStringCreateWithUTF8CString(soup_cookie_get_value(cookie));
                auto domain = WKStringCreateWithUTF8CString(soup_cookie_get_domain(cookie));
                auto path   = WKStringCreateWithUTF8CString(soup_cookie_get_path(cookie));
                WKCookieRef cookieRef =
                    WKCookieCreate(
                        name,
                        value,
                        domain,
                        path,
                        expires ? static_cast<double>(soup_date_to_time_t(expires)) * 1000 : 0,
                        soup_cookie_get_http_only(cookie),
                        soup_cookie_get_secure(cookie),
                        !expires);
                WKRelease(name);
                WKRelease(value);
                WKRelease(domain);
                WKRelease(path);
                return cookieRef;
            };
            size_t idx = 0;
            auto cookiesArray = std::unique_ptr<WKTypeRef[]>(new WKTypeRef[cookies.size()]);
            for (const auto& cookie : cookies)
            {
                std::unique_ptr<SoupCookie, void(*)(SoupCookie*)> sc(soup_cookie_parse(cookie.c_str(), nullptr), soup_cookie_free);
                if (!sc)
                    continue;
                const char* domain = soup_cookie_get_domain(sc.get());
                if (!domain)
                    continue;
                // soup_cookie_parse() may prepend '.' to the domain,
                // check the original cookie string and remove '.' if needed
                if (domain[0] == '.')
                {
                    const char kDomainNeedle[] = "domain=";
                    const size_t kDomainNeedleLength = sizeof(kDomainNeedle) - 1;
                    auto it = std::search(
                        cookie.begin(), cookie.end(), kDomainNeedle, kDomainNeedle + kDomainNeedleLength,
                        [](const char c1, const char c2) {
                            return ::tolower(c1) == c2;
                        });
                    if (it != cookie.end())
                        it += kDomainNeedleLength;
                    if (it != cookie.end() && *it != '.' && *it != ';')
                    {
                        char* adjustedDomain = g_strdup(domain + 1);
                        soup_cookie_set_domain(sc.get(), adjustedDomain);
                    }
                }
                cookiesArray[idx++] = toWKCookie(sc.get());
            }
            auto cookieArray = WKArrayCreateAdoptingValues(cookiesArray.get(), idx);
            WKCookieManagerSetCookies(_cookieManager, cookieArray);
            WKRelease(cookieArray);
            #endif
        }
#endif

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

        uint32_t URL(const string& URL) override
        {
            TRACE(Trace::Information, (_T("New URL: %s"), URL.c_str()));

            if (_context != nullptr) {
                using SetURLData = std::tuple<WebKitImplementation*, string>;
                auto *data = new SetURLData(this, URL);
                g_main_context_invoke_full(
                    _context,
                    G_PRIORITY_DEFAULT,
                    [](gpointer customdata) -> gboolean {
                        auto& data = *static_cast<SetURLData*>(customdata);
                        WebKitImplementation* object = std::get<0>(data);

                        string url = std::get<1>(data);
                        object->_adminLock.Lock();
                        object->_URL = url;
                        object->_adminLock.Unlock();

                        object->SetResponseHTTPStatusCode(-1);
#ifdef WEBKIT_GLIB_API
                        webkit_web_view_load_uri(object->_view, object->_URL.c_str());
#else
                        object->SetNavigationRef(nullptr);
                        auto shellURL = WKURLCreateWithUTF8CString(object->_URL.c_str());
                        WKPageLoadURL(object->_page, shellURL);
                        WKRelease(shellURL);
#endif
                        return G_SOURCE_REMOVE;
                    },
                    data,
                    [](gpointer customdata) {
                        delete static_cast<SetURLData*>(customdata);
                    });
            }

            return Core::ERROR_NONE;
        }

        uint32_t URL(string& url) const override
        {
            _adminLock.Lock();
            url = _URL;
            _adminLock.Unlock();

            return 0;
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

        uint32_t Reset(VARIABLE_IS_NOT_USED const resettype type) override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t Identifier(string& id) const override
        {

            uint32_t status = Core::ERROR_UNAVAILABLE;
            PluginHost::ISubSystem* subSystem = _service->SubSystems();
            if (subSystem) {
                const PluginHost::ISubSystem::IIdentifier* identifier(subSystem->Get<PluginHost::ISubSystem::IIdentifier>());
                if (identifier != nullptr) {
                    uint8_t buffer[64];

                    buffer[0] = static_cast<const PluginHost::ISubSystem::IIdentifier*>(identifier)
                                ->Identifier(sizeof(buffer) - 1, &(buffer[1]));

                    if (buffer[0] != 0) {
                        id = Core::SystemInfo::Instance().Id(buffer, ~0);
                    }

                    identifier->Release();
                }

                subSystem->Release();
            }

            return Core::ERROR_NONE;
        }

        uint32_t ContentLink(VARIABLE_IS_NOT_USED const string& link) override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t LaunchPoint(VARIABLE_IS_NOT_USED launchpointtype& point) const override
        {
            return Core::ERROR_UNAVAILABLE;
        }

        uint32_t LaunchPoint(VARIABLE_IS_NOT_USED const launchpointtype&) override
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
                        if (!it.IsValid()) {
                            continue;
                        }
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

        void OnURLChanged(const string& URL)
        {
            _adminLock.Lock();

            _URL = URL;

            std::list<Exchange::IWebBrowser::INotification*>::iterator index(_notificationClients.begin());
            {
                while (index != _notificationClients.end()) {
                    (*index)->URLChange(URL, false);
                    index++;
                }
            }
            {
                std::list<Exchange::IBrowser::INotification*>::iterator index(_notificationBrowserClients.begin());
                while (index != _notificationBrowserClients.end()) {
                    (*index)->URLChanged(URL);
                    index++;
                }
            }

            _adminLock.Unlock();
        }

#ifndef WEBKIT_GLIB_API
        void OnLoadFinished(const string& URL, WKNavigationRef navigation)
        {
            if (_navigationRef != navigation) {
                TRACE(Trace::Information, (_T("Ignore 'loadfinished' for previous navigation request")));
                return;
            }
            OnLoadFinished(URL);
        }
#endif
        void OnLoadFinished(const string& URL)
        {
            _adminLock.Lock();

            _URL = URL;
            {
                std::list<Exchange::IWebBrowser::INotification*>::iterator index(_notificationClients.begin());

                while (index != _notificationClients.end()) {
                    (*index)->LoadFinished(URL, _httpStatusCode);
                    index++;
                }
            }
            {
                std::list<Exchange::IBrowser::INotification*>::iterator index(_notificationBrowserClients.begin());

                while (index != _notificationBrowserClients.end()) {
                    (*index)->LoadFinished(URL);
                    index++;
                }
            }

            _adminLock.Unlock();
        }
        void OnLoadFailed(const string& URL)
        {
            _adminLock.Lock();

            std::list<Exchange::IWebBrowser::INotification*>::iterator index(_notificationClients.begin());

            while (index != _notificationClients.end()) {
                (*index)->LoadFailed(URL);
                index++;
            }

            _adminLock.Unlock();
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

        uint32_t Configure(PluginHost::IShell* service) override
        {
            #ifndef WEBKIT_GLIB_API
            _consoleLogPrefix = service->Callsign();
            #endif
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

            #if defined(ENABLE_LOGGING_UTILS)
            if (!_config.LoggingTarget.Value().empty()) {
                if (!RedirectAllLogsToService(_config.LoggingTarget.Value())) {
                    SYSLOG(Logging::Error, (_T("Could not redirect logs to %s"), _config.LoggingTarget.Value().c_str()));
                }
            }
            #endif

            bool environmentOverride(WebKitBrowser::EnvironmentOverride(_config.EnvironmentOverride.Value()));

            if ((environmentOverride == false) || (Core::SystemInfo::GetEnvironment(_T("WPE_WEBKIT_URL"), _URL) == false)) {
                _URL = _config.URL.Value();
            }

            Core::SystemInfo::SetEnvironment(_T("QUEUEPLAYER_FLUSH_MODE"), _T("3"), false);
            Core::SystemInfo::SetEnvironment(_T("HOME"), service->PersistentPath());

            if (_config.ClientIdentifier.IsSet() == true) {
                string value(service->Callsign() + ',' + _config.ClientIdentifier.Value());
                Core::SystemInfo::SetEnvironment(_T("CLIENT_IDENTIFIER"), value, !environmentOverride);
            } else {
                Core::SystemInfo::SetEnvironment(_T("CLIENT_IDENTIFIER"), service->Callsign(), !environmentOverride);
            }

            // Set dummy window for gst-gl
            Core::SystemInfo::SetEnvironment(_T("GST_GL_WINDOW"), _T("dummy"), !environmentOverride);

            // MSE Buffers
            if (_config.MSEBuffers.Value().empty() == false) {
                Core::SystemInfo::SetEnvironment(_T("MSE_MAX_BUFFER_SIZE"), _config.MSEBuffers.Value(), !environmentOverride);
            }

            // Memory Pressure
#if !HAS_MEMORY_PRESSURE_SETTINGS_API
            std::stringstream limitStr;
            if ((_config.Memory.IsSet() == true) && (_config.Memory.NetworkProcessSettings.Limit.IsSet() == true)) {
                limitStr << "networkprocess:" << _config.Memory.NetworkProcessSettings.Limit.Value() << "m";
            }
            if ((_config.Memory.IsSet() == true) && (_config.Memory.WebProcessSettings.Limit.IsSet() == true)) {
                limitStr << (!limitStr.str().empty() ? "," : "") << "webprocess:" << _config.Memory.WebProcessSettings.Limit.Value() << "m";
            }
            if (!limitStr.str().empty()) {
                Core::SystemInfo::SetEnvironment(_T("WPE_POLL_MAX_MEMORY"), limitStr.str(), !environmentOverride);
            }
#endif

            // Memory Profile
            if (_config.MemoryProfile.Value().empty() == false) {
                Core::SystemInfo::SetEnvironment(_T("WPE_RAM_SIZE"), _config.MemoryProfile.Value(), !environmentOverride);
            }

            // GStreamer on-disk buffering
            if (_config.MediaDiskCache.Value() == false) {
                Core::SystemInfo::SetEnvironment(_T("WPE_SHELL_DISABLE_MEDIA_DISK_CACHE"), _T("1"), !environmentOverride);
            } else {
                Core::SystemInfo::SetEnvironment(_T("WPE_SHELL_MEDIA_DISK_CACHE_PATH"), service->PersistentPath(), !environmentOverride); }

            // Disk Cache
            if (_config.DiskCache.Value().empty() == false) {
                Core::SystemInfo::SetEnvironment(_T("WPE_DISK_CACHE_SIZE"), _config.DiskCache.Value(), !environmentOverride);
            }

            // Disk Cache Dir
            if (_config.DiskCacheDir.Value().empty() == false) {
               Core::SystemInfo::SetEnvironment(_T("XDG_CACHE_HOME"), _config.DiskCacheDir.Value(), !environmentOverride);
            }

            if (_config.XHRCache.Value() == false) {
                Core::SystemInfo::SetEnvironment(_T("WPE_DISABLE_XHR_RESPONSE_CACHING"), _T("1"), !environmentOverride);
            }

            // Enable cookie persistent storage
            if (_config.CookieStorage.Value().empty() == false) {
                Core::SystemInfo::SetEnvironment(_T("WPE_SHELL_COOKIE_STORAGE"), _T("1"), !environmentOverride);
            }

            // Use cairo noaa compositor
            if (_config.Compositor.Value().empty() == false) {
                Core::SystemInfo::SetEnvironment(_T("CAIRO_GL_COMPOSITOR"), _config.Compositor.Value(), !environmentOverride);
            }

            // WebInspector
            if (_config.Inspector.Value().empty() == false) {
#ifdef WEBKIT_GLIB_API
                if (_config.InspectorNative.Value()) {
                    Core::SystemInfo::SetEnvironment(_T("WEBKIT_INSPECTOR_SERVER"), _config.Inspector.Value(), !environmentOverride);
                } else {
                    Core::SystemInfo::SetEnvironment(_T("WEBKIT_INSPECTOR_HTTP_SERVER"), _config.Inspector.Value(), !environmentOverride);
                }
#else
                if (_config.Automation.Value()) {
                    Core::SystemInfo::SetEnvironment(_T("WEBKIT_INSPECTOR_SERVER"), _config.Inspector.Value(), !environmentOverride);
                } else {
                    Core::SystemInfo::SetEnvironment(_T("WEBKIT_LEGACY_INSPECTOR_SERVER"), _config.Inspector.Value(), !environmentOverride);
                }
#endif
            }

            // RPI mouse support
            if (_config.Cursor.Value() == true) {
                Core::SystemInfo::SetEnvironment(_T("WPE_BCMRPI_CURSOR"), _T("1"), !environmentOverride);
            }

            // RPI touch support
            if (_config.Touch.Value() == true) {
                Core::SystemInfo::SetEnvironment(_T("WPE_BCMRPI_TOUCH"), _T("1"), !environmentOverride);
            }

            // Rank Thunder Decryptor higher than ClearKey one
            if (_config.ThunderDecryptorPreference.Value() == true) {
                Core::SystemInfo::SetEnvironment(_T("WEBKIT_GST_EME_RANK_PRIORITY"), _T("Thunder"), !environmentOverride);
            }

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

            for (auto environmentVariableIndex = 0; environmentVariableIndex < _config.EnvironmentVariables.Length(); environmentVariableIndex++) {
                const auto& environmentVariable = _config.EnvironmentVariables[environmentVariableIndex];
                Core::SystemInfo::SetEnvironment(environmentVariable.Name.Value(), environmentVariable.Value.Value());
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
        void SetNavigationRef(WKNavigationRef ref)
        {
            _navigationRef = ref;
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
            return _page;
        }

        string GetConsoleLogPrefix() const
        {
            return _consoleLogPrefix;
        }
#endif
        BEGIN_INTERFACE_MAP(WebKitImplementation)
        INTERFACE_ENTRY(Exchange::IWebBrowser)
        INTERFACE_ENTRY(Exchange::IBrowser)
        INTERFACE_ENTRY (Exchange::IApplication)
        INTERFACE_ENTRY (Exchange::IBrowserScripting)
#if defined(ENABLE_CLOUD_COOKIE_JAR)
        INTERFACE_ENTRY (Exchange::IBrowserCookieJar)
#endif
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
                        if (object->_config.LoadBlankPageOnSuspendEnabled.Value()) {
                            const char kBlankURL[] = "about:blank";
                            if (object->_URL != kBlankURL)
                                object->SetURL(kBlankURL);
                            ASSERT(object->_URL == kBlankURL);
                        }
#ifdef WEBKIT_GLIB_API
                        webkit_web_view_suspend(object->_view);
#else
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
#ifdef WEBKIT_GLIB_API
        static void initializeWebExtensionsCallback(WebKitWebContext* context, WebKitImplementation* browser)
        {
            webkit_web_context_set_web_extensions_directory(context, browser->_extensionPath.c_str());
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
        static gboolean decidePolicyCallback(WebKitWebView*, WebKitPolicyDecision* decision, WebKitPolicyDecisionType type, WebKitImplementation* browser)
        {
            if (type == WEBKIT_POLICY_DECISION_TYPE_RESPONSE) {
                auto *response = webkit_response_policy_decision_get_response(WEBKIT_RESPONSE_POLICY_DECISION(decision));
                if (webkit_uri_response_is_main_frame(response))
                    browser->SetResponseHTTPStatusCode(webkit_uri_response_get_status_code(response));
            }
            webkit_policy_decision_use(decision);
            return TRUE;
        }
        static void uriChangedCallback(WebKitWebView* webView, GParamSpec*, WebKitImplementation* browser)
        {
            browser->OnURLChanged(Core::ToString(webkit_web_view_get_uri(webView)));
        }
        static void loadChangedCallback(WebKitWebView* webView, WebKitLoadEvent loadEvent, WebKitImplementation* browser)
        {
            if (loadEvent == WEBKIT_LOAD_FINISHED) {
                if (browser->_ignoreLoadFinishedOnce) {
                    browser->_ignoreLoadFinishedOnce = false;
                    return;
                }
                const std::string uri = webkit_web_view_get_uri(webView);
                if (browser->_httpStatusCode < 0 && uri.find("http", 0, 4) != std::string::npos &&
                    webkit_web_view_get_estimated_load_progress(webView) < 1.0)
                {
                    // Load failed or there is another load in progress already
                    return;
                }
                browser->OnLoadFinished(Core::ToString(uri.c_str()));
            }
        }
        static void loadFailedCallback(WebKitWebView*, WebKitLoadEvent loadEvent, const gchar* failingURI, GError* error, WebKitImplementation* browser)
        {
            string message(string("{ \"url\": \"") + failingURI + string("\", \"Error message\": \"") + error->message + string("\", \"loadEvent\":") + Core::NumberType<uint32_t>(loadEvent).Text() + string(" }"));
            SYSLOG(Trace::Information, (_T("LoadFailed: %s"), message.c_str()));
            if (g_error_matches(error, WEBKIT_NETWORK_ERROR, WEBKIT_NETWORK_ERROR_CANCELLED)) {
                browser->_ignoreLoadFinishedOnce = true;
                return;
            }
            browser->OnLoadFailed(failingURI);
        }
        static void webProcessTerminatedCallback(VARIABLE_IS_NOT_USED WebKitWebView* webView, WebKitWebProcessTerminationReason reason, WebKitImplementation* browser)
        {
            switch (reason) {
            case WEBKIT_WEB_PROCESS_CRASHED:
                SYSLOG(Trace::Fatal, (_T("CRASH: WebProcess crashed: exiting ...")));
                break;
            case WEBKIT_WEB_PROCESS_EXCEEDED_MEMORY_LIMIT:
                SYSLOG(Trace::Fatal, (_T("CRASH: WebProcess terminated due to memory limit: exiting ...")));
                break;
            case WEBKIT_WEB_PROCESS_TERMINATED_BY_API:
                SYSLOG(Trace::Fatal, (_T("CRASH: WebProcess terminated by API")));
                break;
            }
            g_signal_handlers_block_matched(webView, G_SIGNAL_MATCH_DATA, 0, 0, nullptr, nullptr, browser);
            struct ExitJob : public Core::IDispatch
            {
                virtual void Dispatch() { exit(1); }
            };
            Core::IWorkerPool::Instance().Submit(Core::proxy_cast<Core::IDispatch>(Core::ProxyType<ExitJob>::Create()));
        }
        static void closeCallback(VARIABLE_IS_NOT_USED WebKitWebView* webView, WebKitImplementation* browser)
        {
            browser->NotifyClosure();
        }
        static gboolean decidePermissionCallback(VARIABLE_IS_NOT_USED WebKitWebView* webView, WebKitPermissionRequest* permissionRequest)
        {
            webkit_permission_request_allow(permissionRequest);
            return TRUE;
        }
        static gboolean showNotificationCallback(VARIABLE_IS_NOT_USED WebKitWebView* webView, WebKitNotification* notification, VARIABLE_IS_NOT_USED WebKitImplementation* browser)
        {
            TRACE_GLOBAL(HTML5Notification, (_T("%s - %s"), webkit_notification_get_title(notification), webkit_notification_get_body(notification)));
            return FALSE;
        }
        static WebKitWebView* createWebViewForAutomationCallback(VARIABLE_IS_NOT_USED WebKitAutomationSession* session, WebKitImplementation* browser)
        {
            return browser->_view;
        }
        static void automationStartedCallback(VARIABLE_IS_NOT_USED WebKitWebContext* context, WebKitAutomationSession* session, WebKitImplementation* browser)
        {
            WebKitApplicationInfo *info = webkit_application_info_new();
            webkit_application_info_set_name(info, "WPEWebKitBrowser");
            webkit_application_info_set_version(info, 1, 0, 0);
            webkit_automation_session_set_application_info(session, info);
            webkit_application_info_unref(info);

            g_signal_connect(session, "create-web-view", reinterpret_cast<GCallback>(createWebViewForAutomationCallback), browser);
        }
        static gboolean userMessageReceivedCallback(WebKitWebView*, WebKitUserMessage* message, WebKitImplementation* browser)
        {
            const char* name = webkit_user_message_get_name(message);
            if (g_strcmp0(name, Tags::BridgeObjectQuery) == 0) {
                GVariant* payload;
                const char* payloadPtr;

                payload = webkit_user_message_get_parameters(message);
                if (!payload) {
                    return false;
                }
                g_variant_get(payload, "&s", &payloadPtr);
                string payloadStr(payloadPtr);
                browser->OnBridgeQuery(payloadStr);
            }
            return true;
        }
#if defined(ENABLE_CLOUD_COOKIE_JAR)
        static void cookieManagerChangedCallback(WebKitCookieManager* manager, WebKitImplementation* browser) {
            browser->NotifyCookieJarChanged();
        }
#endif
        uint32_t Worker() override
        {
            _context = g_main_context_new();
            _loop = g_main_loop_new(_context, FALSE);
            g_main_context_push_thread_default(_context);

            HangDetector hangdetector(*this);

            bool automationEnabled = _config.Automation.Value();

            WebKitWebContext* wkContext;
            if (automationEnabled) {
                wkContext = webkit_web_context_new_ephemeral();
                webkit_web_context_set_automation_allowed(wkContext, TRUE);
                g_signal_connect(wkContext, "automation-started", reinterpret_cast<GCallback>(automationStartedCallback), this);
            } else {
                gchar* wpeStoragePath;
                if (_config.LocalStorage.IsSet() == true && _config.LocalStorage.Value().empty() == false) {
#ifdef USE_EXACT_PATHS
                    wpeStoragePath = g_build_filename(_config.LocalStorage.Value().c_str(), nullptr);
#else
                    wpeStoragePath = g_build_filename(_config.LocalStorage.Value().c_str(), "wpe", "local-storage", nullptr);
#endif
                } else {
                    wpeStoragePath = g_build_filename(g_get_user_cache_dir(), "wpe", "local-storage", nullptr);
                }
                g_mkdir_with_parents(wpeStoragePath, 0700);

                // Default value suggested by HTML5 spec
                uint32_t localStorageDatabaseQuotaInBytes = 5 * 1024 * 1024;
                if (_config.LocalStorageSize.IsSet() == true && _config.LocalStorageSize.Value() != 0) {
                    localStorageDatabaseQuotaInBytes = _config.LocalStorageSize.Value() * 1024;
                    TRACE(Trace::Information, (_T("Configured LocalStorage Quota  %u bytes"), localStorageDatabaseQuotaInBytes));
                }

                gchar* wpeDiskCachePath;
                if (_config.DiskCacheDir.IsSet() == true && _config.DiskCacheDir.Value().empty() == false) {
#ifdef USE_EXACT_PATHS
                    wpeDiskCachePath = g_build_filename(_config.DiskCacheDir.Value().c_str(), nullptr);
#else
                    wpeDiskCachePath = g_build_filename(_config.DiskCacheDir.Value().c_str(), "wpe", "disk-cache", nullptr);
#endif
                } else {
                    wpeDiskCachePath = g_build_filename(g_get_user_cache_dir(), "wpe", "disk-cache", nullptr);
                }
                g_mkdir_with_parents(wpeDiskCachePath, 0700);

                gchar* indexedDBPath = nullptr;
                if (_config.IndexedDBPath.IsSet() && !_config.IndexedDBPath.Value().empty()) {
                    indexedDBPath = g_build_filename(_config.IndexedDBPath.Value().c_str(), "wpe", "databases", "indexeddb", nullptr);
                } else {
                    indexedDBPath = g_build_filename(g_get_user_cache_dir(), "wpe", "databases", "indexeddb", nullptr);
                }
                g_mkdir_with_parents(indexedDBPath, 0700);

                uint64_t indexedDBSizeBytes = 0;    // No limit by default, use WebKit defaults (1G at the moment of writing)
                if (_config.IndexedDBSize.IsSet() && _config.IndexedDBSize.Value() != 0) {
                    indexedDBSizeBytes = _config.IndexedDBSize.Value() * 1024;
                }

#if HAS_MEMORY_PRESSURE_SETTINGS_API
                if ((_config.Memory.IsSet() == true) && (_config.Memory.NetworkProcessSettings.IsSet() == true)) {
                    WebKitMemoryPressureSettings* memoryPressureSettings = webkit_memory_pressure_settings_new();
                    if (_config.Memory.NetworkProcessSettings.Limit.IsSet() == true) {
                        webkit_memory_pressure_settings_set_memory_limit(memoryPressureSettings, _config.Memory.NetworkProcessSettings.Limit.Value());
                    }
                    if (_config.Memory.NetworkProcessSettings.PollInterval.IsSet() == true) {
                        webkit_memory_pressure_settings_set_poll_interval(memoryPressureSettings, _config.Memory.NetworkProcessSettings.PollInterval.Value());
                    }
                    webkit_website_data_manager_set_memory_pressure_settings(memoryPressureSettings);
                    webkit_memory_pressure_settings_free(memoryPressureSettings);
                }
#endif
                auto* websiteDataManager = webkit_website_data_manager_new(
                    "local-storage-directory", wpeStoragePath,
                    "disk-cache-directory", wpeDiskCachePath,
                    "local-storage-quota", localStorageDatabaseQuotaInBytes,
                    "indexeddb-directory", indexedDBPath,
                    "per-origin-storage-quota", indexedDBSizeBytes,
                     nullptr);
                g_free(wpeStoragePath);
                g_free(wpeDiskCachePath);
                g_free(indexedDBPath);

#if HAS_MEMORY_PRESSURE_SETTINGS_API
                if ((_config.Memory.IsSet() == true) && (_config.Memory.WebProcessSettings.IsSet() == true)) {
                    WebKitMemoryPressureSettings* memoryPressureSettings = webkit_memory_pressure_settings_new();
                    if (_config.Memory.WebProcessSettings.Limit.IsSet() == true) {
                        webkit_memory_pressure_settings_set_memory_limit(memoryPressureSettings, _config.Memory.WebProcessSettings.Limit.Value());
                    }
                    if (_config.Memory.WebProcessSettings.GPUFile.IsSet() == true) {
                        Core::SystemInfo::SetEnvironment(_T("WPE_POLL_MAX_MEMORY_GPU_FILE"), _config.Memory.WebProcessSettings.GPUFile.Value());
                    }
                    if (_config.Memory.WebProcessSettings.GPULimit.IsSet() == true) {
                        webkit_memory_pressure_settings_set_video_memory_limit(memoryPressureSettings, _config.Memory.WebProcessSettings.GPULimit.Value());
                    }
                    if (_config.Memory.WebProcessSettings.PollInterval.IsSet() == true) {
                        webkit_memory_pressure_settings_set_poll_interval(memoryPressureSettings, _config.Memory.WebProcessSettings.PollInterval.Value());
                    }

                    // Pass web process memory pressure settings to WebKitWebContext constructor
                    wkContext = WEBKIT_WEB_CONTEXT(g_object_new(WEBKIT_TYPE_WEB_CONTEXT, "website-data-manager", websiteDataManager, "memory-pressure-settings", memoryPressureSettings, nullptr));
                    webkit_memory_pressure_settings_free(memoryPressureSettings);
                } else
#endif
                {
                    wkContext = webkit_web_context_new_with_website_data_manager(websiteDataManager);
                }
                g_object_unref(websiteDataManager);
            }

            if (_dataPath.empty() == false) {
                _extensionPath = _dataPath + "/" + _config.ExtensionDir.Value();
                // Set up injected bundle. Will be loaded once WPEWebProcess is started.
                g_signal_connect(wkContext, "initialize-web-extensions", G_CALLBACK(initializeWebExtensionsCallback), this);
            }

            if (!webkit_web_context_is_ephemeral(wkContext)) {
                auto* cookieManager = webkit_web_context_get_cookie_manager(wkContext);
                #if defined(ENABLE_CLOUD_COOKIE_JAR)
                if (_config.CloudCookieJarEnabled.IsSet() && _config.CloudCookieJarEnabled.Value()) {
                    g_signal_connect(cookieManager, "changed", reinterpret_cast<GCallback>(cookieManagerChangedCallback), this);
                } else
                #endif
                {
                    gchar* cookieDatabasePath;
                    if (_config.CookieStorage.IsSet() == true && _config.CookieStorage.Value().empty() == false) {
                        cookieDatabasePath = g_build_filename(_config.CookieStorage.Value().c_str(), "cookies.db", nullptr);
                    } else {
                        cookieDatabasePath = g_build_filename(g_get_user_cache_dir(), "cookies.db", nullptr);
                    }

                    webkit_cookie_manager_set_persistent_storage(cookieManager, cookieDatabasePath, WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);
                    if (_config.CookieAcceptPolicy.IsSet()) {
                        HTTPCookieAcceptPolicy(_config.CookieAcceptPolicy.Value());
                    } else {
                        webkit_cookie_manager_set_accept_policy(cookieManager, _httpCookieAcceptPolicy);
                    }
                }
            }

            if (!_config.CertificateCheck) {
                webkit_web_context_set_tls_errors_policy(wkContext, WEBKIT_TLS_ERRORS_POLICY_IGNORE);
            }

            auto* languages = static_cast<char**>(g_new0(char*, _config.Languages.Length() + 1));
            Core::JSON::ArrayType<Core::JSON::String>::Iterator index(_config.Languages.Elements());

            for (unsigned i = 0; index.Next(); ++i) {
                languages[i] = g_strdup(index.Current().Value().c_str());
            }

            webkit_web_context_set_preferred_languages(wkContext, languages);
            g_strfreev(languages);

            auto* preferences = webkit_settings_new();

            webkit_settings_set_enable_encrypted_media(preferences, TRUE);
            webkit_settings_set_enable_mediasource(preferences, TRUE);
            webkit_settings_set_enable_media_stream(preferences, TRUE);
            webkit_settings_set_enable_page_cache(preferences, FALSE);
            webkit_settings_set_enable_directory_upload(preferences, FALSE);

#if WEBKIT_CHECK_VERSION(2, 38, 0)
            webkit_settings_set_enable_webrtc(preferences, TRUE);
#endif

            // Turn on/off WebGL
            webkit_settings_set_enable_webgl(preferences, _config.WebGLEnabled.Value());

            if (_config.AllowFileURLsCrossAccess.IsSet()) {
              // Turn on/off file URLs Cross Access
              webkit_settings_set_allow_file_access_from_file_urls(preferences, _config.AllowFileURLsCrossAccess.Value());
              webkit_settings_set_allow_universal_access_from_file_urls(preferences, _config.AllowFileURLsCrossAccess.Value());
            }

            if (_config.SpatialNavigation.IsSet()) {
              // Turn on/off spatial navigation
              webkit_settings_set_enable_spatial_navigation(preferences, _config.SpatialNavigation.Value());
              webkit_settings_set_enable_tabs_to_links(preferences, _config.SpatialNavigation.Value());
            }
            webkit_settings_set_allow_scripts_to_close_windows(preferences, _config.AllowWindowClose.Value());
            webkit_settings_set_enable_non_composited_webgl(preferences, _config.NonCompositedWebGLEnabled.Value());

            // Media Content Types Requiring Hardware Support
            if (_config.MediaContentTypesRequiringHardwareSupport.IsSet() == true
                && _config.MediaContentTypesRequiringHardwareSupport.Value().empty() == false) {
                webkit_settings_set_media_content_types_requiring_hardware_support(preferences,
                    _config.MediaContentTypesRequiringHardwareSupport.Value().c_str());
            }

            if (_config.UserAgent.IsSet() == true && _config.UserAgent.Value().empty() == false) {
                webkit_settings_set_user_agent(preferences, _config.UserAgent.Value().c_str());
            } else {
                webkit_settings_set_user_agent_with_application_details(preferences, "WPE", "1.0");
                _config.UserAgent = webkit_settings_get_user_agent(preferences);
            }

            webkit_settings_set_enable_html5_local_storage(preferences, _localStorageEnabled);
            webkit_settings_set_enable_html5_database(preferences, _config.IndexedDBEnabled.Value());

            // webaudio support
            webkit_settings_set_enable_webaudio(preferences, _config.WebAudioEnabled.Value());

            // Allow mixed content.
            bool enableWebSecurity = _config.Secure.Value();
#if WEBKIT_CHECK_VERSION(2, 38, 0)
            g_object_set(G_OBJECT(preferences),
                     "disable-web-security", !enableWebSecurity,
                     "allow-running-of-insecure-content", !enableWebSecurity,
                     "allow-display-of-insecure-content", !enableWebSecurity, nullptr);
#else
            g_object_set(G_OBJECT(preferences),
                     "enable-websecurity", enableWebSecurity,
                     "allow-running-of-insecure-content", !enableWebSecurity,
                     "allow-display-of-insecure-content", !enableWebSecurity, nullptr);
#endif
            _view = WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
                "backend", webkit_web_view_backend_new(wpe_view_backend_create(), nullptr, nullptr),
                "web-context", wkContext,
                "settings", preferences,
                "is-controlled-by-automation", automationEnabled,
                nullptr));
            g_object_unref(wkContext);
            g_object_unref(preferences);

            unsigned frameDisplayedCallbackID = 0;
            if (_config.FPS.Value() == true) {
                frameDisplayedCallbackID = webkit_web_view_add_frame_displayed_callback(_view, [](WebKitWebView*, gpointer userData) {
                    auto* browser = static_cast<WebKitImplementation*>(userData);
                    browser->SetFPS();
                }, this, nullptr);
            }

            auto* userContentManager = webkit_web_view_get_user_content_manager(_view);
            // webkit_user_content_manager_register_script_message_handler_in_world(userContentManager, "wpeNotifyWPEFramework", std::to_string(_guid).c_str());
            g_signal_connect(userContentManager, "script-message-received::wpeNotifyWPEFramework",
                reinterpret_cast<GCallback>(wpeNotifyWPEFrameworkMessageReceivedCallback), this);
            webkit_user_content_manager_register_script_message_handler(userContentManager, "wpeNotifyWPEFramework");

            SetupUserContentFilter();
            TryLoadingUserScripts();

            if (_config.Transparent.Value() == true) {
                WebKitColor transparent { 0, 0, 0, 0};
                webkit_web_view_set_background_color(_view, &transparent);
            }

            g_signal_connect(_view, "decide-policy", reinterpret_cast<GCallback>(decidePolicyCallback), this);
            g_signal_connect(_view, "notify::uri", reinterpret_cast<GCallback>(uriChangedCallback), this);
            g_signal_connect(_view, "load-changed", reinterpret_cast<GCallback>(loadChangedCallback), this);
            g_signal_connect(_view, "web-process-terminated", reinterpret_cast<GCallback>(webProcessTerminatedCallback), this);
            g_signal_connect(_view, "close", reinterpret_cast<GCallback>(closeCallback), this);
            g_signal_connect(_view, "permission-request", reinterpret_cast<GCallback>(decidePermissionCallback), nullptr);
            g_signal_connect(_view, "show-notification", reinterpret_cast<GCallback>(showNotificationCallback), this);
            g_signal_connect(_view, "user-message-received", reinterpret_cast<GCallback>(userMessageReceivedCallback), this);
            g_signal_connect(_view, "notify::is-web-process-responsive", reinterpret_cast<GCallback>(isWebProcessResponsiveCallback), this);
            g_signal_connect(_view, "load-failed", reinterpret_cast<GCallback>(loadFailedCallback), this);

            _configurationCompleted.SetState(true);

            URL(static_cast<const string>(_URL));

            // Move into the correct state, as requested
            auto* backend = webkit_web_view_backend_get_wpe_backend(webkit_web_view_get_backend(_view));
            _adminLock.Lock();
            if ((_state == PluginHost::IStateControl::SUSPENDED) || (_state == PluginHost::IStateControl::UNINITIALIZED)) {
                _state = PluginHost::IStateControl::UNINITIALIZED;
                wpe_view_backend_add_activity_state(backend, wpe_view_activity_state_visible | wpe_view_activity_state_focused);
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
            // webkit_user_content_manager_unregister_script_message_handler_in_world(userContentManager, "wpeNotifyWPEFramework", std::to_string(_guid).c_str());
            webkit_user_content_manager_unregister_script_message_handler(userContentManager, "wpeNotifyWPEFramework");

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
            if (_config.LocalStorage.IsSet() == true && _config.LocalStorage.Value().empty() == false) {
                wpeStoragePath = g_build_filename(_config.LocalStorage.Value().c_str(), "wpe", "local-storage", nullptr);
            } else {
                wpeStoragePath = g_build_filename(g_get_user_cache_dir(), "wpe", "local-storage", nullptr);
            }

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

            WKContextRef wkContext = WKContextCreateWithConfiguration(contextConfiguration);
            WKSoupSessionSetIgnoreTLSErrors(wkContext, !_config.CertificateCheck);

            if (_config.Languages.IsSet()) {
                WKMutableArrayRef languages = WKMutableArrayCreate();
                Core::JSON::ArrayType<Core::JSON::String>::Iterator index(_config.Languages.Elements());

                while (index.Next() == true) {
                    WKStringRef itemString = WKStringCreateWithUTF8CString(index.Current().Value().c_str());
                    WKArrayAppendItem(languages, itemString);
                    WKRelease(itemString);
                }

                WKSoupSessionSetPreferredLanguages(wkContext, languages);
                WKRelease(languages);
            }

            WKRelease(contextConfiguration);

            WKGeolocationManagerRef geolocationManager = WKContextGetGeolocationManager(wkContext);
            WKGeolocationManagerSetProvider(geolocationManager, &_handlerGeolocationProvider.base);

            _notificationManager = WKContextGetNotificationManager(wkContext);
            _handlerNotificationProvider.base.clientInfo = static_cast<void*>(this);
            WKNotificationManagerSetProvider(_notificationManager, &_handlerNotificationProvider.base);

            auto pageGroupIdentifier = WKStringCreateWithUTF8CString(_config.PageGroup.Value().c_str());
            auto pageGroup = WKPageGroupCreateWithIdentifier(pageGroupIdentifier);
            WKRelease(pageGroupIdentifier);

            auto preferences = WKPreferencesCreate();

            // Allow mixed content.
            bool allowMixedContent = _config.Secure.Value();
            WKPreferencesSetAllowRunningOfInsecureContent(preferences, !allowMixedContent);
            WKPreferencesSetAllowDisplayOfInsecureContent(preferences, !allowMixedContent);

            // WebSecurity
            WKPreferencesSetWebSecurityEnabled(preferences, allowMixedContent);

            // Turn off log message to stdout.
            WKPreferencesSetLogsPageMessagesToSystemConsoleEnabled(preferences, _config.LogToSystemConsoleEnabled.Value());

            // Turn on gamepads.
            WKPreferencesSetGamepadsEnabled(preferences, true);

            // Turn on fullscreen API.
            WKPreferencesSetFullScreenEnabled(preferences, true);

            // Turn off BackForwardList
            WKPreferencesSetPageCacheEnabled(preferences, FALSE);

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

            // webaudio support
            WKPreferencesSetWebAudioEnabled(preferences, _config.WebAudioEnabled.Value());

            WKPageGroupSetPreferences(pageGroup, preferences);

            _userContentController = WKUserContentControllerCreate();
            auto pageConfiguration = WKPageConfigurationCreate();
            WKPageConfigurationSetContext(pageConfiguration, wkContext);
            WKPageConfigurationSetPageGroup(pageConfiguration, pageGroup);
            WKPageConfigurationSetUserContentController(pageConfiguration, _userContentController);

            SetupUserContentFilter();
            TryLoadingUserScripts();

            _cookieManager = WKContextGetCookieManager(wkContext);
            #if defined(ENABLE_CLOUD_COOKIE_JAR)
            if (_config.CloudCookieJarEnabled.IsSet() && _config.CloudCookieJarEnabled.Value()) {
                WKCookieManagerClientV0 client = {
                    { 0, this },
                    // cookiesDidChange
                    [](WKCookieManagerRef, const void* clientInfo) {
                        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
                        browser->NotifyCookieJarChanged();
                    }
                };
                WKCookieManagerSetClient(_cookieManager, &client.base);
                WKCookieManagerStartObservingCookieChanges(_cookieManager);
            } else
            #endif
            {
                gchar* cookieDatabasePath;
                if (_config.CookieStorage.IsSet() == true && _config.CookieStorage.Value().empty() == false) {
                    cookieDatabasePath = g_build_filename(_config.CookieStorage.Value().c_str(), "cookies.db", nullptr);
                } else {
                    cookieDatabasePath = g_build_filename(g_get_user_cache_dir(), "cookies.db", nullptr);
                }
                auto path = WKStringCreateWithUTF8CString(cookieDatabasePath);
                g_free(cookieDatabasePath);
                WKCookieManagerSetCookiePersistentStorage(_cookieManager, path, kWKCookieStorageTypeSQLite);
            }
            WKCookieManagerSetHTTPCookieAcceptPolicy(_cookieManager, _httpCookieAcceptPolicy);

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

            if (_config.Transparent.Value() == true) {
                WKPageSetDrawsBackground(_page, false);
            }

            // Register handlers for page navigation and message from injected bundle.
            _handlerWebKit.base.clientInfo = static_cast<void*>(this);
            WKPageSetPageNavigationClient(_page, &_handlerWebKit.base);

            _handlerInjectedBundle.base.clientInfo = static_cast<void*>(this);
            WKContextSetInjectedBundleClient(wkContext, &_handlerInjectedBundle.base);

            WKPageSetProxies(_page, nullptr);

            WKPageSetCustomBackingScaleFactor(_page, _config.ScaleFactor.Value());

            if (_config.Automation.Value()) {
                _handlerAutomation.base.clientInfo = static_cast<void*>(this);
                WKContextSetAutomationClient(wkContext, &_handlerAutomation.base);
            }

            _handlerPageUI.base.clientInfo = static_cast<void*>(this);
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

            URL(static_cast<const string>(_URL));

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
            WKRelease(_userContentController);
            WKRelease(pageConfiguration);
            WKRelease(pageGroup);
            WKRelease(wkContext);
            WKRelease(preferences);

            g_main_context_pop_thread_default(_context);
            g_main_loop_unref(_loop);
            g_main_context_unref(_context);

            return Core::infinite;
        }
#endif // WEBKIT_GLIB_API

        void AddUserScriptImpl(const char* scriptContent, bool topFrameOnly)
        {
#ifdef WEBKIT_GLIB_API
            auto* userContentManager = webkit_web_view_get_user_content_manager(_view);
            auto* script = webkit_user_script_new(
                scriptContent,
                topFrameOnly ? WEBKIT_USER_CONTENT_INJECT_TOP_FRAME : WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
                WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
                nullptr,
                nullptr
                );
            webkit_user_content_manager_add_script(userContentManager, script);
            webkit_user_script_unref(script);
#else
            auto scriptContentRef = WKStringCreateWithUTF8CString(scriptContent);
            auto scriptRef = WKUserScriptCreateWithSource(scriptContentRef, kWKInjectAtDocumentStart, topFrameOnly);
            WKUserContentControllerAddUserScript(_userContentController, scriptRef);
            WKRelease(scriptRef);
            WKRelease(scriptContentRef);
#endif
        }

        void TryLoadingUserScripts()
        {
            if (_config.UserScripts.IsSet()) {
                auto loadScript = [&](const std::string& path) {
                    gchar* scriptContent;
                    auto success = g_file_get_contents(path.c_str(), &scriptContent, nullptr, nullptr);
                    if (!success) {
                        SYSLOG(Trace::Error, (_T("Unable to read user script '%s'"), path.c_str()));
                        return;
                    }
                    AddUserScriptImpl(scriptContent, false);
                    g_free(scriptContent);
                };
                for (unsigned userScriptIndex = 0; userScriptIndex < _config.UserScripts.Length(); userScriptIndex++) {
                    const auto& scriptPath = _config.UserScripts[userScriptIndex].Value();
                    const auto fullScriptPath = (!scriptPath.empty() && scriptPath[0] == '/') || _dataPath.empty()
                                                ? scriptPath
                                                : _dataPath + "/" + scriptPath;
                    TRACE(Trace::Information, (_T("Loading user script '%s'"), fullScriptPath.c_str()));
                    loadScript(fullScriptPath);
                }
            }
        }

        void SetupUserContentFilter()
        {
#ifdef WEBKIT_GLIB_API
            if (!_config.ContentFilter.Value().empty()) {
                // User content filter is compiled into binary-like file and put inside filter storage path.
                // The file is used to share the data between WebKit processes.
                // Filter storage path is the same for all browser instances: <cache_dir>/content_filters
                // Individual filter is put inside storage path as a file named ContentFilterList-<identifier>
                // where <identifier> is taken from webkit_user_content_filter_store_save() param
                // (_service->Callsign().c_str() in this case).
                // Each browser instance will have its own, single filter file, e.g.:
                //   <cache_dir>/content_filters/ContentFilterList-HtmlApp-0
                gchar* filtersPath = g_build_filename(g_get_user_cache_dir(), "content_filters", nullptr);
                WebKitUserContentFilterStore* store = webkit_user_content_filter_store_new(filtersPath);
                g_free(filtersPath);
                GBytes* data = g_bytes_new(_config.ContentFilter.Value().c_str(), _config.ContentFilter.Value().size());

                webkit_user_content_filter_store_save(store, _service->Callsign().c_str(), data, nullptr, [](GObject* obj, GAsyncResult* result, gpointer data) {
                    WebKitImplementation* webkit_impl = static_cast<WebKitImplementation*>(data);
                    WebKitUserContentFilter* filter = webkit_user_content_filter_store_save_finish(WEBKIT_USER_CONTENT_FILTER_STORE(obj), result, nullptr);
                    auto* userContentManager = webkit_web_view_get_user_content_manager(webkit_impl->_view);
                    webkit_user_content_manager_add_filter(userContentManager, filter);
                }, this);

                g_bytes_unref(data);
            }
#else
        // GLIB only supported
#endif
        }

        void CheckWebProcess()
        {
            if ( _webProcessCheckInProgress )
                return;

            _webProcessCheckInProgress = true;

#ifdef WEBKIT_GLIB_API
            webkit_web_view_is_web_process_responsive_async(
                _view,
                nullptr,
                [](GObject* object, GAsyncResult* result, gpointer user_data) {
                    bool isWebProcessResponsive = webkit_web_view_is_web_process_responsive_finish(WEBKIT_WEB_VIEW(object), result, nullptr);
                    WebKitImplementation* webkit_impl = static_cast<WebKitImplementation*>(user_data);
                    webkit_impl->DidReceiveWebProcessResponsivenessReply(isWebProcessResponsive);
                },
                this);
#else
            WKPageIsWebProcessResponsive(
                _page,
                this,
                [](bool isWebProcessResponsive, void* customdata) {
                    WebKitImplementation* object = static_cast<WebKitImplementation*>(customdata);
                    object->DidReceiveWebProcessResponsivenessReply(isWebProcessResponsive);
                });
#endif
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

#ifdef WEBKIT_GLIB_API
            std::string activeURL(webkit_web_view_get_uri(_view));
            if (_webprocessPID == -1) {
              _webprocessPID = webkit_web_view_get_web_process_identifier(_view);
            }
            pid_t webprocessPID = _webprocessPID;
#else
            std::string activeURL = GetPageActiveURL(GetPage());
            pid_t webprocessPID = WKPageGetProcessIdentifier(GetPage());
#endif

            if (isWebProcessResponsive) {
                SYSLOG(Logging::Notification, (_T("WebProcess recovered after %d unresponsive replies, pid=%u, url=%s\n"),
                                            _unresponsiveReplyNum, webprocessPID, activeURL.c_str()));
                _unresponsiveReplyNum = 0;
            } else {
                ++_unresponsiveReplyNum;
                SYSLOG(Logging::Notification, (_T("WebProcess is unresponsive, pid=%u, reply num=%d(max=%d), url=%s\n"),
                                            webprocessPID, _unresponsiveReplyNum, kWebProcessUnresponsiveReplyDefaultLimit,
                                            activeURL.c_str()));
            }

            if (!isWebProcessResponsive && _state == PluginHost::IStateControl::SUSPENDED) {
                SYSLOG(Logging::Notification, (_T("Killing unresponsive suspended WebProcess, pid=%u, reply num=%d(max=%d), url=%s\n"),
                                            webprocessPID, _unresponsiveReplyNum, kWebProcessUnresponsiveReplyDefaultLimit,
                                            activeURL.c_str()));
                if (_unresponsiveReplyNum <= kWebProcessUnresponsiveReplyDefaultLimit) {
                    _unresponsiveReplyNum = kWebProcessUnresponsiveReplyDefaultLimit;
                    Logging::DumpSystemFiles(webprocessPID);
                    if (syscall(__NR_tgkill, webprocessPID, webprocessPID, SIGFPE) == -1) {
                        SYSLOG(Trace::Error, (_T("tgkill failed, signal=%d process=%u errno=%d (%s)"), SIGFPE, webprocessPID, errno, strerror(errno)));
                    }
                } else {
                    DeactivateBrowser(PluginHost::IShell::FAILURE);
                }
                return;
            }

            if (_unresponsiveReplyNum == kWebProcessUnresponsiveReplyDefaultLimit) {
                Logging::DumpSystemFiles(webprocessPID);

                if (syscall(__NR_tgkill, webprocessPID, webprocessPID, SIGFPE) == -1) {
                    SYSLOG(Trace::Error, (_T("tgkill failed, signal=%d process=%u errno=%d (%s)"), SIGFPE, webprocessPID, errno, strerror(errno)));
                }
            } else if (_unresponsiveReplyNum == (2 * kWebProcessUnresponsiveReplyDefaultLimit)) {
                DeactivateBrowser(PluginHost::IShell::WATCHDOG_EXPIRED);
            }
        }

#ifdef WEBKIT_GLIB_API
        static void isWebProcessResponsiveCallback(WebKitWebView*, GParamSpec*, WebKitImplementation* self)
        {
            if (webkit_web_view_get_is_web_process_responsive(self->_view) == true) {

                if (self->_unresponsiveReplyNum > 0) {

                    std::string activeURL(webkit_web_view_get_uri(self->_view));
                    SYSLOG(Logging::Notification, (_T("WebProcess recovered after %d unresponsive replies, url=%s\n"),
                                                self->_unresponsiveReplyNum, activeURL.c_str()));
                    self->_unresponsiveReplyNum = 0;
                }
            }
        }
#else
        static void WebProcessDidBecomeResponsive(WKPageRef page, const void* clientInfo)
        {
            auto &self = *const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
            if (self._unresponsiveReplyNum > 0) {

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
        string _URL;
        string _dataPath;
        PluginHost::IShell* _service;
        string _headers;
        bool _localStorageEnabled;
        int32_t _httpStatusCode;

#ifdef WEBKIT_GLIB_API
        WebKitWebView* _view;
        uint64_t _guid;
        WebKitCookieAcceptPolicy _httpCookieAcceptPolicy;
        pid_t _webprocessPID;
        string _extensionPath;
        bool _ignoreLoadFinishedOnce;
#else
        WKViewRef _view;
        WKPageRef _page;
        WKWebAutomationSessionRef _automationSession;
        WKNotificationManagerRef _notificationManager;
        WKHTTPCookieAcceptPolicy _httpCookieAcceptPolicy;
        WKNavigationRef _navigationRef;
        string _consoleLogPrefix;
        WKUserContentControllerRef _userContentController;
        WKCookieManagerRef _cookieManager;
#endif
#if defined(ENABLE_CLOUD_COOKIE_JAR)
        Plugin::CookieJar _cookieJar;
        std::list<Exchange::IBrowserCookieJar::INotification*> _cookieJarClients;
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

    /* static */ void didStartProvisionalNavigation(WKPageRef page, WKNavigationRef navigation, WKTypeRef userData, const void* clientInfo)
    {
        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));

        WKURLRef urlRef = WKPageCopyActiveURL(page);
        WKStringRef urlStringRef = WKURLCopyString(urlRef);

        string url = WKStringToString(urlStringRef);

        browser->SetNavigationRef(navigation);
        browser->OnURLChanged(url);

        WKRelease(urlRef);
        WKRelease(urlStringRef);
    }

    /* static */ void didSameDocumentNavigation(const OpaqueWKPage* page, const OpaqueWKNavigation* nav, WKSameDocumentNavigationType type, const void* clientInfo, const void* info)
    {
        if (type == kWKSameDocumentNavigationAnchorNavigation) {
            WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(info));

            WKURLRef urlRef = WKPageCopyActiveURL(page);
            WKStringRef urlStringRef = WKURLCopyString(urlRef);

            string url = WKStringToString(urlStringRef);

            browser->OnURLChanged(url);

            WKRelease(urlRef);
            WKRelease(urlStringRef);
        }
    }

    /* static */ void didFinishDocumentLoad(WKPageRef page, WKNavigationRef navigation, WKTypeRef userData, const void* clientInfo)
    {

        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));

        WKURLRef urlRef = WKPageCopyActiveURL(page);
        WKStringRef urlStringRef = WKURLCopyString(urlRef);

        string url = WKStringToString(urlStringRef);

        browser->OnLoadFinished(url, navigation);

        WKRelease(urlRef);
        WKRelease(urlStringRef);
    }

    /* static */ void requestClosure(const void* clientInfo)
    {
        // WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
        // TODO: @Igalia, make sure the clientInfo is actually holding the correct clientINfo, currently it is nullptr. For
        // now we use the Singleton, this is fine as long as there is only 1 instance (in process) or it is always fine if we
        // are running out-of-process..
        WebKitImplementation* realBrowser = static_cast<WebKitImplementation*>(implementation);
        realBrowser->NotifyClosure();
    }

    /* static */ void onNotificationShow(WKPageRef page, WKNotificationRef notification, const void* clientInfo)
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

    /* static */ void onFrameDisplayed(WKViewRef view, const void* clientInfo)
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

    /* static */ void didFailNavigation(WKPageRef page, WKNavigationRef, WKErrorRef error, WKTypeRef, const void *clientInfo)
    {
        const int WebKitNetworkErrorCancelled = 302;
        int errorcode = WKErrorGetErrorCode(error);
        auto errorDomain = WKErrorCopyDomain(error);

        string url = GetPageActiveURL(page);
        string message(string("{ \"url\": \"") + url + string("\", \"Error code\":") + Core::NumberType<uint32_t>(errorcode).Text() + string(" }"));
        SYSLOG(Trace::Information, (_T("LoadFailed: %s"), message.c_str()));

        bool isCanceled =
            errorDomain &&
            WKStringIsEqualToUTF8CString(errorDomain, "WebKitNetworkError") &&
            WebKitNetworkErrorCancelled == WKErrorGetErrorCode(error);
        WKRelease(errorDomain);

        if (isCanceled)
            return;

        WebKitImplementation* browser = const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
        browser->OnLoadFailed(url);
    }

    /* static */ void webProcessDidCrash(WKPageRef, const void*)
    {
        SYSLOG(Trace::Fatal, (_T("CRASH: WebProcess crashed, exiting...")));
        exit(1);
    }

    /* static */ void willAddDetailedMessageToConsole(WKPageRef, WKStringRef source, WKStringRef, uint64_t line, uint64_t column, WKStringRef message, WKStringRef, const void* clientInfo)
    {
        auto &self = *const_cast<WebKitImplementation*>(static_cast<const WebKitImplementation*>(clientInfo));
        TRACE_GLOBAL(BrowserConsoleLog, (self.GetConsoleLogPrefix(), message, line, column));
    }
#endif // !WEBKIT_GLIB_API
} // namespace Plugin

} // namespace WPEFramework
