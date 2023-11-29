/*
 * Copyright (c) 2023 LIBERTY GLOBAL all rights reserved.
 */

#ifndef OMI_CLIENT_HPP_
#define OMI_CLIENT_HPP_

#include <thread>
#include <future>
#include <vector>
#include <map>
#include <gio/gio.h>
#include <glib.h>

#include "omi_dbus_api.h"

#include "../Module.h"

#define OMI_CONFIG_DBUS_INTERFACE_NAME        "com.lgi.onemw.omi1"
#define OMI_CONFIG_DBUS_INTERFACE_OBJECT_PATH "/com/lgi/onemw/omi1"

namespace OmiDbus
{

class OmiClient
{
public:
    enum class ErrorType { verityFailed };
    typedef std::function<void(const std::string&, ErrorType, const void*)> OmiErrorListener;

public:
    OmiClient();
    ~OmiClient();
    OmiClient(const OmiClient&) = delete;
    OmiClient& operator=(OmiClient const&) = delete;
    
    int Run();
    void Stop();

    bool mountCryptedBundle(const std::string& id,
                                    const std::string& rootfs_file_path,
                                    const std::string& config_json_path,
                                    std::string& bundlePath /*out parameter*/);
    bool umountCryptedBundle(const std::string& id);
    long unsigned registerListener(const OmiErrorListener &listener, const void* cbParams);
    void unregisterListener(long unsigned tag);
    
private:
    void handleVerityFailed(GVariant *parameters);
    static void handleDbusEvent(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name_c_str,
                              GVariant *parameters, gpointer user_data);

    void connectSignal(const char* signalName, GCallback callback);
    void disconnectAllSignals();
    void Worker();

    Omi1* m_interface;
    std::pair<OmiErrorListener, const void*> m_listener;
    std::mutex m_listener_mtx;
    std::vector<gulong> m_signalHandles;
    std::thread m_loopThread;
    std::promise<bool> m_initialized;
    GMainContext *m_mainContext;
    GMainLoop *m_mainLoop;
};

} // namespace lginet

#endif // #ifdef OMI_CLIENT_HPP_
