/*
 * Copyright (c) 2023 LIBERTY GLOBAL all rights reserved.
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
#include "omi_client.hpp"
#include "UtilsLogging.h"

namespace OmiDbus
{

using namespace WPEFramework;

OmiClient::OmiClient()
{
    LOGINFO("user_data: %p", this);
}

OmiClient::~OmiClient()
{
    if (m_interface) {
        Stop();
    }
    LOGINFO("user_data: %p", this);
}

void OmiClient::connectSignal(const char* signalName,
                                      GCallback callback)
{
    auto object = m_interface->proxy;

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

void OmiClient::disconnectAllSignals()
{
    auto object = m_interface->proxy;
    for (auto handle : m_signalHandles) {
        g_signal_handler_disconnect(object, handle);
    }
    m_signalHandles.clear();
}

void OmiClient::handleDbusEvent(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name_c_str, GVariant *parameters, gpointer user_data)
{
    std::string signal_name = signal_name_c_str;
    OmiClient *self = static_cast<OmiClient *>(user_data);

    if ("VerityFailed" == signal_name)
    {
        self->handleVerityFailed(parameters);
    }
}

void OmiClient::handleVerityFailed(GVariant *parameters)
{
    gchar *id_c_str;

    g_variant_get(parameters, "(&s)", &id_c_str);
    std::string id{id_c_str};

    std::unique_lock<std::mutex> lock(m_listener_mtx);
    if(m_listener.first) {
        m_listener.first(id, ErrorType::verityFailed, m_listener.second);
    }
}

bool OmiClient::mountCryptedBundle(const std::string& id,
                                const std::string& rootfs_file_path,
                                const std::string& config_json_path,
                                std::string& bundlePath /*out parameter*/) {
    gchar *containerPath;

    gboolean res = omi1_call_mount_sync(m_interface,
                                        id.c_str(),
                                        rootfs_file_path.c_str(),
                                        config_json_path.c_str(),
                                        &containerPath,
                                        nullptr,
                                        nullptr);

    if (!res)
    {
        return false;
    }

    bundlePath = std::string(containerPath);

    return true;                               
}

bool OmiClient::umountCryptedBundle(const std::string& id)  {
    return omi1_call_umount_sync(m_interface,
                                 id.c_str(),
                                 nullptr,
                                 nullptr);
}

long unsigned OmiClient::registerListener(const OmiErrorListener &listener, const void* cbParams)  {
    std::unique_lock<std::mutex> lock(m_listener_mtx);
    m_listener = std::make_pair(listener, cbParams);
    return 0;
}

void OmiClient::unregisterListener(long unsigned tag)  {
    std::unique_lock<std::mutex> lock(m_listener_mtx);
    m_listener = std::make_pair(OmiErrorListener(), nullptr);
}

int OmiClient::Run()
{
    LOGINFO("user_data: %p", this);

    m_loopThread = std::thread(&OmiClient::Worker, this);
    bool initialized = m_initialized.get_future().get();

    if (initialized && m_interface)
    {
        connectSignal("g-signal", G_CALLBACK (OmiClient::handleDbusEvent));
        LOGINFO("Omi proxy created");
    }
    else
    {
        LOGERR("Failed to create networkconfig proxy.");
    }

    return initialized ? 0 : -1;
}

void OmiClient::Worker()
{
    LOGINFO("OmiClient::Worker() TID: %u", gettid());

    m_mainContext = g_main_context_new();
    m_mainLoop = g_main_loop_new(m_mainContext, false);

    if(m_mainLoop && m_mainContext) {
        g_main_context_push_thread_default(m_mainContext);

        GError* error = nullptr;
        m_interface = omi1_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                    G_DBUS_PROXY_FLAGS_NONE,
                                    OMI_CONFIG_DBUS_INTERFACE_NAME,
                                    OMI_CONFIG_DBUS_INTERFACE_OBJECT_PATH,
                                    NULL, /* GCancellable */
                                    &error);
        if (error)
        {
            LOGERR("Failed to create networkconfig proxy: %s", error->message);
            g_error_free(error);
        }
        m_initialized.set_value(m_interface != nullptr);

        LOGINFO("OmiClient::Worker() start main loop TID: %u", gettid());
        g_main_loop_run(m_mainLoop); // blocks
        LOGINFO("OmiClient::Worker() main loop finished TID: %u", gettid());
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

static void release_omi1(Omi1* interface)
{
    if (interface->proxy) g_object_unref(interface->proxy);
    g_dbus_connection_flush_sync(interface->connection, NULL, NULL);
    g_object_unref(interface->connection);
    g_free(interface);
}

void OmiClient::Stop()
{
    if (m_interface)
    {
        disconnectAllSignals();
        release_omi1(m_interface);
        m_interface = nullptr;
        g_main_context_invoke(m_mainContext, +[](gpointer ptr) -> gboolean {
            LOGINFO("OmiClient::Stop() quit main loop TID: %u", gettid());
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

