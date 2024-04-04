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

#include "../tags.h"
#include <wpe/webkit-web-extension.h>
#include <map>
#include <string>

namespace {

// map of event listeners for current test case
static std::map<std::string, JSCValue*> eventListeners;
// current test case JS object
static JSCValue* jsTestCaseObj = nullptr;

static void throwJSException(JSCContext *jsContext, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    JSCException* jsException = jsc_exception_new_vprintf(jsContext, format, args);
    va_end(args);

    g_warning("TestRunnerJS: JSC exception thrown - %s", jsc_exception_get_message(jsException));
    jsc_context_throw_exception(jsContext, jsException);
    g_object_unref(jsException);
}


static bool callEventListener(const std::string& eventName, JSCValue* event)
{
    auto it = eventListeners.find(eventName);
    if (it == eventListeners.end()) {
        g_warning("TestRunnerJS: Event listener %s not registered", eventName.c_str());
        return false;
    }
    JSCValue* ignore = jsc_value_function_call(it->second, JSC_TYPE_VALUE, event, G_TYPE_NONE);
    g_object_unref(ignore);
    return true;
}

static bool sendEndedEvent(WebKitWebPage* page, bool success, const std::string& message)
{
    WebKitFrame* frame = webkit_web_page_get_main_frame(page);

    JSCContext* jsContext = webkit_frame_get_js_context(frame);
    JSCValue* jsEventObject = jsc_value_new_object(jsContext, nullptr, nullptr);

    JSCValue* jsSuccessVal = jsc_value_new_boolean(jsContext, success);
    jsc_value_object_define_property_data(jsEventObject,
                                          "success",
                                          JSC_VALUE_PROPERTY_CONFIGURABLE,
                                          jsSuccessVal);

    JSCValue* jsMessageVal = jsc_value_new_string(jsContext, message.c_str());
    jsc_value_object_define_property_data(jsEventObject,
                                          "message",
                                          JSC_VALUE_PROPERTY_CONFIGURABLE,
                                          jsMessageVal);

    callEventListener("ended", jsEventObject);

    g_object_unref(jsMessageVal);
    g_object_unref(jsSuccessVal);
    g_object_unref(jsEventObject);
    g_object_unref(jsContext);
    return true;
}

static void onRunTestCaseReply(GObject *object, GAsyncResult *result,
                               gpointer userData)
{
    WebKitWebPage* page = (WebKitWebPage*) userData;
    GError *error = nullptr;
    WebKitUserMessage* reply = webkit_web_page_send_message_to_view_finish(page, result, &error);
    if (error) {
        g_warning("TestRunnerJS: Failed to run test case %s", error->message);
        sendEndedEvent(page, false, error->message);
        g_error_free(error);
        return;
    }

    gboolean success = FALSE;
    auto replyParams = webkit_user_message_get_parameters(reply);
    g_variant_get(replyParams, "b", &success);
    if (!success)
        sendEndedEvent(page, false, "Failed to create view");

    g_object_unref(reply);
}

static void onTestCaseDestroy(WebKitWebPage *page)
{
    jsTestCaseObj = nullptr;
    eventListeners.clear();
    auto msgName = std::string(Testing::Tags::TestRunnerPrefix) + Testing::Tags::TestRunnerDestroyView;
    WebKitUserMessage* message = webkit_user_message_new(msgName.c_str(),
                                                         nullptr);
    webkit_web_page_send_message_to_view(page,
                                         message,
                                         nullptr,
                                         nullptr,
                                         nullptr);
}

static void onTestCaseAddEventListener(const char* event_name, JSCValue* function, gpointer userData)
{
    if (!jsc_value_is_function(function)) {
        g_warning("TestRunnerJS: addEventListener receive a non-function param.");
        return;
    }

    eventListeners[event_name] = (JSCValue*)g_object_ref(function);
}

static JSCValue* onTestRunnerRunTestCase(GPtrArray* array, WebKitWebPage *page)
{
    JSCContext *jsContext = jsc_context_get_current();

    // check an url string was supplied
    if (array->len < 1) {
        throwJSException(jsContext, "missing url argument");
        return jsc_value_new_null(jsContext);
    }

    gpointer arg0 = g_ptr_array_index(array, 0);
    if (!JSC_IS_VALUE(arg0) || !jsc_value_is_string(JSC_VALUE(arg0))) {
        throwJSException(jsContext, "invalid url argument");
        return jsc_value_new_null(jsContext);
    }

    if (jsTestCaseObj) {
        throwJSException(jsContext, "Another test case in progress");
        return jsc_value_new_null(jsContext);
    }

    gchar *str = jsc_value_to_string(JSC_VALUE(arg0));
    const std::string url(str);
    g_free(str);

    std::string msgName = std::string(Testing::Tags::TestRunnerPrefix) + Testing::Tags::TestRunnerCreateView;
    WebKitUserMessage *message = webkit_user_message_new(msgName.c_str(),
                                                         g_variant_new("s", url.c_str()));
    webkit_web_page_send_message_to_view(page,
                                         message,
                                         nullptr,
                                         onRunTestCaseReply,
                                         page);

    jsTestCaseObj = jsc_value_new_object(jsContext, nullptr, nullptr);

    JSCValue* jsAddEventListenerFunction = jsc_value_new_function(
            jsContext,
            "addEventListener",
            G_CALLBACK(onTestCaseAddEventListener),
            nullptr,
            nullptr,
            G_TYPE_NONE,
            2, G_TYPE_STRING, JSC_TYPE_VALUE);
    jsc_value_object_set_property(jsTestCaseObj, "addEventListener", jsAddEventListenerFunction);
    g_object_unref(jsAddEventListenerFunction);

    JSCValue* jsDestroyFunction = jsc_value_new_function(
            jsContext,
            "destroy",
            G_CALLBACK(onTestCaseDestroy),
            page,
            nullptr,
            G_TYPE_NONE,
            0, G_TYPE_NONE);
    jsc_value_object_set_property(jsTestCaseObj, "destroy", jsDestroyFunction);
    g_object_unref(jsDestroyFunction);

    return jsTestCaseObj;
}

static void onTestCaseReportResult(gboolean success, const char* msg, WebKitWebPage* page)
{
    auto msgName = std::string(Testing::Tags::TestCasePrefix) + Testing::Tags::TestCaseReportResult;
    WebKitUserMessage *message = webkit_user_message_new(msgName.c_str(),
                                                         g_variant_new("(bs)",
                                                         success, msg));
    webkit_web_page_send_message_to_view(page,
                                         message,
                                         nullptr,
                                         nullptr,
                                         nullptr);
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length())
        return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

} // namespace

namespace WPEFramework {
namespace JavaScript {
namespace TestRunner {

void InjectJS(WebKitScriptWorld *world, WebKitWebPage *page, WebKitFrame *frame)
{
    if (!world || !page || !frame) {
        g_warning("Invalid args: %p %p %p", world, page, frame);
        return;
    }

    if (!webkit_frame_is_main_frame(frame)) {
        return;
    }

    JSCContext* jsContext = webkit_frame_get_js_context_for_script_world(frame, world);

    // window.testRunner.runTestCase
    JSCValue* jsTestRunnerObject = jsc_value_new_object(jsContext, nullptr, nullptr);
    JSCValue* jsRunTestCaseFunction = jsc_value_new_function_variadic(
        jsContext, nullptr, G_CALLBACK(onTestRunnerRunTestCase), page, nullptr, JSC_TYPE_VALUE);
    jsc_value_object_set_property(jsTestRunnerObject, "runTestCase", jsRunTestCaseFunction);
    jsc_context_set_value(jsContext, "testRunner", jsTestRunnerObject);
    g_object_unref(jsRunTestCaseFunction);
    g_object_unref(jsTestRunnerObject);

    // window.testCase.reportTestResult
    JSCValue* jsTestCaseObject = jsc_value_new_object(jsContext, nullptr, nullptr);
    JSCValue* jsReportResultFunction = jsc_value_new_function(
        jsContext,
        "reportResult",
        G_CALLBACK(onTestCaseReportResult),
        page,
        nullptr,
        G_TYPE_NONE,
        2, G_TYPE_BOOLEAN, G_TYPE_STRING);
    jsc_value_object_set_property(jsTestCaseObject, "reportResult", jsReportResultFunction);
    jsc_context_set_value(jsContext, "testCase", jsTestCaseObject);
    g_object_unref(jsReportResultFunction);
    g_object_unref(jsTestCaseObject);

    g_object_unref(jsContext);
}

bool HandleMessageToPage(WebKitWebPage* page, WebKitUserMessage* message)
{
    // Message from UI process to testRunner web page
    const char* name = webkit_user_message_get_name(message);
    if (endsWith(name, Testing::Tags::TestRunnerCaseEnded)) {
        gboolean success = FALSE;
        const char* msg = nullptr;
        auto replyParams = webkit_user_message_get_parameters(message);
        g_variant_get(replyParams, "(b&s)", &success, &msg);

        sendEndedEvent(page, success, msg);
    }
    return true;
}

} // TestRunner
} // JavaScript
} // WPEFramework
