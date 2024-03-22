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

#include "testrunner.h"
#include "focusmanager.h"

#include <cassert>
#include <map>
#include <string>
#include <wpe/webkit.h>
#include <wpe/wpe.h>

namespace {

class TestRunnerImpl : public Testing::TestRunner {
public:
    TestRunnerImpl() {}
    ~TestRunnerImpl() {}

    bool EnsureInitialized(WebKitWebView* parent_view, const char* extension_dir) override;
    bool handleUserMessage(WebKitUserMessage *message) override;

private:
    bool handleRunTestCase(WebKitUserMessage *message);
    bool handleDestroyTestCase(WebKitUserMessage *message);


    bool handleUserMessageFromTestCase(WebKitWebView *webView,
                                       WebKitUserMessage *message);
    bool handleTestCaseEnded(WebKitUserMessage *message);

    void sendTestCaseResponse(bool success, const char* msg);
    bool createSubView();
    bool prepareExtensionsDir();
    void setParentVisibility(bool visible);

    static void initWebExtensionsCallback(WebKitWebContext *context,
                                          void *userData);
    static bool userMessageReceivedCallback(WebKitWebView *webView,
                                            WebKitUserMessage *message,
                                            void *userData);
    static void webProcessTerminatedCallback(WebKitWebView* webView,
                                             WebKitWebProcessTerminationReason reason,
                                             void *userData);
    static void loadFailedCallback(WebKitWebView* webView, WebKitLoadEvent loadEvent,
                                   const gchar* failingURI, GError* error, void *userData);

private:
    std::string m_extensionDir;
    WebKitWebView* m_parentView = nullptr;
    WebKitWebView* m_testCaseView = nullptr;
};

bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length())
        return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool TestRunnerImpl::EnsureInitialized(WebKitWebView* parent_view, const char* extension_dir) {
    if (m_parentView)
        return true;

    m_extensionDir = extension_dir;
    m_parentView = parent_view;

    Testing::FocusManager::Instance()->Start();

    return true;
}

bool TestRunnerImpl::handleUserMessage(WebKitUserMessage *message) {
    const char* msgName = webkit_user_message_get_name(message);
    if (endsWith(msgName, Testing::Tags::TestRunnerCreateView)) {
        handleRunTestCase(message);
    } else if (endsWith(msgName, Testing::Tags::TestRunnerDestroyView)) {
        handleDestroyTestCase(message);
    } else {
        g_warning("TestRunner: Received unknown user message '%s'", msgName);
    }
    return true;
}

bool TestRunnerImpl::handleRunTestCase(WebKitUserMessage *message) {
    GVariant *payload = webkit_user_message_get_parameters(message);
    if (!payload) {
        g_warning("TestRunner: Invalid message payload %p %p", message, payload);
        return false;
    }
    const char* testUrl = nullptr;
    g_variant_get(payload, "s", &testUrl);
    if (!testUrl) {
        g_warning("TestRunner: No test url provided");
        return false;
    }
    createSubView();
    assert(m_testCaseView);

    // load URL provided
    webkit_web_view_load_uri(m_testCaseView, testUrl);

    auto msgName = std::string(Testing::Tags::TestRunnerPrefix) + Testing::Tags::TestRunnerCreateViewReply;
    WebKitUserMessage* reply = webkit_user_message_new(msgName.c_str(), g_variant_new("b", true));
    webkit_user_message_send_reply(message, reply);

    // hide parent page so we are able to see a video layer
    setParentVisibility(false);
    return true;
}

bool TestRunnerImpl::createSubView() {
    // Use the same settings as parent view to make sure we have the same behavior
    WebKitWebsiteDataManager *dataManager =
        webkit_web_view_get_website_data_manager(m_parentView);
    WebKitWebContext *webkitContext =
        webkit_web_context_new_with_website_data_manager(dataManager);
    WebKitSettings *settings = webkit_web_view_get_settings (WEBKIT_WEB_VIEW(m_parentView));

    g_signal_connect(webkitContext, "initialize-web-extensions",
                     G_CALLBACK(initWebExtensionsCallback), this);

    m_testCaseView = WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
                                     "backend", webkit_web_view_backend_new(wpe_view_backend_create(), nullptr, nullptr),
                                     "web-context", webkitContext,
                                     "settings", settings,
                                     "is-controlled-by-automation", FALSE,
                                     nullptr));
    g_object_unref(webkitContext);

    g_signal_connect(m_testCaseView, "user-message-received", G_CALLBACK(userMessageReceivedCallback), this);
    g_signal_connect(m_testCaseView, "web-process-terminated", G_CALLBACK(webProcessTerminatedCallback), this);
    g_signal_connect(m_testCaseView, "load-failed", G_CALLBACK(loadFailedCallback), this);
    // TODO: need to handle those signals
    // g_signal_connect(m_testCaseView, "notify::is-web-process-responsive", G_CALLBACK(isWebProcessResponsiveCallback), this);
    return true;
}

void TestRunnerImpl::setParentVisibility(bool visible) {
    auto* backend = webkit_web_view_backend_get_wpe_backend(webkit_web_view_get_backend(m_parentView));
    assert(backend);
    if (visible) {
        wpe_view_backend_add_activity_state(backend, wpe_view_activity_state_in_window);
    } else {
        wpe_view_backend_remove_activity_state(backend, wpe_view_activity_state_in_window);
    }
}

bool TestRunnerImpl::handleDestroyTestCase(WebKitUserMessage *message) {
    g_clear_object(&m_testCaseView);
    setParentVisibility(true);
    return true;
}

bool TestRunnerImpl::handleUserMessageFromTestCase(WebKitWebView *webView,
                                                   WebKitUserMessage *message)
{
    // This is a message from test case sub-page WebProcess to UI Process
    assert(webView == m_testCaseView);
    const char* name = webkit_user_message_get_name(message);
    if (!g_str_has_prefix(name, Testing::Tags::TestCasePrefix)) {
        return false;
    }

    if (endsWith(name, Testing::Tags::TestCaseReportResult)) {
        handleTestCaseEnded(message);
    } else {
        g_warning("TestRunner: Unknown user message received from test case: %s", name);
    }
    return true;
}

bool TestRunnerImpl::handleTestCaseEnded(WebKitUserMessage *message) {
    GVariant *payload = webkit_user_message_get_parameters(message);
    if (!payload) {
        g_warning("TestRunner: Failed to get the user-message payload");
        return false;
    }
    gboolean success = FALSE;
    const char* msg = nullptr;
    g_variant_get(payload, "(b&s)", &success, &msg);

    sendTestCaseResponse(success, msg);

    return true;
}

void TestRunnerImpl::sendTestCaseResponse(bool success, const char* msg) {
    if (!msg) {
        msg = "(no message)";
    }
    auto msgName = std::string(Testing::Tags::TestRunnerPrefix) + Testing::Tags::TestRunnerCaseEnded;
    webkit_web_view_send_message_to_page(
        m_parentView,
        webkit_user_message_new(msgName.c_str(), g_variant_new("(bs)", success, msg)),
                                nullptr, nullptr, nullptr);
}

void TestRunnerImpl::initWebExtensionsCallback(WebKitWebContext *context,
                                               void *userData)
{
    TestRunnerImpl* runner = (TestRunnerImpl*)userData;
    webkit_web_context_set_web_extensions_directory(context, runner->m_extensionDir.c_str());
    GVariant* data = g_variant_new("(smsbb)", "", nullptr, true, true);
    webkit_web_context_set_web_extensions_initialization_user_data(context, data);
}

bool TestRunnerImpl::userMessageReceivedCallback(WebKitWebView *webView,
                                                 WebKitUserMessage *message,
                                                 void *userData)
{
    TestRunnerImpl* runner = (TestRunnerImpl*)userData;
    return runner->handleUserMessageFromTestCase(webView, message);
}

void TestRunnerImpl::webProcessTerminatedCallback(WebKitWebView* webView,
                                                  WebKitWebProcessTerminationReason reason,
                                                  void *userData)
{
    static auto reasons = []() {
        std::map<WebKitWebProcessTerminationReason, const char*> reasons;
#define EMPLACE_ENTRY(entry) reasons.emplace(entry, #entry)
        EMPLACE_ENTRY(WEBKIT_WEB_PROCESS_CRASHED);
        EMPLACE_ENTRY(WEBKIT_WEB_PROCESS_EXCEEDED_MEMORY_LIMIT);
        EMPLACE_ENTRY(WEBKIT_WEB_PROCESS_TERMINATED_BY_API);
#undef EMPLACE_ENTRY
        return reasons;
    }();

    g_warning("TestRunner: webProcessTerminatedCallback %d : %s", reason, reasons[reason]);
    std::string message = "TERMINATED: ";
    message += std::to_string(reason) + "[" + reasons[reason] + "]";

    TestRunnerImpl* runner = (TestRunnerImpl*)userData;
    runner->sendTestCaseResponse(false, message.c_str());
}

void TestRunnerImpl::loadFailedCallback(WebKitWebView* webView, WebKitLoadEvent loadEvent,
                                        const gchar* failingURI, GError* error, void *userData)
{
    g_warning("TestRunner: Failed to load URL %s [%s]", failingURI, error->message);
    std::string message = "LOAD_FAILED: ";
    message += error->message;

    TestRunnerImpl* runner = (TestRunnerImpl*)userData;
    runner->sendTestCaseResponse(false, message.c_str());
}

} // namespace

namespace Testing {

TestRunner* TestRunner::Instance() {
    static TestRunnerImpl testRunner;
    return &testRunner;
}

} // namespace Testing