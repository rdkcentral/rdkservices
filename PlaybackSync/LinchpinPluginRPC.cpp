/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#include <sstream>

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wformat"
#include "utils.h"
#include "LinchpinPluginRPC.h"

#define LINCHPIN_THUNDER_TIMEOUT 20000

const string g_stringLinchpinPluginCallsign("org.rdk.LinchPinClient");

namespace WPEFramework
{
    namespace Plugin
    {
        // Methods
        enum
        {
            LINCHPIN_METHOD_CONNECT_ID = 0,
            LINCHPIN_METHOD_DISCONNECT_ID,
            LINCHPIN_METHOD_SUBSCRIBE_ID,
            LINCHPIN_METHOD_UNSUBSCRIBE_ID,
            LINCHPIN_METHOD_FETCH_MEESAGE_ID,
            LINCHPIN_METHOD_PUBLISH_ID
        };

        const char* g_linchpinMethodNames[] =
        {
            [LINCHPIN_METHOD_CONNECT_ID]        = "connect",
            [LINCHPIN_METHOD_DISCONNECT_ID]     = "disconnect",
            [LINCHPIN_METHOD_SUBSCRIBE_ID]      = "subscribe",
            [LINCHPIN_METHOD_UNSUBSCRIBE_ID]    = "unsubscribe",
            [LINCHPIN_METHOD_FETCH_MEESAGE_ID]  = "fetchMessage",
            [LINCHPIN_METHOD_PUBLISH_ID]        = "publish"
        };
        // End Methods

        // Subscribed Events
        enum
        {
            LINCHPIN_EVENT_LINCHPIN_NOTIFY_ID = 0,
            LINCHPIN_EVENT_LINCHPIN_CONNECTION_ID
        };

        const char* g_eventNames[] =
        {
            [LINCHPIN_EVENT_LINCHPIN_NOTIFY_ID]        = "onNotifyMessageReceived",
            [LINCHPIN_EVENT_LINCHPIN_CONNECTION_ID]    = "onConnectionClosed"
        };
        // End Subscribed Events

        // Parameters
        enum
        {
            LINCHPIN_PARAM_ENDPOINT_ID = 0,
            LINCHPIN_PARAM_CLIENTNAME_ID,
            LINCHPIN_PARAM_CREDENTIALS_ID,
            LINCHPIN_PARAM_CONNECTION_ID,
            LINCHPIN_PARAM_STATUS_ID,
            LINCHPIN_PARAM_STATUS_CODE_ID,
            LINCHPIN_PARAM_TOPIC_ID,
            LINCHPIN_PARAM_TYPE_ID,
            LINCHPIN_PARAM_PAYLOAD_ID,
            LINCHPIN_PARAM_PAYLOAD_EVENT_ID,
            LINCHPIN_PARAM_PAYLOAD_PARAM_ID
        };

        const char* g_paramNames[] =
        {
            [LINCHPIN_PARAM_ENDPOINT_ID]        = "endpoint",
            [LINCHPIN_PARAM_CLIENTNAME_ID]      = "clientName",
            [LINCHPIN_PARAM_CREDENTIALS_ID]     = "credentials",
            [LINCHPIN_PARAM_CONNECTION_ID]      = "connectionID",
            [LINCHPIN_PARAM_STATUS_ID]          = "connectionStatus",
            [LINCHPIN_PARAM_STATUS_CODE_ID]     = "lastStatusCode",
            [LINCHPIN_PARAM_TOPIC_ID]           = "topic",
            [LINCHPIN_PARAM_TYPE_ID]            = "payloadType",
            [LINCHPIN_PARAM_PAYLOAD_ID]         = "payload",
            [LINCHPIN_PARAM_PAYLOAD_EVENT_ID]   = "event",
            [LINCHPIN_PARAM_PAYLOAD_PARAM_ID]   = "param"
        };
        // End Parameters


        enum
        {
            LINCHPIN_PARAM_STATUS_CONNECTED_ID = 0,
            LINCHPIN_PARAM_STATUS_DISCONNECTED_ID
        };

        const char* g_paramStatus[] =
        {
            [LINCHPIN_PARAM_STATUS_CONNECTED_ID]    = "CONNECTED",
            [LINCHPIN_PARAM_STATUS_DISCONNECTED_ID] = "DISCONNECTED"
        };

        LinchpinPluginRPC::LinchpinPluginRPC() : InterPluginRPC() {}

        LinchpinPluginRPC::LinchpinPluginRPC(ILinchpinPluginEvents* handler) : InterPluginRPC(), m_handlerLinchpinEvents(handler) {}

        LinchpinPluginRPC::LinchpinPluginRPC(PluginHost::IShell* service) : InterPluginRPC(service, g_stringLinchpinPluginCallsign) {}

        LinchpinPluginRPC::~LinchpinPluginRPC() { Disconnect(); }

        bool LinchpinPluginRPC::Disconnect()
        {
            if (m_isConnected)
            {
                if (!m_connectionID.empty())
                {
                    JsonObject params, response;
                    params[g_paramNames[LINCHPIN_PARAM_CONNECTION_ID]] = m_connectionID;
                    InterPluginRPC::Invoke(LINCHPIN_THUNDER_TIMEOUT, string(g_linchpinMethodNames[LINCHPIN_METHOD_DISCONNECT_ID]), params, response);
                }
                // Clear the connection state regardless of return state of method above!!!
                m_isConnected = false;
                m_connectionID.clear();
            }
            return true;
        }

        bool LinchpinPluginRPC::Connect(const string& endpoint, const string& client, int64_t type, const string& token)
        {
            JsonObject params, response;
            uint32_t code;

            // Cleanup any existing connections
            Disconnect();

            params[g_paramNames[LINCHPIN_PARAM_ENDPOINT_ID]] = endpoint;
            params[g_paramNames[LINCHPIN_PARAM_CLIENTNAME_ID]] = client;
            params[g_paramNames[LINCHPIN_PARAM_CREDENTIALS_ID]] = token;
            code = InterPluginRPC::Invoke(LINCHPIN_THUNDER_TIMEOUT, string(g_linchpinMethodNames[LINCHPIN_METHOD_CONNECT_ID]), params, response);

            std::stringstream message;
            bool rc = false;
            if (Core::ERROR_NONE != code || !response.HasLabel(g_paramNames[LINCHPIN_PARAM_STATUS_ID]) ||
                !response.HasLabel(g_paramNames[LINCHPIN_PARAM_CONNECTION_ID]))
                message << "Not able to connect ";
            else
            {
                if (string(g_paramStatus[LINCHPIN_PARAM_STATUS_CONNECTED_ID]) != response[g_paramNames[LINCHPIN_PARAM_STATUS_ID]].String())
                    message << "Not connected ";
                else
                {
                    rc = true;
                    m_isConnected = true;
                    m_connectionID = response[g_paramNames[LINCHPIN_PARAM_CONNECTION_ID]].String();
                    m_publishType = type;
                    message << "Connected ";
                }
            }
            string stringResponse;
            response.ToString(stringResponse);
            message << "to endpoint '" << endpoint << "' response '" << stringResponse << '\'';
            LOGINFO("%s", message.str().c_str());
            return rc;
        }

        bool LinchpinPluginRPC::Attach(PluginHost::IShell* service)
        {
            Utils::activatePlugin(g_stringLinchpinPluginCallsign.c_str());
            bool rc = InterPluginRPC::Attach(service, g_stringLinchpinPluginCallsign);
            if (!rc)
                LOGINFO("Not able to open inter plugin RPC with %s", g_stringLinchpinPluginCallsign.c_str());
            else
            {
                // Subscribe to linchpin plugin events here now that we are attached
                std::string sourceCallsign = g_stringLinchpinPluginCallsign + ".1";
                std::string sinkCallsign = service->Callsign() + ".1";
                string token;
                Utils::SecurityToken::getSecurityToken(token);
                string query = "token=" + token;
                Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));

                m_linchpinConnection = std::make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(sourceCallsign.c_str(),
                                            sinkCallsign.c_str(), false, query);
                if (!m_linchpinConnection)
                    LOGERR("Failed to subscribe to linchpin, callsign '%s' for events", sourceCallsign.c_str());
                else
                {
                    int32_t status = Core::ERROR_GENERAL;
                    status = m_linchpinConnection->Subscribe<JsonObject>(LINCHPIN_THUNDER_TIMEOUT,
                    _T(g_eventNames[LINCHPIN_EVENT_LINCHPIN_NOTIFY_ID]), &LinchpinPluginRPC::onNotifyMessageReceived, this);
                    if (Core::ERROR_NONE != status)
                        LOGERR("Failed to subscribe to linchpin event '%s' failure code %d", g_eventNames[LINCHPIN_EVENT_LINCHPIN_NOTIFY_ID], status);

                    status = m_linchpinConnection->Subscribe<JsonObject>(LINCHPIN_THUNDER_TIMEOUT,
                    _T(g_eventNames[LINCHPIN_EVENT_LINCHPIN_CONNECTION_ID]), &LinchpinPluginRPC::onConnectionClosed, this);
                    if (Core::ERROR_NONE != status)
                        LOGERR("Failed to subscribe to linchpin event '%s' failure code %d", g_eventNames[LINCHPIN_EVENT_LINCHPIN_CONNECTION_ID], status);
                }
            }
            return(rc);
        }

        bool LinchpinPluginRPC::InternalSubscriptionManagement(const char* method, string& topic)
        {
            if (!m_isConnected)
            {
                LOGINFO("Can't '%s' to '%s', not connected to linchpin", method, topic.c_str());
                return false;
            }

            JsonObject params, response;
            uint32_t code;
            params[g_paramNames[LINCHPIN_PARAM_CONNECTION_ID]] = m_connectionID;
            params[g_paramNames[LINCHPIN_PARAM_TOPIC_ID]] = topic;

            code = InterPluginRPC::Invoke(LINCHPIN_THUNDER_TIMEOUT, string(method), params, response);
            string stringResponse;
            response.ToString(stringResponse);
            bool rc = false;
            if (Core::ERROR_NONE == code && response.HasLabel(g_paramNames[LINCHPIN_PARAM_STATUS_ID]) &&
                response.HasLabel(g_paramNames[LINCHPIN_PARAM_STATUS_CODE_ID]))
            {
                if (string(g_paramStatus[LINCHPIN_PARAM_STATUS_CONNECTED_ID]) == response[g_paramNames[LINCHPIN_PARAM_STATUS_ID]].String())
                {
                    std::stringstream message;
                    // Connection is good but the subscription failed!
                    if (response[g_paramNames[LINCHPIN_PARAM_STATUS_CODE_ID]] != "0")
                        message << "Not able to " << string(method);
                    else
                    {
                        message << string(method);
                        rc = true;
                    }

                    message << " to '" << topic << "' return response " << stringResponse << '\'';
                    LOGINFO("%s", message.str().c_str());
                    return rc;
                }
            }

            // Something horribly bad, clear the connection state (if any)
            LOGERR("Can't '%s' to '%s', bad connection state or bad response code(%d) string(%s), disconnecting from linchpin!", 
                    method, topic.c_str(), code, stringResponse.c_str());
            Disconnect();
            return rc;
        }

        bool LinchpinPluginRPC::Subscribe(string& topic)
        {
            return (InternalSubscriptionManagement(g_linchpinMethodNames[LINCHPIN_METHOD_SUBSCRIBE_ID], topic));
        }

        bool LinchpinPluginRPC::Unsubscribe(string& topic)
        {
            return (InternalSubscriptionManagement(g_linchpinMethodNames[LINCHPIN_METHOD_UNSUBSCRIBE_ID], topic));
        }

        bool LinchpinPluginRPC::FetchMessage(string& topic, string& pbevent, JsonObject& parameters)
        {
            JsonObject params, response;
            uint32_t code;
            params[g_paramNames[LINCHPIN_PARAM_CONNECTION_ID]] = m_connectionID;
            params[g_paramNames[LINCHPIN_PARAM_TOPIC_ID]] = topic;

            code = InterPluginRPC::Invoke(LINCHPIN_THUNDER_TIMEOUT, string(g_linchpinMethodNames[LINCHPIN_METHOD_FETCH_MEESAGE_ID]), params, response);
            string stringResponse;
            response.ToString(stringResponse);
            if (Core::ERROR_NONE != code || !response.HasLabel(g_paramNames[LINCHPIN_PARAM_PAYLOAD_ID])
                || Core::JSON::Variant::type::STRING != response[g_paramNames[LINCHPIN_PARAM_PAYLOAD_ID]].Content())
            {
                LOGWARN("Notification payload fetch error for '%s', response code(%d) string(%s)",
                        topic.c_str(), code, stringResponse.c_str());
                return false;
            }

            string stringPayload = response[g_paramNames[LINCHPIN_PARAM_PAYLOAD_ID]].String();
            Core::OptionalType<Core::JSON::Error> error;
            JsonObject jsonPayload;
            // JSON integrity validation
            if (!jsonPayload.FromString(stringPayload, error))
            {
                LOGERR("Notification payload parse error for '%s' - %s at %ld - %s", topic.c_str(),
                        error.Value().Message().c_str(), error.Value().Position(), error.Value().Context().c_str());
                return false;
            }

            if (!jsonPayload.HasLabel(g_paramNames[LINCHPIN_PARAM_PAYLOAD_EVENT_ID]) ||
                Core::JSON::Variant::type::STRING != jsonPayload[g_paramNames[LINCHPIN_PARAM_PAYLOAD_EVENT_ID]].Content() ||
                (jsonPayload.HasLabel(g_paramNames[LINCHPIN_PARAM_PAYLOAD_PARAM_ID]) &&
                 Core::JSON::Variant::type::OBJECT != jsonPayload[g_paramNames[LINCHPIN_PARAM_PAYLOAD_PARAM_ID]].Content()))
            {
                LOGWARN("Notification payload '%s' for '%s' missing valid event/param information, ignored", stringResponse.c_str(), topic);
                return false;
            }
            pbevent = jsonPayload[g_paramNames[LINCHPIN_PARAM_PAYLOAD_EVENT_ID]].String();
            if (jsonPayload.HasLabel(g_paramNames[LINCHPIN_PARAM_PAYLOAD_PARAM_ID]))
                parameters = jsonPayload[g_paramNames[LINCHPIN_PARAM_PAYLOAD_PARAM_ID]].Object();

            LOGINFO("Notification payload fetched for '%s', with payload data '%s'", topic.c_str(), stringResponse.c_str());
            return true;
        }

        void LinchpinPluginRPC::onNotifyMessageReceived(const JsonObject& parameters)
        {
            string stringParameters;
            parameters.ToString(stringParameters);

            if (!parameters.HasLabel(g_paramNames[LINCHPIN_PARAM_TOPIC_ID]) ||
                parameters[g_paramNames[LINCHPIN_PARAM_TOPIC_ID]].Content() != Core::JSON::Variant::type::STRING)
            {
                LOGERR("Invalid! parameters '%s' with publish event notification, dropping event", stringParameters.c_str());
                return;
            }

            if (!m_handlerLinchpinEvents)
                LOGINFO("Default handling for event publish notification with parameters '%s'", stringParameters.c_str());
            else
            {
                m_handlerLinchpinEvents->onNotifyMessageReceived(parameters[g_paramNames[LINCHPIN_PARAM_TOPIC_ID]].String());
                LOGINFO("Event publish notification with parameters '%s' forwarded to user handler", stringParameters.c_str());
            }
        }

        void LinchpinPluginRPC::onConnectionClosed(const JsonObject& parameters)
        {
            if (m_handlerLinchpinEvents)
                m_handlerLinchpinEvents->onConnectionClosed();
            else
            {
                string stringParameters;
                parameters.ToString(stringParameters);
                LOGINFO("Default handling for event connection closed with parameters '%s'", stringParameters.c_str());
            }
        }

        bool LinchpinPluginRPC::PublishMessage(const string& topic, const string& pbevent, const JsonObject& parameters)
        {
            if (topic.empty()|| !m_isConnected)
            {
                LOGINFO("Failed to publish '%s' event, no connection or subscription", pbevent.c_str());
                return false;
            }

            JsonObject jsonPayload;
            jsonPayload[g_paramNames[LINCHPIN_PARAM_PAYLOAD_EVENT_ID]] = pbevent;
            jsonPayload[g_paramNames[LINCHPIN_PARAM_PAYLOAD_PARAM_ID]] = parameters;

            JsonObject params, response;
            params[g_paramNames[LINCHPIN_PARAM_CONNECTION_ID]] = m_connectionID;
            params[g_paramNames[LINCHPIN_PARAM_TOPIC_ID]] = topic;
            
            string stringObject;
            jsonPayload.ToString(stringObject);
            params[g_paramNames[LINCHPIN_PARAM_PAYLOAD_ID]] = stringObject;
            params[g_paramNames[LINCHPIN_PARAM_TYPE_ID]] = std::to_string(m_publishType);

            uint32_t code = InterPluginRPC::Invoke(LINCHPIN_THUNDER_TIMEOUT, string(g_linchpinMethodNames[LINCHPIN_METHOD_PUBLISH_ID]), params, response);
            string stringResponse;
            response.ToString(stringResponse);
            bool rc = false;
            if (Core::ERROR_NONE == code && response.HasLabel(g_paramNames[LINCHPIN_PARAM_STATUS_ID]) &&
                response.HasLabel(g_paramNames[LINCHPIN_PARAM_STATUS_CODE_ID]))
            {
                    std::stringstream message;
                    // Connection is good but the publish failed!
                    if (response[g_paramNames[LINCHPIN_PARAM_STATUS_CODE_ID]] != "0")
                        message << "Failed!";
                    else
                    {
                        message << "Succeded!";
                        rc = true;
                    }

                    message << " publishing '" << pbevent << "' on '" << topic << "' return response '" << stringResponse << '\'';
                    LOGINFO("%s", message.str().c_str());
                    return rc;
            }

            // Something horribly bad, clear the connection state (if any)
            LOGERR("Can't publish '%s' to '%s', bad connection state or bad response code(%d) string(%s), disconnecting from linchpin!", 
                    pbevent, topic.c_str(), code, stringResponse.c_str());
            Disconnect();
            return rc;
        }
   }
}