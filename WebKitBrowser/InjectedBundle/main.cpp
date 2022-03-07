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

#include "Module.h"

#include <cstdio>
#include <memory>
#include <syslog.h>

#ifdef WEBKIT_GLIB_API

#include <wpe/webkit-web-extension.h>
#include "../BrowserConsoleLog.h"

using namespace WPEFramework;

#else

#include <WPE/WebKit.h>
#include <WPE/WebKit/WKBundleFrame.h>
#include <WPE/WebKit/WKBundlePage.h>
#include <WPE/WebKit/WKURL.h>

#include <cstdio>
#include <memory>
#include <syslog.h>

#include "ClassDefinition.h"
#include "NotifyWPEFramework.h"
#include "Utils.h"
#include "WhiteListedOriginDomainsList.h"
#include "RequestHeaders.h"

#if defined(ENABLE_BADGER_BRIDGE)
#include "BridgeObject.h"
#endif

#if defined(ENABLE_AAMP_JSBINDINGS)
#include "AAMPJSBindings.h"
#endif

using namespace WPEFramework;
using JavaScript::ClassDefinition;

WKBundleRef g_Bundle;
std::string g_currentURL;

namespace WPEFramework {
namespace WebKit {
namespace Utils {

WKBundleRef GetBundle() {
    return (g_Bundle);
}
std::string GetURL() {
    return (g_currentURL);
}

} } }

#endif

#include "WhiteListedOriginDomainsList.h"
using WebKit::WhiteListedOriginDomainsList;

static Core::NodeId GetConnectionNode()
{
    string nodeName;

    Core::SystemInfo::GetEnvironment(string(_T("COMMUNICATOR_CONNECTOR")), nodeName);

    return (Core::NodeId(nodeName.c_str()));
}

static class PluginHost {
private:
    PluginHost(const PluginHost&) = delete;
    PluginHost& operator=(const PluginHost&) = delete;

public:
    PluginHost()
        : _engine(Core::ProxyType<RPC::InvokeServerType<2, 0, 4>>::Create())
        , _comClient(Core::ProxyType<RPC::CommunicatorClient>::Create(GetConnectionNode(), Core::ProxyType<Core::IIPCServer>(_engine)))
    {
        _engine->Announcements(_comClient->Announcement());
    }
    ~PluginHost()
    {
        TRACE(Trace::Information, (_T("Destructing injected bundle stuff!!! [%d]"), __LINE__));
        Deinitialize();
    }

public:
    void Initialize(WKBundleRef bundle, const void* userData = nullptr)
    {
        // We have something to report back, do so...
        uint32_t result = _comClient->Open(RPC::CommunicationTimeOut);
        if (result != Core::ERROR_NONE) {
            TRACE(Trace::Error, (_T("Could not open connection to node %s. Error: %s"), _comClient->Source().RemoteId().c_str(), Core::NumberType<uint32_t>(result).Text().c_str()));
        } else {
            // Due to the LXC container support all ID's get mapped. For the TraceBuffer, use the host given ID.
            Trace::TraceUnit::Instance().Open(_comClient->ConnectionId());
        }
#ifdef WEBKIT_GLIB_API
        _bundle = WEBKIT_WEB_EXTENSION(g_object_ref(bundle));

        const char *uid;
        const char *whitelist;
        gboolean logToSystemConsoleEnabled;
        g_variant_get((GVariant*) userData, "(&sm&sb)", &uid, &whitelist, &logToSystemConsoleEnabled);

        _scriptWorld = webkit_script_world_new_with_name(uid);

        g_signal_connect(_scriptWorld, "window-object-cleared",
                G_CALLBACK(windowObjectClearedCallback), nullptr);

        if (logToSystemConsoleEnabled == TRUE) {
            g_signal_connect(bundle, "page-created", G_CALLBACK(pageCreatedCallback), this);
        }

        if (whitelist != nullptr) {
            _whiteListedOriginDomainPairs =
                    WhiteListedOriginDomainsList::RequestFromWPEFramework(
                            whitelist);
            WhiteList(bundle);
        }
#else
        _whiteListedOriginDomainPairs = WhiteListedOriginDomainsList::RequestFromWPEFramework();
        #if defined(ENABLE_BADGER_BRIDGE)
        JavaScript::BridgeObject::Initialize();
        #endif
#endif

    }

    void Deinitialize()
    {
        if (_comClient.IsValid() == true) {
            _comClient.Release();
        }
#ifdef WEBKIT_GLIB_API
        g_object_unref(_scriptWorld);
        g_object_unref(_bundle);
#endif
        Core::Singleton::Dispose();
    }

    void WhiteList(WKBundleRef bundle)
    {
        // Whitelist origin/domain pairs for CORS, if set.
        if (_whiteListedOriginDomainPairs) {
            _whiteListedOriginDomainPairs->AddWhiteListToWebKit(bundle);
        }
    }

#ifdef WEBKIT_GLIB_API

private:
    static void automationMilestone(const char* arg1, const char* arg2, const char* arg3)
    {
        g_printerr("TEST TRACE: \"%s\" \"%s\" \"%s\"\n", arg1, arg2, arg3);
        TRACE_GLOBAL(Trace::Information, (_T("TEST TRACE: \"%s\" \"%s\" \"%s\""), arg1, arg2, arg3));
    }

    static void windowObjectClearedCallback(WebKitScriptWorld* world, WebKitWebPage*, WebKitFrame* frame)
    {
        if (webkit_frame_is_main_frame(frame) == false)
            return;

        JSCContext* jsContext = webkit_frame_get_js_context_for_script_world(frame, world);

        JSCValue* automation = jsc_value_new_object(jsContext, nullptr, nullptr);
        JSCValue* function = jsc_value_new_function(jsContext, nullptr, reinterpret_cast<GCallback>(automationMilestone),
            nullptr, nullptr, G_TYPE_NONE, 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        jsc_value_object_set_property(automation, "Milestone", function);
        g_object_unref(function);
        jsc_context_set_value(jsContext, "automation", automation);
        g_object_unref(automation);

        static const char wpeNotifyWPEFramework[] = "var wpe = {};\n"
            "wpe.NotifyWPEFramework = function() {\n"
            "  let retval = new Array;\n"
            "  for (let i = 0; i < arguments.length; i++) {\n"
            "    retval[i] = arguments[i];\n"
            "  }\n"
            "  window.webkit.messageHandlers.wpeNotifyWPEFramework.postMessage(retval);\n"
            "}";
        JSCValue* result = jsc_context_evaluate(jsContext, wpeNotifyWPEFramework, -1);
        g_object_unref(result);

        g_object_unref(jsContext);
    }
    static void pageCreatedCallback(WebKitWebExtension*, WebKitWebPage* page, PluginHost* host)
    {
        g_signal_connect(page, "console-message-sent",
                G_CALLBACK(consoleMessageSentCallback), nullptr);
    }
    static void consoleMessageSentCallback(WebKitWebPage* page, WebKitConsoleMessage* message)
    {
        string messageString = Core::ToString(webkit_console_message_get_text(message));
        uint64_t line = static_cast<uint64_t>(webkit_console_message_get_line(message));

        TRACE_GLOBAL(BrowserConsoleLog, (messageString, line, 0));
    }

    WKBundleRef _bundle;
    WebKitScriptWorld* _scriptWorld;

#endif

private:
    Core::ProxyType<RPC::InvokeServerType<2, 0, 4> > _engine;
    Core::ProxyType<RPC::CommunicatorClient> _comClient;

    // White list for CORS.
    std::unique_ptr<WhiteListedOriginDomainsList> _whiteListedOriginDomainPairs;

} _wpeFrameworkClient;

extern "C" {

__attribute__((destructor)) static void unload()
{
    _wpeFrameworkClient.Deinitialize();
}

#ifdef WEBKIT_GLIB_API

// Declare module name for tracer.
MODULE_NAME_DECLARATION(BUILD_REFERENCE)

G_MODULE_EXPORT void webkit_web_extension_initialize_with_user_data(WebKitWebExtension* extension, GVariant* userData)
{
    _wpeFrameworkClient.Initialize(extension, userData);
}

#else

// Adds class to JS world.
static void InjectInJSWorld(ClassDefinition& classDef, WKBundleFrameRef frame, WKBundleScriptWorldRef scriptWorld)
{
    JSGlobalContextRef context = WKBundleFrameGetJavaScriptContextForWorld(frame, scriptWorld);

    ClassDefinition::FunctionIterator function = classDef.GetFunctions();
    uint32_t functionCount = function.Count();

    // We need an extra entry that we set to all zeroes, to signal end of data.
    std::vector<JSStaticFunction> staticFunctions;
    staticFunctions.reserve(functionCount + 1);

    int index = 0;
    while (function.Next()) {
        staticFunctions[index++] = (*function)->BuildJSStaticFunction();
    }

    staticFunctions[functionCount] = { nullptr, nullptr, 0 };

    JSClassDefinition jsClassDefinition = {
        0, // version
        kJSClassAttributeNone, //attributes
        classDef.GetClassName().c_str(), // className
        0, // parentClass
        nullptr, // staticValues
        staticFunctions.data(), // staticFunctions
        nullptr, //initialize
        nullptr, //finalize
        nullptr, //hasProperty
        nullptr, //getProperty
        nullptr, //setProperty
        nullptr, //deleteProperty
        nullptr, //getPropertyNames
        nullptr, //callAsFunction
        nullptr, //callAsConstructor
        nullptr, //hasInstance
        nullptr, //convertToType
    };

    JSClassRef jsClass = JSClassCreate(&jsClassDefinition);
    JSValueRef jsObject = JSObjectMake(context, jsClass, nullptr);
    JSClassRelease(jsClass);

    // @Zan: can we make extension name same as ClassName?
    JSStringRef extensionString = JSStringCreateWithUTF8CString(classDef.GetExtName().c_str());
    JSObjectSetProperty(context, JSContextGetGlobalObject(context), extensionString, jsObject,
        kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, nullptr);
    JSStringRelease(extensionString);
}

static bool shouldGoToBackForwardListItem(WKBundlePageRef, WKBundleBackForwardListItemRef item, WKTypeRef*, const void*)
{
    bool result = true;
    if (item) {
        auto itemUrl = WKBundleBackForwardListItemCopyURL(item);
        auto blankUrl = WKURLCreateWithUTF8CString("about:blank");
        result = !WKURLIsEqual(itemUrl, blankUrl);
        WKRelease(blankUrl);
        WKRelease(itemUrl);
    }
    return result;
}

static void updateCurrentURL(WKBundleFrameRef frame)
{
    if (WKBundleFrameIsMainFrame(frame)) {
        WKURLRef mainFrameURL = WKBundleFrameCopyURL(frame);
        WKStringRef urlString = WKURLCopyString(mainFrameURL);
        g_currentURL = WebKit::Utils::WKStringToString(urlString);
        WKRelease(urlString);
        WKRelease(mainFrameURL);
    }
}

static WKBundlePageLoaderClientV6 s_pageLoaderClient = {
    { 6, nullptr },
    // didStartProvisionalLoadForFrame
    [](WKBundlePageRef page, WKBundleFrameRef frame, WKTypeRef*, const void *) {
        #if defined(ENABLE_AAMP_JSBINDINGS)
        JavaScript::AAMP::UnloadJSBindings(frame);
        #endif

        if (WKBundleFrameIsMainFrame(frame)) {
            auto blankUrl = WKURLCreateWithUTF8CString("about:blank");
            auto frameUrl = WKBundleFrameCopyURL(frame);
            if (WKURLIsEqual(frameUrl, blankUrl)) {
                WKBundleBackForwardListClear(WKBundlePageGetBackForwardList(page));
            }
            WKRelease(blankUrl);
            WKRelease(frameUrl);
        }
    },
    nullptr, // didReceiveServerRedirectForProvisionalLoadForFrame
    nullptr, // didFailProvisionalLoadWithErrorForFrame
    // didCommitLoadForFrame
    [](WKBundlePageRef, WKBundleFrameRef frame, WKTypeRef*, const void *) {
        updateCurrentURL(frame);
    },
    nullptr, // didFinishDocumentLoadForFrame
    // didFinishLoadForFrame
    [](WKBundlePageRef, WKBundleFrameRef frame, WKTypeRef*, const void*) {
        updateCurrentURL(frame);
    },
    nullptr, // didFailLoadWithErrorForFrame
    nullptr, // didSameDocumentNavigationForFrame
    nullptr, // didReceiveTitleForFrame
    nullptr, // didFirstLayoutForFrame
    nullptr, // didFirstVisuallyNonEmptyLayoutForFrame
    nullptr, // didRemoveFrameFromHierarchy
    nullptr, // didDisplayInsecureContentForFrame
    nullptr, // didRunInsecureContentForFrame
    // didClearWindowObjectForFrame
    [](WKBundlePageRef page, WKBundleFrameRef frame, WKBundleScriptWorldRef scriptWorld, const void*) {
        bool isMainCtx = (WKBundleFrameGetJavaScriptContext(frame) == WKBundleFrameGetJavaScriptContextForWorld(frame, scriptWorld));
        if (isMainCtx) {
            #if defined(ENABLE_AAMP_JSBINDINGS)
            JavaScript::AAMP::LoadJSBindings(frame);
            #endif
            #if defined(ENABLE_BADGER_BRIDGE)
            JavaScript::BridgeObject::InjectJS(frame);
            #endif
        }

        // Add JS classes to JS world.
        ClassDefinition::Iterator ite = ClassDefinition::GetClassDefinitions();
        while (ite.Next()) {
            InjectInJSWorld(*ite, frame, scriptWorld);
        }
    },
    nullptr, // didCancelClientRedirectForFrame
    nullptr, // willPerformClientRedirectForFrame
    nullptr, // didHandleOnloadEventsForFrame
    nullptr, // didLayoutForFrame
    nullptr, // didNewFirstVisuallyNonEmptyLayout_unavailable
    nullptr, // didDetectXSSForFrame
    shouldGoToBackForwardListItem,
    nullptr, // globalObjectIsAvailableForFrame
    nullptr, // willDisconnectDOMWindowExtensionFromGlobalObject
    nullptr, // didReconnectDOMWindowExtensionToGlobalObject
    nullptr, // willDestroyGlobalObjectForDOMWindowExtension
    nullptr, // didFinishProgress
    nullptr, // shouldForceUniversalAccessFromLocalURL
    nullptr, // didReceiveIntentForFrame_unavailable
    nullptr, // registerIntentServiceForFrame_unavailable
    nullptr, // didLayout
    nullptr, // featuresUsedInPage
    nullptr, // willLoadURLRequest
    nullptr, // willLoadDataRequest
};

static WKBundlePageUIClientV4 s_pageUIClient = {
    { 4, nullptr },
    nullptr, // willAddMessageToConsole
    nullptr, // willSetStatusbarText
    nullptr, // willRunJavaScriptAlert
    nullptr, // willRunJavaScriptConfirm
    nullptr, // willRunJavaScriptPrompt
    nullptr, // mouseDidMoveOverElement
    nullptr, // pageDidScroll
    nullptr, // unused1
    nullptr, // shouldGenerateFileForUpload
    nullptr, // generateFileForUpload
    nullptr, // unused2
    nullptr, // statusBarIsVisible
    nullptr, // menuBarIsVisible
    nullptr, // toolbarsAreVisible
    nullptr, // didReachApplicationCacheOriginQuota
    nullptr, // didExceedDatabaseQuota
    nullptr, // createPlugInStartLabelTitle
    nullptr, // createPlugInStartLabelSubtitle
    nullptr, // createPlugInExtraStyleSheet
    nullptr, // createPlugInExtraScript
    nullptr, // unused3
    nullptr, // unused4
    nullptr, // unused5
    nullptr, // didClickAutoFillButton
    //willAddDetailedMessageToConsole
    [](WKBundlePageRef page, WKConsoleMessageSource source, WKConsoleMessageLevel level, WKStringRef message, uint32_t lineNumber,
        uint32_t columnNumber, WKStringRef url, const void* clientInfo) {
        auto prepareMessage = [&]() {
            string messageString = WebKit::Utils::WKStringToString(message);
            const uint16_t maxStringLength = Trace::TRACINGBUFFERSIZE - 1;
            if (messageString.length() > maxStringLength) {
                messageString = messageString.substr(0, maxStringLength);
            }
            return messageString;
        };

        // TODO: use "Trace" classes for different levels.
        TRACE_GLOBAL(Trace::Information, (prepareMessage()));
    }
};

static WKURLRequestRef willSendRequestForFrame(
  WKBundlePageRef page, WKBundleFrameRef, uint64_t, WKURLRequestRef request, WKURLResponseRef, const void*) {
    WebKit::ApplyRequestHeaders(page, request);
    WKRetain(request);
    return request;
}

static void didReceiveMessageToPage(
  WKBundleRef, WKBundlePageRef page, WKStringRef messageName, WKTypeRef messageBody, const void*) {
    if (WKStringIsEqualToUTF8CString(messageName, Tags::Headers)) {
        WebKit::SetRequestHeaders(page, messageBody);
        return;
    }

    #if defined(ENABLE_BADGER_BRIDGE)
    if (JavaScript::BridgeObject::HandleMessageToPage(page, messageName, messageBody))
        return;
    #endif
}

static void willDestroyPage(WKBundleRef, WKBundlePageRef page, const void*)
{
    WebKit::RemoveRequestHeaders(page);
}

static WKBundlePageResourceLoadClientV0 s_resourceLoadClient = {
    {0, nullptr},
    nullptr, // didInitiateLoadForResource
    willSendRequestForFrame,
    nullptr, // didReceiveResponseForResource
    nullptr, // didReceiveContentLengthForResource
    nullptr, // didFinishLoadForResource
    nullptr // didFailLoadForResource
};

static WKBundleClientV1 s_bundleClient = {
    { 1, nullptr },
    // didCreatePage
    [](WKBundleRef bundle, WKBundlePageRef page, const void* clientInfo) {
        // Register page loader client, for javascript callbacks.
        WKBundlePageSetPageLoaderClient(page, &s_pageLoaderClient.base);

        // Register UI client, this one will listen to log messages.
        WKBundlePageSetUIClient(page, &s_pageUIClient.base);

        WKBundlePageSetResourceLoadClient(page, &s_resourceLoadClient.base);

        _wpeFrameworkClient.WhiteList(bundle);
    },
    willDestroyPage, // willDestroyPage
    nullptr, // didInitializePageGroup
    nullptr, // didReceiveMessage
    didReceiveMessageToPage, // didReceiveMessageToPage
};

// Declare module name for tracer.
MODULE_NAME_DECLARATION(BUILD_REFERENCE)

extern "C" __attribute__((__visibility__("default")))
void WKBundleInitialize(WKBundleRef bundle, WKTypeRef)
{
    g_Bundle = bundle;

    _wpeFrameworkClient.Initialize(bundle);

    WKBundleSetClient(bundle, &s_bundleClient.base);
}

#endif

}
