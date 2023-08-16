/*
 * Copyright (c) 2022, 2023 LIBERTY GLOBAL all rights reserved.
 */

#include <algorithm>
#include <functional>
#include <cstring>
#include <cerrno>
#include <string>
#include <thread>
#include <gio/gio.h>
#include <glib.h>
#include <signal.h>
#include <unistd.h>
#include "lginetwork_client.hpp"
#include "UtilsLogging.h"

namespace lginet
{

using namespace WPEFramework;

class DbusHandlerCallbacks {
    DbusHandlerCallbacks() = delete;

public:
    static LgiNetworkClient* castUserData(gpointer aUserData)
    {
        return reinterpret_cast<LgiNetworkClient*>(aUserData);
    }

    static void cbHandleStatusChanged(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar *aId,
                                        const gchar *aIfaceStatus,
                                        gpointer     aUserData)
    {
        return reinterpret_cast<LgiNetworkClient*>(aUserData)->onHandleStatusChanged(aNetworkConfigProxy, aId, aIfaceStatus);
    }

    static void cbHandleIPv4ConfigurationChanged(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar*    aId,
                                        gpointer        aUserData)
    {
        return reinterpret_cast<LgiNetworkClient*>(aUserData)->onHandleIpv4ConfigurationChanged(aNetworkConfigProxy, aId);
    }

    static void cbHandleIPv6ConfigurationChanged(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar*    aId,
                                        gpointer        aUserData)
    {
        return reinterpret_cast<LgiNetworkClient*>(aUserData)->onHandleIpv6ConfigurationChanged(aNetworkConfigProxy, aId);
    }

    static void cbHandleNetworkingEvent(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar*    aId,
                                        const gchar*    aEvent,
                                        guint           aCount,
                                        GVariant*       aParams,
                                        gpointer        aUserData)
    {
        return reinterpret_cast<LgiNetworkClient*>(aUserData)->onHandleNetworkingEvent(aNetworkConfigProxy, aId, aEvent, aCount, aParams);
    }
};

LgiNetworkClient::LgiNetworkClient()
{
    LOGINFO("user_data: %p", this);
#ifdef GDBUS_USE_CODEGEN_IMPL
    LOGINFO("using gdbus-codegen impl");
#else
    LOGINFO("using gdbusproxy impl");
#endif
}

LgiNetworkClient::~LgiNetworkClient()
{
    if (m_interface)
        Stop();
    LOGINFO("user_data: %p", this);
}

void LgiNetworkClient::connectSignal(const char* signalName,
                                      GCallback callback)
{
#ifdef GDBUS_USE_CODEGEN_IMPL
    auto object = m_interface;
#else
    auto object = m_interface->proxy;
#endif

    auto handle = g_signal_connect(object, signalName, callback, this);
    if (!handle)
    {
        LOGERR("Cannot connect to signal %s", signalName);
    }
    else
    {
        m_signalHandles.push_back(handle);
    }
}

void LgiNetworkClient::disconnectAllSignals()
{
#ifdef GDBUS_USE_CODEGEN_IMPL
    auto object = m_interface;
#else
    auto object = m_interface->proxy;
#endif
    for (auto handle : m_signalHandles)
        g_signal_handler_disconnect(object, handle);
    m_signalHandles.clear();
}

// returns vector containing all network interfaces
std::vector<std::string>* LgiNetworkClient::getInterfaces()
{
    GError* error = nullptr;
    guint count = 0;
    guint idx = 0;
    gchar** out_ids = nullptr;
    std::vector<std::string>* result = nullptr;

    if (networkconfig1_call_get_interfaces_sync(m_interface, &count, &out_ids, nullptr, &error))
    {
        result = new std::vector<std::string>();
        while (idx < count)
        {
            result->push_back(string(out_ids[idx++]));
        }
    }
    else
    {
       LOGERR("Failed to call networkconfig1_call_get_interfaces_sync - %s", error ? error->message : "(unknown)"); // ?
       if (error)
           g_error_free(error);
    }

    if (out_ids) {
        for (idx = 0; idx < count; ++idx) {
            g_free(out_ids[idx]);
        }
        g_free(out_ids);
    }

    return result;
}

// populates vector of pairs containing all parameters for specified iface
bool LgiNetworkClient::getParamsForInterface(const std::string iface, std::vector<std::pair<std::string, std::string>>& params)
{
    GError* error = nullptr;
    guint out_count = 0;
    gint out_status = 1;
    GVariant* out_params = nullptr;

    if (networkconfig1_call_get_params_sync(m_interface, iface.c_str(), &out_status, &out_count, &out_params, nullptr, &error))
    {
        if (out_status)
        {
            LOGERR("interface %s doesn't exist", iface.c_str());
            if (out_params)
            {
                g_variant_unref(out_params);
            }
            return false;
        }
        GVariantIter    iter;
        gchar*          key;
        gchar*          value;

        if (out_params)
        {
            g_variant_iter_init(&iter, out_params);
            while (g_variant_iter_loop (&iter, "{ss}", &key, &value))
            {
                params.push_back(make_pair(std::string(key), std::string(value)));
            }
            g_variant_unref(out_params);
        }

        return true;
    }
    else
    {
       LOGERR("Failed to call networkconfig1_call_get_params_sync - %s", error ? error->message : "(unknown)"); // ?
       if (error)
           g_error_free(error);
        return false;
    }
}

// populates a map containing specified parameters for specified iface
bool LgiNetworkClient::getSpecificParamsForInterface(const std::string iface, std::map<std::string, std::string>& params)
{
    GError* error = nullptr;
    guint out_count = 0;
    gint out_status = 1;
    GVariant* out_params = nullptr;

    if (!params.empty() && networkconfig1_call_get_params_sync(m_interface, iface.c_str(), &out_status, &out_count, &out_params, nullptr, &error))
    {
        if (out_status)
        {
            LOGERR("interface %s doesn't exist", iface.c_str());
            if (out_params)
                g_variant_unref(out_params);
            return false;
        }
        GVariantIter    iter;
        gchar*          key;
        gchar*          value;

        if (out_params)
        {
            g_variant_iter_init(&iter, out_params);
            while (g_variant_iter_loop (&iter, "{ss}", &key, &value))
            {
                std::string skey(key);
                auto iter = params.find(skey);
                if (iter != params.end())
                    iter->second = string(value);
            }
            g_variant_unref(out_params);
        }

        return true;
    }
    else
    {
        return false;
    }
}

std::string LgiNetworkClient::getDefaultInterface()
{
    std::string iface;
    GError* error = nullptr;
    gchar* out_id = nullptr;
    gint out_status = 0;
    if (networkconfig1_call_get_active_interface_sync(m_interface, &out_id, &out_status, nullptr, &error))
    {
        iface.assign(out_id);
        g_free(out_id);
    }
    else
    {
        LOGERR("Failed to get default interface: %s",  error ? error->message : "(unknown)");
        if (error)
            g_error_free(error);
    }
    return iface;
}

bool LgiNetworkClient::isInterfaceEnabled(const std::string iface)
{
    gint out_status = 0;
    gboolean out_enabled = false;
    GError* error = nullptr;
    if (networkconfig1_call_is_enabled_sync(m_interface, iface.c_str(), &out_status, &out_enabled, nullptr, &error))
        return out_enabled;
    else
    {
        if (error)
        {
            LOGERR("is_enabled_sync for %s failed: %s", iface.c_str(), error->message);
            g_error_free(error);
        }
        return false;
    }
}

bool LgiNetworkClient::setInterfaceEnabled(const std::string iface, bool newstate)
{
    gint out_status = 0;
    GError* error = nullptr;
    if (networkconfig1_call_enable_sync(m_interface, iface.c_str(), newstate, &out_status, nullptr, &error))
        return true;
    else
    {
        if (error)
        {
            LOGERR("Failed to %s interface %s: %s", newstate ? "enable" : "disable", iface.c_str(), error->message);
            g_error_free(error);
        }
        return false;
    }
}

void LgiNetworkClient::onHandleNetworkingEvent(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar*    aId,
                                        const gchar*    aEvent,
                                        guint           aCount,
                                        GVariant*       aParams)
{
    if (aCount && aEvent && onNetworkingEvent)
    {
        std::map<string, string> params;
        GVariantIter lIter;
        gchar*       lKey   = nullptr;
        gchar*       lValue = nullptr;
        g_variant_iter_init(&lIter, aParams);
        while (g_variant_iter_loop (&lIter, "{ss}", &lKey, &lValue))
        {
            string key(lKey);
            string value(lValue);
            params[lKey] = lValue;
        }
        onNetworkingEvent(std::string(aId), std::string(aEvent), params);
    }
}

void LgiNetworkClient::onHandleIpv4ConfigurationChanged(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar*    aId)
{
    if (onHandleIpv4ConfigurationChangedEvent)
        onHandleIpv4ConfigurationChangedEvent(aId);
}

void LgiNetworkClient::onHandleIpv6ConfigurationChanged(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar*    aId)
{
    if (onHandleIpv6ConfigurationChangedEvent)
        onHandleIpv6ConfigurationChangedEvent(aId);
}

void LgiNetworkClient::onHandleStatusChanged(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar *aId,
                                        const gchar *aIfaceStatus)
{
    if (onStatusChangeEvent)
        onStatusChangeEvent(std::string(aId), std::string(aIfaceStatus));
}

#ifndef GDBUS_USE_CODEGEN_IMPL
static void handle_dbus_event(GDBusProxy *proxy,
                          char *sender_name,
                          char *_signal_name,
                          GVariant *parameters,
                          gpointer user_data)
{
    std::string signal_name{_signal_name};

    const gsize num_params = g_variant_n_children(parameters);
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    LgiNetworkClient *client = static_cast<LgiNetworkClient *>(user_data);

    // store the return values from g_variant_iter_next_value to release them later
    std::vector<GVariant*> variants;

    for (gsize i=0; i<num_params; ++i) {
        variants.push_back(g_variant_iter_next_value(&iter));
    }

    if (signal_name == "IPv4ConfigurationChanged" && num_params == 1)
    {
        // IPv4ConfigurationChanged g_variant_get_type_string is (s)
        const gchar *aId = g_variant_get_string(variants[0], NULL);
        DbusHandlerCallbacks::cbHandleIPv4ConfigurationChanged(client, aId, user_data);
    }
    else if (signal_name == "IPv6ConfigurationChanged" && num_params == 1)
    {
        // IPv6ConfigurationChanged g_variant_get_type_string is (s)
        const gchar *aId = g_variant_get_string(variants[0], NULL);
        DbusHandlerCallbacks::cbHandleIPv6ConfigurationChanged(client, aId, user_data);
    }
    else if (signal_name == "NetworkingEvent" && num_params == 4)
    {
        // NetworkingEvent g_variant_get_type_string is (ssua{ss})
        const gchar *aId = g_variant_get_string(variants[0], NULL);
        const gchar *aEvent = g_variant_get_string(variants[1], NULL);
        // count is 'u' / guint32
        guint aCount = g_variant_get_uint32(variants[2]);
        GVariant *aParams = variants[3];
        DbusHandlerCallbacks::cbHandleNetworkingEvent(client, aId, aEvent, aCount, aParams, user_data);
    }
    else if (signal_name == "StatusChanged" && num_params == 2)
    {
        // StatusChanged g_variant_get_type_string is (ss)
        const gchar *aId = g_variant_get_string(variants[0], NULL);
        const gchar *aIfaceStatus = g_variant_get_string(variants[1], NULL);
        DbusHandlerCallbacks::cbHandleStatusChanged(client, aId, aIfaceStatus, user_data);
    }
    else
    {
        LOGINFO("handle_dbus_event: unsupported event; sender_name: %s signal_name: %s, num_params: %zu" , sender_name, _signal_name, num_params);
    }

    // free the return value from g_variant_iter_next_value (https://developer-old.gnome.org/glib/stable/glib-GVariant.html#g-variant-iter-next-value)
    for (auto& v : variants) {
        g_variant_unref(v);
    }
}
#endif

int LgiNetworkClient::Run()
{
    LOGINFO("user_data: %p", this);

    m_loopThread = std::thread(&LgiNetworkClient::Worker, this);
    bool initialized = m_initialized.get_future().get();

    if (initialized && m_interface)
    {
#ifdef GDBUS_USE_CODEGEN_IMPL
        connectSignal("ipv4-configuration-changed", G_CALLBACK(DbusHandlerCallbacks::cbHandleIPv4ConfigurationChanged));
        connectSignal("networking-event", G_CALLBACK (DbusHandlerCallbacks::cbHandleNetworkingEvent));
        connectSignal("status-changed", G_CALLBACK (DbusHandlerCallbacks::cbHandleStatusChanged));
#else
        connectSignal("g-signal", G_CALLBACK (handle_dbus_event));
#endif
        LOGINFO("Networkconfig proxy created");
    }
    else
    {
        LOGERR("Failed to create networkconfig proxy.");
    }

    return initialized ? 0 : -1;
}

void LgiNetworkClient::Worker()
{
    LOGINFO("LgiNetworkClient::Worker() TID: %u", gettid());

    m_mainContext = g_main_context_new();
    m_mainLoop = g_main_loop_new(m_mainContext, false);

    if(m_mainLoop && m_mainContext) {
        g_main_context_push_thread_default(m_mainContext);

        GError* error = nullptr;
        m_interface = networkconfig1_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                    G_DBUS_PROXY_FLAGS_NONE,
                                    NETWORK_CONFIG_DBUS_INTERFACE_NAME,
                                    NETWORK_CONFIG_DBUS_INTERFACE_OBJECT_PATH,
                                    NULL, /* GCancellable */
                                    &error);
        if (error)
        {
            LOGERR("Failed to create networkconfig proxy: %s", error->message);
            g_error_free(error);
        }
        m_initialized.set_value(m_interface != nullptr);

        LOGINFO("LgiNetworkClient::Worker() start main loop TID: %u", gettid());
        g_main_loop_run(m_mainLoop); // blocks
        LOGINFO("LgiNetworkClient::Worker() main loop finished TID: %u", gettid());
        g_main_context_pop_thread_default(m_mainContext);
        g_main_loop_unref(m_mainLoop);
        g_main_context_unref(m_mainContext);
        m_mainLoop = nullptr;
        m_mainContext = nullptr;
    }
    else {
        LOGERR("Failed to create glib main loop");
        m_initialized.set_value(false);
    }
}

static void release_networkconfig1(Networkconfig1* interface)
{
#ifdef GDBUS_USE_CODEGEN_IMPL
    g_object_unref(interface);
#else
    if (interface->proxy) g_object_unref(interface->proxy);
    g_dbus_connection_flush_sync(interface->connection, NULL, NULL);
    g_object_unref(interface->connection);
    g_free(interface);
#endif
}

void LgiNetworkClient::Stop()
{
    if (m_interface)
    {
        disconnectAllSignals();
        release_networkconfig1(m_interface);
        m_interface = nullptr;
        g_main_context_invoke(m_mainContext, +[](gpointer ptr) -> gboolean {
            LOGINFO("LgiNetworkClient::Stop() quit main loop TID: %u", gettid());
            g_main_loop_quit((GMainLoop*)ptr);
            return FALSE;
        }, (gpointer)m_mainLoop);

        if(m_loopThread.joinable()) {
            m_loopThread.join();
        }
        else {
            LOGERR("Worker thread should be joinable");
        }
        LOGINFO("signals disconnected");
    }
}

} // namespace lginet

